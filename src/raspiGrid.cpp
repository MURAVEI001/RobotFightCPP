#include "raspiGrid.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>

namespace {

constexpr uint32_t kMagic         = 0x52415350; // "RASP"
constexpr int      kMaxUdpPayload = 1400;
constexpr uint32_t kMaxFrameSize  = 32u * 1024u * 1024u; // sanity: 32 MB

#pragma pack(push, 1)
struct FrameHeader {
    uint32_t magic;
    uint16_t camera_id;
    uint32_t frame_id;
    uint32_t total_size;
    uint32_t chunk_off;
    uint32_t chunk_len;
};
#pragma pack(pop)

static_assert(sizeof(FrameHeader) == 22, "FrameHeader must be exactly 22 bytes");

} // namespace

// ---------------------------------------------------------------------------
RaspiGrid::RaspiGrid(const Config& cfg) : cfg_(cfg) {
    if (cfg_.cell_width <= 0 || cfg_.cell_height <= 0 ||
        cfg_.cols <= 0 || cfg_.rows <= 0) {
        throw std::runtime_error("RaspiGrid: invalid grid geometry");
    }

    rebuildGrid();

    fd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (fd_ < 0) {
        throw std::runtime_error(
            std::string("RaspiGrid: socket() failed: ") + std::strerror(errno));
    }

    int one = 1;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    // Буфер побольше — 6 камер на 30 fps дают ощутимые всплески.
    int rcvbuf = 4 * 1024 * 1024;
    ::setsockopt(fd_, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(cfg_.port);

    if (::inet_pton(AF_INET, cfg_.bind_address.c_str(), &addr.sin_addr) != 1) {
        ::close(fd_);
        throw std::runtime_error("RaspiGrid: bad bind address: " + cfg_.bind_address);
    }

    if (::bind(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        const std::string err = std::strerror(errno);
        ::close(fd_);
        throw std::runtime_error("RaspiGrid: bind() failed: " + err);
    }

    const int flags = ::fcntl(fd_, F_GETFL, 0);
    if (flags < 0 || ::fcntl(fd_, F_SETFL, flags | O_NONBLOCK) < 0) {
        ::close(fd_);
        throw std::runtime_error("RaspiGrid: fcntl(O_NONBLOCK) failed");
    }
}

RaspiGrid::~RaspiGrid() {
    if (fd_ >= 0) ::close(fd_);
}

// ---------------------------------------------------------------------------
void RaspiGrid::rebuildGrid() {
    const int W = cfg_.cols * cfg_.cell_width;
    const int H = cfg_.rows * cfg_.cell_height;

    grid_ = cv::Mat::zeros(H, W, CV_8UC3);
    cell_ready_.assign(static_cast<size_t>(cfg_.cols * cfg_.rows), false);

    // Незавершённые сборки относились к старой раскладке — сбрасываем.
    for (auto& [cam, fa] : assemblies_) {
        fa = FrameAssembly{};
    }
}

void RaspiGrid::resizeGrid(int cols, int rows) {
    if (cols <= 0 || rows <= 0) return;
    if (cols == cfg_.cols && rows == cfg_.rows) return;

    cfg_.cols = cols;
    cfg_.rows = rows;

    // Чистим маппинг камер, которые теперь вне диапазона.
    const int n = cellsCount();
    for (auto it = cfg_.camera_to_cell.begin(); it != cfg_.camera_to_cell.end(); ) {
        if (it->second < 0 || it->second >= n)
            it = cfg_.camera_to_cell.erase(it);
        else
            ++it;
    }

    rebuildGrid();
}

// ---------------------------------------------------------------------------
void RaspiGrid::poll() {
    uint8_t buf[65536];

    for (;;) {
        sockaddr_in from{};
        socklen_t   from_len = sizeof(from);

        const ssize_t n = ::recvfrom(fd_, buf, sizeof(buf), 0,
                                     reinterpret_cast<sockaddr*>(&from),
                                     &from_len);

        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            std::cerr << "[RaspiGrid] recvfrom: " << std::strerror(errno) << "\n";
            break;
        }
        if (n > 0) {
            handleDatagram(buf, static_cast<size_t>(n));
        }
    }

    // Отбрасываем недособранные кадры, которые висят слишком долго.
    const auto now = std::chrono::steady_clock::now();
    for (auto& [cam_id, fa] : assemblies_) {
        if (!fa.active) continue;
        const auto age_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - fa.started_at).count();
        if (age_ms > cfg_.frame_timeout_ms) {
            ++frames_dropped_;
            fa.active = false;
        }
    }
}

