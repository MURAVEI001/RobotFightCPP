#include <opencv2/opencv.hpp>
#include <iostream>
#include <random>

cv::VideoCapture cap("/Users/muravei/projects/RobotFightCPP/4.mp4");
cv::Mat frame, fgmask, dst, blur_frame, kernel, labels, stats, centroids, output, crop_frame;
cv::Ptr<cv::BackgroundSubtractorKNN> subsctractorKNN = cv::createBackgroundSubtractorKNN(100, 500.0, false);
cv::KalmanFilter kf(4,2,0);
float dt = 1.0f;

struct Robot {
    float x;
    float y;
};

Robot robot1{0, 0};
Robot robot2{0, 0};

void addRobot(int index) {
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(50, 255);

    int area = stats.at<int>(index, cv::CC_STAT_AREA);
    if (area < 500 || area > 4000) return;

    cv::Vec3b color(
        static_cast<uchar>(dist(rng)),
        static_cast<uchar>(dist(rng)),
        static_cast<uchar>(dist(rng))
    );
    output.setTo(color, labels == index);

    int x = stats.at<int>(index, cv::CC_STAT_LEFT);
    int y = stats.at<int>(index, cv::CC_STAT_TOP);
    int w = stats.at<int>(index, cv::CC_STAT_WIDTH);
    int h = stats.at<int>(index, cv::CC_STAT_HEIGHT);

    cv::rectangle(output, cv::Rect(x, y, w, h), cv::Scalar(255, 255, 255), 1);

    double cx = centroids.at<double>(index, 0);
    double cy = centroids.at<double>(index, 1);
    cv::circle(output, cv::Point(cvRound(cx), cvRound(cy)), 3, cv::Scalar(0, 0, 255), -1);

    if (index == 1) {
        robot1.x = static_cast<int>(cx);
        robot1.y = static_cast<int>(cy);
    } else {
        robot2.x = static_cast<int>(cx);
        robot2.y = static_cast<int>(cy);
    }
}

int main() {
    kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    kf.transitionMatrix = (cv::Mat_<float>(4,4) << 
        1, 0, dt, 0,
        0, 1, 0, dt,
        0, 0, 1, 0,
        0, 0, 0, 1);
    
        cv::setIdentity(kf.measurementMatrix);
        cv::setIdentity(kf.processNoiseCov, cv::Scalar::all(1e-2));
        cv::setIdentity(kf.measurementNoiseCov, cv::Scalar::all(1e-1));

         kf.statePost = (cv::Mat_<float>(4, 1) << 0, 0, 0, 0);

        cv::Mat measurement(2, 1, CV_32F); // Входное измерение (x, y)
        cv::Mat prediction;                // Выход: предсказание
        cv::Mat corrected; 

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        cv::Rect roi(330, 90, 600, 600);

        crop_frame = frame(roi);

        cv::GaussianBlur(crop_frame, blur_frame, cv::Size(7, 7), 0);
        subsctractorKNN->apply(blur_frame, fgmask);
        cv::dilate(fgmask, dst, kernel);

        int num_labels = cv::connectedComponentsWithStats(
            dst, labels, stats, centroids, 8, CV_32S);

        output = cv::Mat::zeros(crop_frame.size(), CV_8UC3);

        for (int i = 1; i < num_labels; i++) {
            addRobot(i);
        }
        measurement.at<float>(0) = robot1.x;
        measurement.at<float>(1) = robot1.y;

        prediction = kf.predict();

        corrected = kf.correct(measurement);
        cv::circle(crop_frame, cv::Point(corrected.at<float>(0,0),corrected.at<float>(1,0)),5 , cv::Scalar(255, 255, 255), -1);
 

        if (!output.empty()) {
            cv::imshow("Components", output);
            cv::imshow("Comp3r2onents", crop_frame);

        }

        if (cv::waitKey(30) == 27) break;
    }

    cap.release();
    cv::destroyAllWindows();
}