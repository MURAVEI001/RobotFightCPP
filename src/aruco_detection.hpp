#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <vector>

struct MarkerResult {
    std::vector<int> ids;
    std::vector<std::vector<cv::Point2f>> corners;
    std::vector<double> anglesDeg;
};

class ArucoMarkerDetector {
public:
    explicit ArucoMarkerDetector(int dictId);

    MarkerResult detect(const cv::Mat& frame);
    const MarkerResult& last() const { return last_; }

private:
    cv::aruco::Dictionary         dict_;
    cv::aruco::DetectorParameters params_;
    cv::aruco::ArucoDetector      detector_;
    MarkerResult                  last_;
};