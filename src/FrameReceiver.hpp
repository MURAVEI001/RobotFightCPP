#pragma once

#include <cstdint>
#include <string>
#include <thread>
#include <atomic>
#include <vector>

#include "FrameProcessor.hpp"

class FrameReceiver {
public:
    FrameReceiver(uint16_t port, FrameProcessor* processor);
    ~FrameReceiver();

    bool start();
    void stop();

private:
    uint16_t port_;
    FrameProcessor* processor_;
    int listen_sock_ = -1;
    std::atomic<bool> running_{false};
    std::vector<std::thread> client_threads_;

    void acceptLoop();
    void clientLoop(int client_sock, const std::string& client_id);
};