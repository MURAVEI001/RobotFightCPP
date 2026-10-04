#pragma once

#include <cstdint>
#include <string>
#include <map>
#include <mutex>
#include <chrono>

#include <opencv2/core.hpp>

class FrameProcessor {
public:
    FrameProcessor() = default;

    // Вызывается из потоков-клиентов при получении кадра.
    // Данные декодируются тут же, поэтому буфер клиента может быть переиспользован.
    void processFrame(const std::string& camera_id,
                      const uint8_t* jpeg_data,
                      size_t jpeg_size);

    // Вызывается из ГЛАВНОГО потока (на macOS это обязательно для imshow).
    // Возвращает false, если пользователь закрыл окно или нажал ESC/q.
    bool renderAndShow(int tile_width  = 640,
                       int tile_height = 360,
                       int grid_cols   = 2,
                       int display_fps = 30);

private:
    struct CameraFrame {
        cv::Mat  image;
        uint64_t frame_count = 0;
        std::chrono::steady_clock::time_point last_update{};
    };

    std::mutex mutex_;
    std::map<std::string, CameraFrame> frames_; // ordered by camera_id
    bool quit_ = false;
};