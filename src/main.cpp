#include <opencv2/opencv.hpp>
#include <vector>
#include <thread>
#include <atomic>
#include "camera_buffer.hpp"
#include "camera_thread.hpp"
#include "grid_builder.hpp"

cv::VideoCapture cap("/Users/muravei/projects/RobotFightCPP/4.mp4");
cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT,cv::Size(15,15));
cv::Mat frame, cropFrame, blurFrame, dilateFrame, mapMotion, labels, stats, centroids;

int main() {
    cv::Ptr<cv::BackgroundSubtractorMOG2> mog = cv::createBackgroundSubtractorMOG2(400, 48.0, false);
    cv::Rect roi(330, 90, 600, 600);
    const int numCameras = 3;
    std::vector<CameraBuffer> buffers(numCameras);
    std::vector<std::thread> threads;
    std::atomic<bool> stop{false};

    for (int i = 0; i < numCameras; ++i)
        threads.emplace_back(cameraThread, i, &buffers[i], &stop);

    GridConfig cfg{ 3, 1, 1920, 1080, 0 };

    int area, x, y, w, h;
    double cx, cy;

    while (true) {
        cv::Mat cropFrame = buildGrid(buffers, cfg);
        // cap >> frame;
        // cropFrame = frame(roi);

        cv::GaussianBlur(cropFrame,blurFrame,cv::Size(13,13),0);
        mog->apply(blurFrame,mapMotion);
        cv::dilate(mapMotion,dilateFrame,kernel);
        int numLabels = cv::connectedComponentsWithStats(dilateFrame,labels,stats,centroids, 8, CV_32S);

        for(int i = 1; i < numLabels; ++i){
            area = stats.at<int>(i, cv::CC_STAT_AREA);
            x = stats.at<int>(i, cv::CC_STAT_LEFT);
            y = stats.at<int>(i, cv::CC_STAT_TOP);
            w = stats.at<int>(i, cv::CC_STAT_WIDTH);
            h = stats.at<int>(i, cv::CC_STAT_HEIGHT);

            cx = centroids.at<double>(i, 0);
            cy = centroids.at<double>(i, 1);
            
            if(area <= 200 && area >= 600){continue;}

            cv::circle(cropFrame, cv::Point(cvRound(cx), cvRound(cy)), 3, cv::Scalar(0,0,255),-1);

        }

        cv::imshow("frame", dilateFrame);
        cv::imshow("frame1", cropFrame);


        if (cv::waitKey(1) == 27) break;
    }
    stop = true;
    for (auto& t : threads) t.join();
    return 0;
}