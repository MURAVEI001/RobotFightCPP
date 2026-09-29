#include "video_source.hpp"
#include <iostream>

cv::VideoCapture openVideo(const std::string& path) {
    cv::VideoCapture cap(path);
    if (!cap.isOpened())
        std::cerr << "Не открылось видео: " << path << "\n";
    return cap;
}