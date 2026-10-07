#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

#include "motion_detection.hpp"
#include "aruco_detection.hpp"
#include "yolo_predict.hpp"

int main() {
    cv::VideoCapture cap("/Users/muravei/projects/RobotFightCPP/media/4.mp4");

    if (!cap.isOpened()) {
        std::cerr << "Не удалось открыть видео\n";
        return -1;
    }

    // --- ArUco ---
    cv::aruco::Dictionary dictionary =
        cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_250);
    cv::aruco::DetectorParameters params;
    cv::aruco::ArucoDetector arucoDetector(dictionary, params);

    // --- Буферы ---
    cv::Mat frame, morphFrame, labels, stats, centroids, grayFrame;

    std::vector<cv::Mat> ROIs;
    std::vector<int> keptLabels;
    std::vector<cv::Point2i> anchors;

    std::vector<std::vector<cv::Point2f>> detectedCorners;
    std::vector<int> detectedIds;

    Marker selfRobot;
    bool findEnemy;
    
    while (cap.read(frame)) {
        // --- 1. Очистка per-frame контейнеров ---
        ROIs.clear();
        keptLabels.clear();
        anchors.clear();

        // --- 2. Motion detection ---
        int numLabels = motionDetect(frame, labels, stats, centroids, morphFrame);
        filterStats(numLabels, stats, 500, 5000, keptLabels);
        getROI(frame, stats, keptLabels, ROIs, anchors);

        // --- 3. Поиск маркера ---
        bool foundThisFrame = false;
        cv::Mat foundRoi;

        for (auto& roi : ROIs) {
            detectedCorners.clear();
            detectedIds.clear();
            arucoDetector.detectMarkers(roi, detectedCorners, detectedIds);

            for (size_t i = 0; i < detectedIds.size(); ++i) {
                if (detectedIds[i] == selfRobot.id) {
                    selfRobot.corners = detectedCorners[i];         
                    selfRobot.angle   = computeMarkerAngle(detectedCorners[i]);
                    foundThisFrame    = true;
                    foundRoi          = roi;
                    break;                                                
                }
            }
            findEnemy = yoloInference(roi);
            if (findEnemy) break;
        }

        // --- 4. Обновление состояния ---
        selfRobot.found = foundThisFrame;

        // // --- 5. Отладочный вывод ---
        // if (selfRobot.found) {
        //     std::cout << "Marker " << selfRobot.id
        //               << " FOUND, angle = " << selfRobot.angle << " deg, corners: ";
        //     for (const auto& p : selfRobot.corners)
        //         std::cout << "(" << p.x << ", " << p.y << ") ";
        //     std::cout << "\n";
        // } else if (!selfRobot.corners.empty()) {
        //     std::cout << "Marker " << selfRobot.id
        //               << " NOT found, last angle = " << selfRobot.angle << " deg\n";
        // } else {
        //     std::cout << "Marker " << selfRobot.id << " never seen\n";
        // }

        // --- 6. Визуализация ---
        if (!foundRoi.empty()) {
            cv::Mat big;
            cv::resize(foundRoi, big, cv::Size(), 4.0, 4.0, cv::INTER_NEAREST);
            cv::imshow("aruco", big);
        }

        cv::imshow("frame", frame);
        cv::imshow("motion", morphFrame);

        if (cv::waitKey(1) == 27) break;
    }
}