#include <opencv2/opencv.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/video/background_segm.hpp>
#include <iostream>

cv::TermCriteria criteria(
        cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER,
        100,    // максимум итераций
        0.1     // порог смещения центров (в пикселях)
    );

int main(){
    cv::VideoCapture cap("/Users/muravei/projects/RobotFightCPP/4.mp4");
    cv::Mat frame,fgmask,dst,blur_frame,kernel;
    cv::Ptr<cv::BackgroundSubtractorKNN> subsctractorKNN = cv::createBackgroundSubtractorKNN(100,500.0, false );  
    kernel = cv::getStructuringElement(cv::MORPH_RECT,cv::Size(3,3));

    while(true){
        cap >> frame;
        cv::Rect roi(330, 90, 600, 600);
        cv::Mat crop_frame = frame(roi);
        cv::GaussianBlur(crop_frame,blur_frame,cv::Size(7,7),0);
        subsctractorKNN->apply(blur_frame,fgmask);
        cv::dilate(fgmask,dst,kernel);

        cv::imshow("frame2", dst);
        cv::imshow("frame", fgmask);
        cv::imshow("frame1", crop_frame);

        if(cv::waitKey(30) == 27) break;
    }
    cap.release();
    cv::destroyAllWindows();
}