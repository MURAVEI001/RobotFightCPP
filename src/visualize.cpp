#include "visualize.hpp"
#include "config.hpp"

void drawMotionBoxes(cv::Mat& frame,
                     const cv::Mat& stats,
                     int numLabels,
                     int minArea,
                     int maxArea) {
    for (int i = 1; i < numLabels; ++i) {
        int area = stats.at<int>(i, cv::CC_STAT_AREA);
        if (area <= minArea || area >= maxArea) continue;

        int x = stats.at<int>(i, cv::CC_STAT_LEFT);
        int y = stats.at<int>(i, cv::CC_STAT_TOP);
        int w = stats.at<int>(i, cv::CC_STAT_WIDTH);
        int h = stats.at<int>(i, cv::CC_STAT_HEIGHT);

        cv::rectangle(frame, {x, y}, {x + w, y + h},
                      cfg::g.visual.boxColor, 3);
    }
}

void drawMarkers(cv::Mat& frame,
                 const MarkerResult& markers,
                 int targetMarkerId) {
    const auto& v = cfg::g.visual;

    for (size_t i = 0; i < markers.ids.size(); ++i) {
        if (markers.ids[i] != targetMarkerId) continue;

        const auto& c = markers.corners[i];

        // Контур маркера
        for (int j = 0; j < 4; ++j)
            cv::line(frame, c[j], c[(j + 1) % 4],
                     v.boxColor, cfg::g.markers.lineThickness);

        // ID
        cv::putText(frame, std::to_string(markers.ids[i]),
                    c[0], cv::FONT_HERSHEY_SIMPLEX,
                    cfg::g.markers.textScale, v.textColor,
                    cfg::g.markers.textThickness);

        // Угол
        cv::putText(frame, std::to_string(markers.anglesDeg[i]),
                    v.angleTextPos, cv::FONT_HERSHEY_SIMPLEX,
                    cfg::g.markers.textScale, v.textColor,
                    cfg::g.markers.textThickness);
    }
}