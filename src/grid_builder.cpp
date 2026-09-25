#include "grid_builder.hpp"
#include <opencv2/opencv.hpp>

cv::Mat buildGrid(std::vector<CameraBuffer>& buffers, const GridConfig& cfg) {
    const int totalW = cfg.cols * cfg.cellW + (cfg.cols - 1) * cfg.gap;
    const int totalH = cfg.rows * cfg.cellH + (cfg.rows - 1) * cfg.gap;

    cv::Mat canvas(totalH, totalW, CV_8UC3, cv::Scalar(0, 0, 0));

    const int numCameras = static_cast<int>(buffers.size());

    for (int r = 0; r < cfg.rows; ++r) {
        for (int c = 0; c < cfg.cols; ++c) {
            const int idx = r * cfg.cols + c;
            if (idx >= numCameras) continue;

            cv::Mat frame;
            if (!buffers[idx].tryGetFrame(frame)) continue;

            cv::Mat resized;
            if (frame.size() != cv::Size(cfg.cellW, cfg.cellH)) {
                cv::resize(frame, resized, cv::Size(cfg.cellW, cfg.cellH),
                           0, 0, cv::INTER_LINEAR);
            } else {
                resized = frame;
            }

            const int x = c * (cfg.cellW + cfg.gap);
            const int y = r * (cfg.cellH + cfg.gap);
            resized.copyTo(canvas(cv::Rect(x, y, cfg.cellW, cfg.cellH)));
        }
    }
    return canvas;
}