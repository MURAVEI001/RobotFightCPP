#include "camera_source.hpp"
#include "config.hpp"
#include <iostream>

cv::VideoCapture openCamera(int index) {
    cv::VideoCapture cap;

#ifdef _WIN32
    cap.open(index, cv::CAP_DSHOW);
#else
    cap.open(index);
#endif

    if (!cap.isOpened()) {
        std::cerr << "Камера " << index << " не открылась\n";
        return cap;
    }

    const auto& c = cfg::g.camera;
    cap.set(cv::CAP_PROP_FPS, c.targetFps);
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc(
        c.fourcc[0], c.fourcc[1], c.fourcc[2], c.fourcc[3]));
    cap.set(cv::CAP_PROP_BUFFERSIZE, c.bufferSize);

    return cap;
}