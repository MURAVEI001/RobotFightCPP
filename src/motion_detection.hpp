#pragma once

#include <opencv2/opencv.hpp>

int motionDetect(cv::Mat& frame, cv::Mat& labels, cv::Mat& stats, cv::Mat& centroids, 
                cv::Mat& morphFrame);

void filterStats(int numLabels, cv::Mat& stats, int minArea, int maxArea,
                 std::vector<int>& keptLabels);

void getROI(cv::Mat& frame, cv::Mat& stats, std::vector<int>& keptLabels, 
                std::vector<cv::Mat>& ROI, std::vector<cv::Point2i>& anchors);
