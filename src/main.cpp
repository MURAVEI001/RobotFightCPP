#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <vector>
#include <thread>
#include <atomic>
#include "camera_buffer.hpp"
#include "camera_thread.hpp"
#include "grid_builder.hpp"
#include "motion_detection.hpp"

int main() {
    cv::VideoCapture cap("/Users/muravei/projects/RobotFightCPP/4.mp4");
    cv::Mat frame, cropFrame, stats, centroids, morphFrame;
    std::vector<int> markerIds;
    std::vector<std::vector<cv::Point2f>> markerCorners, rejected;
    cv::Rect roi(330, 90, 600, 600);
    MotionDetector detectorMotion;
    const int numCameras = 1;
    std::vector<CameraBuffer> buffers(numCameras);
    std::vector<std::thread> threads;
    std::atomic<bool> stop{false};

    cv::aruco::DetectorParameters detectorParams = cv::aruco::DetectorParameters();
    cv::aruco::Dictionary dictAruco = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_250);
    cv::aruco::ArucoDetector detector(dictAruco, detectorParams);

    // for (int i = 0; i < numCameras; ++i)
    //     threads.emplace_back(cameraThread, i, &buffers[i], &stop);

    // GridConfig cfg{1 , 1, 1920, 1080, 0};

    int area, numLabels, cx, cy;

    while (true) {
//      cv::Mat cropFrame = buildGrid(buffers, cfg);
        cap >> frame;
        cropFrame = frame(roi);
        detector.detectMarkers(cropFrame, markerCorners, markerIds, rejected);

        cv::Mat blurred, sharpened;
cv::GaussianBlur(cropFrame, blurred, cv::Size(0, 0), 5.0);

// sharpened = src * (1 + amount) - blurred * amount
double amount = 1.5; // сила резкости
cv::addWeighted(cropFrame, 1.0 + amount, blurred, -amount, 0, sharpened);

        numLabels = detectorMotion.detect(cropFrame, morphFrame, stats, centroids);
        for(int i = 1; i < numLabels; ++i){
            area = stats.at<int>(i, cv::CC_STAT_AREA);
            cx = centroids.at<double>(i, 0);
            cy = centroids.at<double>(i, 1);

            int x = stats.at<int>(i,cv::CC_STAT_LEFT);
            int y = stats.at<int>(i,cv::CC_STAT_TOP);
            int w = stats.at<int>(i,cv::CC_STAT_WIDTH);
            int h = stats.at<int>(i,cv::CC_STAT_HEIGHT);

            if(area <= 500 || area >= 5000){continue;}

            cv::rectangle(cropFrame, cv::Point2i(x,y), cv::Point2i(x+w,y+h),cv::Scalar(0,0,255),3);
        };

        for (size_t i{}; i < markerIds.size(); ++i) {
            if (markerIds[i] == 47) {
                const auto& c = markerCorners[i]; // 4 точки
                for (int j = 0; j < 4; ++j) {
                    cv::line(cropFrame, c[j], c[(j + 1) % 4], cv::Scalar(0, 0, 255), 2);
        }
            cv::Point2f topLeft = markerCorners[i][1];
            cv::Point2f topRight = markerCorners[i][0];

            double angleRad = atan2(topRight.y - topLeft.y, topRight.x - topLeft.x);
            double angleDeg = angleRad * 180.0 / CV_PI;

            cv::putText(cropFrame, std::to_string(markerIds[i]),
                    c[0], cv::FONT_HERSHEY_SIMPLEX, 0.6,
                    cv::Scalar(0, 255, 0), 2);
            cv::putText(cropFrame, std::to_string(angleDeg),
                    cv::Point2i(50,50), cv::FONT_HERSHEY_SIMPLEX, 0.6,
                    cv::Scalar(0, 255, 0), 2);
    }};

        cv::imshow("frame", morphFrame);
        cv::imshow("frame1", cropFrame);


        if (cv::waitKey(1) == 27) break;
    }
    stop = true;
    for (auto& t : threads) t.join();
    return 0;
}