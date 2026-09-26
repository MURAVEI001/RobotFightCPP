#include <opencv2/opencv.hpp>
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
    cv::Rect roi(330, 90, 600, 600);
    MotionDetector detector;
    const int numCameras = 1;
    std::vector<CameraBuffer> buffers(numCameras);
    std::vector<std::thread> threads;
    std::atomic<bool> stop{false};

    // for (int i = 0; i < numCameras; ++i)
    //     threads.emplace_back(cameraThread, i, &buffers[i], &stop);

    // GridConfig cfg{1 , 1, 1920, 1080, 0 };

    int area, x, y, w, h, numLabels;
    double cx, cy;

    while (true) {
        //cv::Mat cropFrame = buildGrid(buffers, cfg);
        cap >> frame;
        cropFrame = frame(roi);

        numLabels = detector.detect(cropFrame, morphFrame, stats, centroids);
        for(int i = 1; i < numLabels; ++i){
            area = stats.at<int>(i, cv::CC_STAT_AREA);
            x = stats.at<int>(i, cv::CC_STAT_LEFT);
            y = stats.at<int>(i, cv::CC_STAT_TOP);
            w = stats.at<int>(i, cv::CC_STAT_WIDTH);
            h = stats.at<int>(i, cv::CC_STAT_HEIGHT);

            cx = centroids.at<double>(i, 0);
            cy = centroids.at<double>(i, 1);
            
            if(area <= 500 || area >= 5000){continue;}

            cv::circle(cropFrame, cv::Point(cvRound(cx), cvRound(cy)), 5, cv::Scalar(0,0,255),-1);

        }

        cv::imshow("frame", morphFrame);
        cv::imshow("frame1", cropFrame);


        if (cv::waitKey(1) == 27) break;
    }
    stop = true;
    for (auto& t : threads) t.join();
    return 0;
}