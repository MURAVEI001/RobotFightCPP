#include "raspiGrid.hpp"

#include <opencv2/highgui.hpp>

#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

namespace {
std::atomic<bool> g_running{true};
void onSignal(int) { g_running = false; }
}

int main() {
    std::signal(SIGINT,  onSignal);
    std::signal(SIGTERM, onSignal);

    RaspiGrid::Config cfg;
    cfg.cell_width  = 1280;
    cfg.cell_height = 720;
    cfg.cols        = 4;
    cfg.rows        = 1;
    cfg.port        = 5000;

    cfg.camera_to_cell = {
        {0, 0}, // верх-лево
        {1, 1}, // верх-право
        {2, 2}, // середина-лево
        {3, 3}, // середина-право
        {4, 4}, // низ-лево
        {5, 5}, // низ-право
    };

    RaspiGrid grid(cfg);

    while (g_running) {
        grid.poll();
        cv::Mat& canvas = grid.grid();
        
        cv::imshow("raspi grid", canvas);
        if (cv::waitKey(1) == 27) break;

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "frames decoded: " << grid.framesDecoded() << "\n"
              << "frames dropped: " << grid.framesDropped() << "\n"
              << "datagrams:      " << grid.datagramsReceived() << "\n";
    return 0;
}