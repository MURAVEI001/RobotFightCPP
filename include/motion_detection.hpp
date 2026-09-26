#pragma once
#include <opencv2/opencv.hpp>

class MotionDetector {
public:
    MotionDetector();

    int detect(cv::InputArray src, cv::OutputArray morphFrame, cv::OutputArray stats, cv::OutputArray centroids);

private:
    cv::Ptr<cv::BackgroundSubtractorMOG2> mog;
    cv::Mat kernel, blurFrame, mapMotion, labels;
};