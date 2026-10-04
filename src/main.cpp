#include "FrameReceiver.hpp"
#include "FrameProcessor.hpp"

#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <atomic>

#include <opencv2/highgui.hpp>

static std::atomic<bool> g_running{true};

static void signalHandler(int) { g_running = false; }

int main(int argc, char** argv) {
    uint16_t port = 9000;
    if (argc > 1) port = static_cast<uint16_t>(std::atoi(argv[1]));

    signal(SIGINT,  signalHandler);
    signal(SIGTERM, signalHandler);

    FrameProcessor processor;
    FrameReceiver  receiver(port, &processor);

    if (!receiver.start()) {
        fprintf(stderr, "Не удалось запустить сервер на порту %u\n", port);
        return 1;
    }

    printf("RobotFightCPP сервер запущен на порту %u.\n", port);
    printf("Ожидание камер. ESC или q — выход.\n");

    // ВАЖНО: imshow/waitKey должны выполняться в главном потоке (macOS/Cocoa).
    while (g_running) {
        if (!processor.renderAndShow(/*tile*/640, 360, /*cols*/2, /*fps*/30)) {
            break;
        }
    }

    receiver.stop();
    cv::destroyAllWindows();
    printf("RobotFightCPP завершён.\n");
    return 0;
}