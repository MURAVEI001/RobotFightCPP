#include "motion_detection.hpp"

MotionDetector::MotionDetector()
    : mog(cv::createBackgroundSubtractorMOG2(400, 48.0, false)),
      kernel(cv::getStructuringElement(cv::MORPH_RECT, cv::Size(15, 15)))
{}

int MotionDetector::detect(cv::InputArray src, cv::OutputArray morphFrame, cv::OutputArray stats, cv::OutputArray centroids) {
    cv::GaussianBlur(src, blurFrame, cv::Size(13, 13), 0);
    mog->apply(blurFrame, mapMotion);
    cv::morphologyEx(mapMotion, morphFrame, cv::MORPH_CLOSE, kernel);
    cv::morphologyEx(morphFrame, morphFrame, cv::MORPH_OPEN, kernel);
    return cv::connectedComponentsWithStats(morphFrame, labels, stats, centroids, 8, CV_32S);
}