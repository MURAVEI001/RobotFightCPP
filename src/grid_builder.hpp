#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include "camera_buffer.hpp"

struct GridConfig {
    int rows;
    int cols;
    int cellW;
    int cellH;
    int gap;
};

cv::Mat buildGrid(std::vector<CameraBuffer>& buffers, const GridConfig& cfg);