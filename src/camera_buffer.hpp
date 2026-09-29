#pragma once
#include <opencv2/opencv.hpp>
#include <mutex>

class CameraBuffer {
public:
    void setFrame(const cv::Mat& frame) {
        std::lock_guard<std::mutex> lock(mtx_);
        frame.copyTo(frame_);
        hasFrame_ = true;
    }

    bool tryGetFrame(cv::Mat& out) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (!hasFrame_) return false;
        frame_.copyTo(out);
        return true;
    }

private:
    std::mutex mtx_;
    cv::Mat    frame_;
    bool       hasFrame_ = false;
};