#pragma once
#include <opencv2/opencv.hpp>
#include "aruco_detection.hpp"

// Рамки движения
void drawMotionBoxes(cv::Mat& frame,
                     const cv::Mat& stats,
                     int numLabels,
                     int minArea,
                     int maxArea);

// Маркеры ArUco
void drawMarkers(cv::Mat& frame,
                 const MarkerResult& markers,
                 int targetMarkerId);