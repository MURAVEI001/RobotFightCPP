#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>

struct CameraBuffer {
    cv::Mat frame;
    std::mutex mtx;
    bool hasNew = false;
};

void cameraThread(int index, CameraBuffer* buf, std::atomic<bool>* stop) {
    cv::VideoCapture cap;
#ifdef _WIN32
    cap.open(index, cv::CAP_DSHOW);
#else
    cap.open(index);
#endif
    if (!cap.isOpened()) {
        std::cerr << "Камера " << index << " не открылась\n";
        return;
    }

    cap.set(cv::CAP_PROP_FPS, 30);
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M','J','P','G'));
    cap.set(cv::CAP_PROP_BUFFERSIZE, 1);

    cv::Mat local;
    while (!stop->load()) {
        if (!cap.read(local) || local.empty()) continue;

        cv::flip(local, local, 0);

        {
            std::lock_guard<std::mutex> lock(buf->mtx);
            cv::swap(buf->frame, local);
            buf->hasNew = true;
        }
    }
}

cv::Mat buildGrid(std::vector<CameraBuffer>& buffers, int rows, int cols, int cellW, int cellH, int gap = 0)
{
    const int totalW = cols * cellW + (cols - 1) * gap;
    const int totalH = rows * cellH + (rows - 1) * gap;

    cv::Mat canvas(totalH, totalW, CV_8UC3, cv::Scalar(0, 0, 0));

    const int numCameras = static_cast<int>(buffers.size());

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            const int idx = r * cols + c;
            if (idx >= numCameras) continue;

            cv::Mat frame;
            {
                std::lock_guard<std::mutex> lock(buffers[idx].mtx);
                if (buffers[idx].frame.empty()) continue;
                frame = buffers[idx].frame.clone();   // независимая копия — нет мерцания
                buffers[idx].hasNew = false;
            }

            cv::Mat resized;
            if (frame.size() != cv::Size(cellW, cellH)) {
                cv::resize(frame, resized, cv::Size(cellW, cellH), 0, 0, cv::INTER_LINEAR);
            } else {
                resized = frame;
            }

            const int x = c * (cellW + gap);
            const int y = r * (cellH + gap);
            resized.copyTo(canvas(cv::Rect(x, y, cellW, cellH)));
        }
    }

    return canvas;
}

int main() {
    const int numCameras = 3;
    std::vector<CameraBuffer> buffers(numCameras);
    std::vector<std::thread> threads;
    std::atomic<bool> stop{false};

    for (int i = 0; i < numCameras; i++)
        threads.emplace_back(cameraThread, i, &buffers[i], &stop);

    int gridRows = 3;
    int gridCols = 1;

    const int cellW = 1920;
    const int cellH = 1080;
    const int gap   = 0;

    while (true) {
        cv::Mat grid = buildGrid(buffers, gridRows, gridCols, cellW, cellH, gap);
        
        cv::imshow("Grid", grid);
        if (cv::waitKey(1) == 27) break;
    }

    stop = true;
    for (auto& t : threads) t.join();
    return 0;
}