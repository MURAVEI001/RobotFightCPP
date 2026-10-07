#include "motion_detection.hpp"

#include <opencv2/opencv.hpp>

int motionDetect(cv::Mat& frame, cv::Mat& labels, cv::Mat& stats, cv::Mat& centroids, cv::Mat& morphFrame){
    cv::Mat blurFrame, mapMotion;
    static cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, {9,9});
    static cv::Ptr<cv::BackgroundSubtractorMOG2> mog2 = cv::createBackgroundSubtractorMOG2(300, 48.0, false);
    cv::GaussianBlur(frame, blurFrame, {15,15}, 0);
    mog2->apply(blurFrame, mapMotion, -1);
    cv::morphologyEx(mapMotion, morphFrame, cv::MORPH_CLOSE, kernel);
    cv::morphologyEx(morphFrame, morphFrame, cv::MORPH_OPEN, kernel);
    int numLabels = cv::connectedComponentsWithStats(morphFrame, labels, stats, centroids, 8, CV_32S);
    return numLabels;
}

void filterStats(int numLabels, cv::Mat& stats, int minArea, int maxArea,
                 std::vector<int>& keptLabels)
{
    keptLabels.clear();
    for (int i = 1; i < numLabels; ++i) {
        int area = stats.at<int>(i, cv::CC_STAT_AREA);
        if (area >= minArea && area <= maxArea)
            keptLabels.push_back(i);
    }
}

void getROI(cv::Mat& frame, cv::Mat& stats, std::vector<int>& keptLabels, std::vector<cv::Mat>& ROI, 
    std::vector<cv::Point2i>& anchors){
    for(int index : keptLabels){
        int x = stats.at<int>(index, cv::CC_STAT_LEFT);
        int y = stats.at<int>(index, cv::CC_STAT_TOP);
        int w = stats.at<int>(index, cv::CC_STAT_WIDTH);
        int h = stats.at<int>(index, cv::CC_STAT_HEIGHT);

        cv::Rect box(x, y, w, h);

        box &= cv::Rect(0, 0, frame.cols, frame.rows);
        if (box.empty()) continue;

        cv::Mat roi = frame(box);
        ROI.push_back(roi);
        anchors.emplace_back(box.x + box.width / 2,
                            box.y + box.height /2);
    }   
}