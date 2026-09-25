#include <opencv2/opencv.hpp>
#include <vector>
#include <thread>
#include <atomic>
#include "camera_buffer.hpp"
#include "camera_thread.hpp"
#include "grid_builder.hpp"

int main() {
    const int numCameras = 3;
    std::vector<CameraBuffer> buffers(numCameras);
    std::vector<std::thread> threads;
    std::atomic<bool> stop{false};

    for (int i = 0; i < numCameras; ++i)
        threads.emplace_back(cameraThread, i, &buffers[i], &stop);

    GridConfig cfg{ 3, 1, 1920, 1080, 0 };

    while (true) {
        cv::Mat grid = buildGrid(buffers, cfg);
        cv::imshow("Grid", grid);
        if (cv::waitKey(1) == 27) break;
    }

    stop = true;
    for (auto& t : threads) t.join();
    return 0;
}