#include "aruco_detection.hpp"

float computeMarkerAngle(const std::vector<cv::Point2f>& c) {
    if (c.size() < 4) return 0.0f;
    float dx = c[3].x - c[0].x;
    float dy = c[3].y - c[0].y;
    float angle = std::atan2(dy, dx) * 180.0f / static_cast<float>(CV_PI);
    if (angle < 0.0f) angle += 360.0f;
    return angle;
}