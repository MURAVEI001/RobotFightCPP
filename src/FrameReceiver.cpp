#include "FrameReceiver.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cerrno>

FrameReceiver::FrameReceiver(uint16_t port, FrameProcessor* processor)
    : port_(port), processor_(processor) {}

FrameReceiver::~FrameReceiver() {
    stop();
}

bool FrameReceiver::start() {
    listen_sock_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock_ < 0) {
        perror("FrameReceiver::start: socket");
        return false;
    }

    int opt = 1;
    setsockopt(listen_sock_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port_);

    if (bind(listen_sock_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        perror("FrameReceiver::start: bind");
        close(listen_sock_);
        listen_sock_ = -1;
        return false;
    }

    if (listen(listen_sock_, 10) < 0) {
        perror("FrameReceiver::start: listen");
        close(listen_sock_);
        listen_sock_ = -1;
        return false;
    }

    running_ = true;
    std::thread(&FrameReceiver::acceptLoop, this).detach();
    printf("RobotFightCPP сервер запущен на порту %u\n", port_);
    return true;
}

void FrameReceiver::stop() {
    running_ = false;
    if (listen_sock_ >= 0) {
        ::close(listen_sock_);
        listen_sock_ = -1;
    }
    for (auto& t : client_threads_) {
        if (t.joinable()) t.join();
    }
    client_threads_.clear();
}

void FrameReceiver::acceptLoop() {
    while (running_) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_sock = accept(listen_sock_,
                                 reinterpret_cast<struct sockaddr*>(&client_addr),
                                 &client_len);
        if (client_sock < 0) {
            if (running_) perror("FrameReceiver::acceptLoop: accept");
            continue;
        }

        // Идентификатор клиента: IP:port
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
// было:
// std::string client_id = std::string(ip_str) + ":" + std::to_string(ntohs(client_addr.sin_port));

// стало:
std::string client_id = std::string(ip_str); // только IP
        printf("Подключился клиент: %s\n", client_id.c_str());

        // Отключаем Nagle для снижения задержки
        int flag = 1;
        setsockopt(client_sock, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

        // Увеличиваем буфер приёма
        int rcvbuf = 4 * 1024 * 1024;
        setsockopt(client_sock, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));

        client_threads_.emplace_back(&FrameReceiver::clientLoop, this, client_sock, client_id);
    }
}

void FrameReceiver::clientLoop(int client_sock, const std::string& client_id) {
    std::vector<uint8_t> frame_buffer;
    frame_buffer.reserve(2 * 1024 * 1024); // 2 MB под Full HD MJPEG

    while (running_) {
        // Чтение заголовка (4 байта длины)
        uint32_t len_net = 0;
        ssize_t n = recv(client_sock, &len_net, sizeof(len_net), MSG_WAITALL);
        if (n != sizeof(len_net)) {
            if (n == 0) {
                printf("Клиент %s отключился\n", client_id.c_str());
            } else {
                perror("FrameReceiver::clientLoop: recv header");
            }
            break;
        }

        uint32_t frame_len = ntohl(len_net);
        if (frame_len == 0 || frame_len > 10 * 1024 * 1024) { // защита от мусора
            fprintf(stderr, "Некорректная длина кадра от %s: %u\n", client_id.c_str(), frame_len);
            break;
        }

        // Чтение данных кадра
        if (frame_buffer.size() < frame_len) {
            frame_buffer.resize(frame_len);
        }
        n = recv(client_sock, frame_buffer.data(), frame_len, MSG_WAITALL);
        if (n != static_cast<ssize_t>(frame_len)) {
            perror("FrameReceiver::clientLoop: recv data");
            break;
        }

        // Передача кадра в обработчик
        if (processor_) {
            processor_->processFrame(client_id, frame_buffer.data(), frame_len);
        }
    }

    ::close(client_sock);
}