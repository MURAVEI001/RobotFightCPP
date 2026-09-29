#include "motion_detection.hpp"
#include "config.hpp"

MotionDetector::MotionDetector()
    : mog_(cv::createBackgroundSubtractorMOG2(
          cfg::g.motion.mogHistory,
          cfg::g.motion.mogVarThreshold,
          cfg::g.motion.mogDetectShadows)),
      kernel_(cv::getStructuringElement(
          cv::MORPH_RECT,
          {cfg::g.motion.morphKernel, cfg::g.motion.morphKernel})) {}

int MotionDetector::detect(cv::InputArray src,
                           cv::OutputArray morphFrame,
                           cv::OutputArray stats,
                           cv::OutputArray centroids) {
    const int k = cfg::g.motion.blurKernel;
    cv::GaussianBlur(src, blurFrame_, {k, k}, 0);
    mog_->apply(blurFrame_, mapMotion_);

    cv::morphologyEx(mapMotion_, morphFrame, cv::MORPH_CLOSE, kernel_);
    cv::morphologyEx(morphFrame, morphFrame, cv::MORPH_OPEN,  kernel_);

    return cv::connectedComponentsWithStats(
        morphFrame, labels_, stats, centroids, 8, CV_32S);
}