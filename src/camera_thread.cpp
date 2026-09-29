#include "camera_thread.hpp"
#include "camera_source.hpp"
#include "config.hpp"
#include <chrono>
#include <thread>

void cameraThread(int index, CameraBuffer* buf, std::atomic<bool>* stop) {
    auto cap = openCamera(index);
    if (!cap.isOpened()) return;

    cv::Mat local;
    while (!stop->load(std::memory_order_relaxed)) {
        if (!cap.read(local) || local.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }
        if (cfg::g.camera.flipVertical)
            cv::flip(local, local, 0);

        buf->setFrame(local);
    }
}