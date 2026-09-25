#include "camera_thread.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>

void cameraThread(int index, CameraBuffer* buf, std::atomic<bool>* stop) {
    cv::VideoCapture cap;
#ifdef _WIN32
    cap.open(index, cv::CAP_DSHOW);
#else
    cap.open(index);
#endif
    if (!cap.isOpened()) {
        std::cerr << "Камера " << index << " не открылась\n";
        return;
    }

    cap.set(cv::CAP_PROP_FPS, 30);
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M','J','P','G'));
    cap.set(cv::CAP_PROP_BUFFERSIZE, 1);

    cv::Mat local;
    while (!stop->load(std::memory_order_relaxed)) {
        if (!cap.read(local) || local.empty()) continue;
        cv::flip(local, local, 0);
        buf->setFrame(local);
    }
}