// ---------------------------------------------------------------------------
void RaspiGrid::handleDatagram(const uint8_t* data, size_t len) {
    if (len < sizeof(FrameHeader)) return;

    FrameHeader hdr;
    std::memcpy(&hdr, data, sizeof(hdr));

    if (ntohl(hdr.magic) != kMagic) return;

    const uint16_t camera_id = ntohs(hdr.camera_id);
    const uint32_t frame_id  = ntohl(hdr.frame_id);
    const uint32_t total     = ntohl(hdr.total_size);
    const uint32_t off       = ntohl(hdr.chunk_off);
    const uint32_t clen      = ntohl(hdr.chunk_len);

    ++datagrams_received_;

    if (clen == 0 || clen > kMaxUdpPayload)   return;
    if (len < sizeof(FrameHeader) + clen)     return;
    if (off + clen > total)                   return;
    if (total == 0 || total > kMaxFrameSize)  return;

    auto& fa = assemblies_[camera_id];

    if (!fa.active || fa.frame_id != frame_id || fa.total_size != total) {
        if (fa.active) ++frames_dropped_; // предыдущий кадр не дособрался
        fa.active       = true;
        fa.frame_id     = frame_id;
        fa.total_size   = total;
        fa.buffer.assign(total, 0);
        fa.chunks_total = (total + kMaxUdpPayload - 1) / kMaxUdpPayload;
        fa.chunk_seen.assign(fa.chunks_total, false);
        fa.chunks_seen  = 0;
        fa.started_at   = std::chrono::steady_clock::now();
    }

    const size_t chunk_index = off / kMaxUdpPayload;
    if (chunk_index >= fa.chunks_total) return;
    if (fa.chunk_seen[chunk_index])     return; // дубликат

    std::memcpy(fa.buffer.data() + off, data + sizeof(FrameHeader), clen);
    fa.chunk_seen[chunk_index] = true;
    ++fa.chunks_seen;

    if (fa.chunks_seen == fa.chunks_total) {
        processAssembly(camera_id, fa);
        fa.active = false;
    }
}

// ---------------------------------------------------------------------------
void RaspiGrid::processAssembly(uint16_t camera_id, FrameAssembly& fa) {
    cv::Mat raw(1, static_cast<int>(fa.buffer.size()), CV_8UC1,
                fa.buffer.data());

    cv::Mat decoded = cv::imdecode(raw, cv::IMREAD_COLOR);
    if (decoded.empty()) {
        ++frames_dropped_;
        return;
    }

    ++frames_decoded_;
    blitToGrid(camera_id, decoded);
}

// ---------------------------------------------------------------------------
void RaspiGrid::blitToGrid(uint16_t camera_id, const cv::Mat& img) {
    const int idx = cellIndexFor(camera_id);
    if (idx < 0 || idx >= cellsCount()) {
        std::cerr << "[RaspiGrid] camera_id " << camera_id
                  << " -> invalid cell " << idx << "\n";
        return;
    }

    const int row = idx / cfg_.cols;
    const int col = idx % cfg_.cols;

    const cv::Rect roi(col * cfg_.cell_width,
                       row * cfg_.cell_height,
                       cfg_.cell_width,
                       cfg_.cell_height);

    cv::Mat resized;
    if (img.size() != roi.size()) {
        cv::resize(img, resized, roi.size(), 0, 0, cv::INTER_AREA);
    } else {
        resized = img;
    }
    resized.copyTo(grid_(roi));

    cell_ready_[static_cast<size_t>(idx)] = true;
}

// ---------------------------------------------------------------------------
int RaspiGrid::cellIndexFor(uint16_t camera_id) const {
    auto it = cfg_.camera_to_cell.find(camera_id);
    if (it != cfg_.camera_to_cell.end()) return it->second;
    return static_cast<int>(camera_id);
}

// ---------------------------------------------------------------------------
cv::Mat RaspiGrid::cellView(int row, int col) {
    if (row < 0 || row >= cfg_.rows) return {};
    if (col < 0 || col >= cfg_.cols) return {};

    return grid_(cv::Rect(col * cfg_.cell_width,
                          row * cfg_.cell_height,
                          cfg_.cell_width,
                          cfg_.cell_height));
}

cv::Mat RaspiGrid::cellView(int idx) {
    if (idx < 0 || idx >= cellsCount()) return {};
    return cellView(idx / cfg_.cols, idx % cfg_.cols);
}

cv::Mat RaspiGrid::cellView(uint16_t camera_id) {
    return cellView(cellIndexFor(camera_id));
}

// ---------------------------------------------------------------------------
bool RaspiGrid::cellReady(uint16_t camera_id) const {
    const int idx = cellIndexFor(camera_id);
    if (idx < 0 || idx >= cellsCount()) return false;
    return cell_ready_[static_cast<size_t>(idx)];
}