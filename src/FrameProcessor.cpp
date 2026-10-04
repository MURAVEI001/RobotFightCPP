#include "FrameProcessor.hpp"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <algorithm>
#include <cstdio>
#include <vector>

void FrameProcessor::processFrame(const std::string& camera_id,
                                  const uint8_t* jpeg_data,
                                  size_t jpeg_size) {
    if (jpeg_data == nullptr || jpeg_size == 0) return;

    // Оборачиваем буфер клиента в cv::Mat БЕЗ копирования (Mat не владеет памятью,
    // но imdecode прочитает её до возврата).
    cv::Mat raw(1, static_cast<int>(jpeg_size), CV_8UC1,
                const_cast<uint8_t*>(jpeg_data));

    // Декодируем ВНЕ мьютекса — это самая тяжёлая часть.
    cv::Mat img = cv::imdecode(raw, cv::IMREAD_COLOR);
    if (img.empty()) {
        fprintf(stderr, "[%s] не удалось декодировать JPEG (%zu байт)\n",
                camera_id.c_str(), jpeg_size);
        return;
    }

    {
        std::lock_guard<std::mutex> lk(mutex_);
        auto& entry       = frames_[camera_id];
        entry.image       = std::move(img);
        entry.last_update = std::chrono::steady_clock::now();
        ++entry.frame_count;
    }
}

bool FrameProcessor::renderAndShow(int tile_w, int tile_h,
                                   int grid_cols, int display_fps) {
    static const std::string kWindow = "RobotFightCPP";

    // ----- 1. Снимок кадров под мьютексом (клонируем, чтобы быстро отпустить lock) -----
    struct Snapshot { std::string id; cv::Mat image; uint64_t count; bool online; };
    std::vector<Snapshot> snapshot;
    const auto now = std::chrono::steady_clock::now();

    {
        std::lock_guard<std::mutex> lk(mutex_);
        for (auto& kv : frames_) {
            if (kv.second.image.empty()) continue;
            const bool online =
                (now - kv.second.last_update) < std::chrono::seconds(2);
            snapshot.push_back({kv.first, kv.second.image.clone(),
                                kv.second.frame_count, online});
        }
    }

    // ----- 2. Если кадров нет — показываем заглушку -----
    if (snapshot.empty()) {
        cv::Mat placeholder(360, 640, CV_8UC3, cv::Scalar(30, 30, 30));
        cv::putText(placeholder, "Waiting for cameras...",
                    cv::Point(150, 190), cv::FONT_HERSHEY_SIMPLEX, 0.8,
                    cv::Scalar(200, 200, 200), 2, cv::LINE_AA);
        cv::imshow(kWindow, placeholder);
        const int key = cv::waitKey(std::max(1, 1000 / std::max(1, display_fps)));
        if (key == 27 || key == 'q' || key == 'Q') quit_ = true;
        else if (cv::getWindowProperty(kWindow, cv::WND_PROP_VISIBLE) < 1) quit_ = true;
        return !quit_;
    }

    // ----- 3. Сетка -----
    const int n    = static_cast<int>(snapshot.size());
    const int cols = std::max(1, grid_cols);
    const int rows = (n + cols - 1) / cols;

    cv::Mat canvas(rows * tile_h, cols * tile_w, CV_8UC3, cv::Scalar(15, 15, 15));

    for (int i = 0; i < n; ++i) {
        const auto& s = snapshot[i];
        const int r = i / cols;
        const int c = i % cols;

        cv::Mat tile;
        cv::resize(s.image, tile, cv::Size(tile_w, tile_h), 0, 0, cv::INTER_LINEAR);
        if (!s.online) {
            // Приглушаем offline-кадр
            cv::Mat dark;
            tile.convertTo(dark, -1, 0.35, 0.0);
            tile = dark;
        }

        cv::Rect roi(c * tile_w, r * tile_h, tile_w, tile_h);
        tile.copyTo(canvas(roi));

        // Разделительная рамка
        cv::rectangle(canvas, roi, cv::Scalar(60, 60, 60), 1);

        // Подпись: IP камеры
        cv::putText(canvas, s.id,
                    cv::Point(c * tile_w + 10, r * tile_h + 25),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6,
                    s.online ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255),
                    1, cv::LINE_AA);

        // Номер кадра
        char buf[64];
        std::snprintf(buf, sizeof(buf), "frame #%llu%s",
                      static_cast<unsigned long long>(s.count),
                      s.online ? "" : "  [OFFLINE]");
        cv::putText(canvas, buf,
                    cv::Point(c * tile_w + 10, r * tile_h + 50),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5,
                    cv::Scalar(200, 200, 200), 1, cv::LINE_AA);
    }

    // ----- 4. Показ и обработка клавиш -----
    cv::imshow(kWindow, canvas);
    const int delay_ms = std::max(1, 1000 / std::max(1, display_fps));
    const int key = cv::waitKey(delay_ms);

    if (key == 27 || key == 'q' || key == 'Q') {
        quit_ = true;
    } else if (cv::getWindowProperty(kWindow, cv::WND_PROP_VISIBLE) < 1) {
        quit_ = true;
    }
    return !quit_;
}