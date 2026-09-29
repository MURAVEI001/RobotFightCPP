#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <string>

namespace cfg {

// ─── Источник видео/камеры ────────────────────────────────
struct VideoSource {
    std::string path = "/Users/muravei/projects/RobotFightCPP/media/4.mp4";
};

struct Camera {
    int  targetFps    = 30;
    int  bufferSize   = 1;
    bool flipVertical = false;
    const char* fourcc = "MJPG";
};

// ─── ROI и сетка ──────────────────────────────────────────
struct Geometry {
    int roiX = 330;
    int roiY = 90;
    int roiW = 600;
    int roiH = 600;

    int gridRows  = 1;
    int gridCols  = 1;
    int gridCellW = 1920;
    int gridCellH = 1080;
    int gridGap   = 0;
};

// ─── Детекция движения ────────────────────────────────────
struct Motion {
    int    mogHistory        = 400;
    double mogVarThreshold   = 48.0;
    bool   mogDetectShadows  = false;

    int    morphKernel       = 15;
    int    blurKernel        = 13;

    int    minArea           = 500;
    int    maxArea           = 5000;
};

// ─── Детекция ArUco ───────────────────────────────────────
struct Markers {
    int arucoDictId    = cv::aruco::DICT_4X4_250;
    int targetMarkerId = 47;

    int    lineThickness = 2;
    int    textThickness = 2;
    double textScale     = 0.6;
};

// ─── Визуализация ─────────────────────────────────────────
struct Visual {
    cv::Scalar boxColor     = {0, 0, 255};
    cv::Scalar textColor    = {0, 255, 0};
    cv::Point  angleTextPos = {50, 50};
};

// ─── Корневой конфиг ──────────────────────────────────────
struct Config {
    VideoSource video;
    Camera      camera;
    Geometry    geometry;
    Motion      motion;
    Markers     markers;
    Visual      visual;
};

// Если хочешь менять на лету — убери const
inline Config g{};

} // namespace cfg