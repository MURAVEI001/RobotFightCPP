#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <vector>

struct Marker {
    const int id = 47;
    float angle = 0.0f;              
    bool  found = false;                
    std::vector<cv::Point2f> corners;       
};

float computeMarkerAngle(const std::vector<cv::Point2f>& c);