#include <opencv2/opencv.hpp>
#include <atomic>
#include <thread>
#include <vector>

#include "config.hpp"
#include "video_source.hpp"
#include "motion_detection.hpp"
#include "aruco_detection.hpp"
#include "visualize.hpp"

int main() {
    auto cap = openVideo(cfg::g.video.path);
    if (!cap.isOpened()) return -1;

    const cv::Rect roi(cfg::g.geometry.roiX,
                       cfg::g.geometry.roiY,
                       cfg::g.geometry.roiW,
                       cfg::g.geometry.roiH);

    MotionDetector     motion;
    ArucoMarkerDetector markerDetector(cfg::g.markers.arucoDictId);

    cv::Mat frame;
    while (cap.read(frame)) {
        cv::Mat crop = frame(roi);

        // 1. Маркеры
        auto markers = markerDetector.detect(crop);

        // 2. Движение
        cv::Mat morph, stats, centroids;
        int numLabels = motion.detect(crop, morph, stats, centroids);

        // 3. Отрисовка
        drawMotionBoxes(crop, stats, numLabels,
                        cfg::g.motion.minArea, cfg::g.motion.maxArea);
        drawMarkers(crop, markers, cfg::g.markers.targetMarkerId);

        // 4. Показ
        cv::imshow("crop",   crop);
        cv::imshow("motion", morph);

        if (cv::waitKey(1) == 27) break;
    }
    return 0;
}