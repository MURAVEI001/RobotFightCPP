#pragma once
#include <vector>
#include "camera_buffer.hpp"

struct GridConfig {
    int rows;
    int cols;
    int cellW;
    int cellH;
    int gap = 0;
};

cv::Mat buildGrid(std::vector<CameraBuffer>& buffers, const GridConfig& cfg);