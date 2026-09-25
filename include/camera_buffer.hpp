#pragma once
#include <opencv2/core.hpp>
#include <mutex>
#include <atomic>

struct CameraBuffer {
    cv::Mat frame;
    std::mutex mtx;
    bool hasNew = false;

    // Удобный метод: забрать кадр без гонок
    bool tryGetFrame(cv::Mat& out) {
        std::lock_guard<std::mutex> lock(mtx);
        if (frame.empty()) return false;
        out = frame.clone();
        hasNew = false;
        return true;
    }

    // Положить новый кадр (swap — без копии)
    void setFrame(cv::Mat& src) {
        std::lock_guard<std::mutex> lock(mtx);
        cv::swap(frame, src);
        hasNew = true;
    }

    CameraBuffer() = default;
    CameraBuffer(const CameraBuffer&) = delete;
    CameraBuffer& operator=(const CameraBuffer&) = delete;
};