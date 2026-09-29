#pragma once
#include <opencv2/opencv.hpp>

class MotionDetector {
public:
    MotionDetector();

    int detect(cv::InputArray src,
               cv::OutputArray morphFrame,
               cv::OutputArray stats,
               cv::OutputArray centroids);

private:
    cv::Ptr<cv::BackgroundSubtractorMOG2> mog_;
    cv::Mat kernel_;
    cv::Mat blurFrame_;
    cv::Mat mapMotion_;
    cv::Mat labels_;
};