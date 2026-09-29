#pragma once
#include <opencv2/opencv.hpp>
#include <string>

cv::VideoCapture openVideo(const std::string& path);