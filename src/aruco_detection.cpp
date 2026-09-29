#include "aruco_detection.hpp"

ArucoMarkerDetector::ArucoMarkerDetector(int dictId)
    : dict_(cv::aruco::getPredefinedDictionary(dictId)),
      params_(cv::aruco::DetectorParameters()),
      detector_(dict_, params_) {}

MarkerResult ArucoMarkerDetector::detect(const cv::Mat& frame) {
    MarkerResult res;
    std::vector<std::vector<cv::Point2f>> rejected;

    detector_.detectMarkers(frame, res.corners, res.ids, rejected);

    res.anglesDeg.reserve(res.ids.size());
    for (const auto& c : res.corners) {
        const cv::Point2f& tl = c[1];
        const cv::Point2f& tr = c[0];
        double rad = std::atan2(tr.y - tl.y, tr.x - tl.x);
        res.anglesDeg.push_back(rad * 180.0 / CV_PI);
    }

    last_ = res;
    return res;
}