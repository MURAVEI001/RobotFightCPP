#pragma once
#include <opencv2/core.hpp>
#include <mutex>
#include <atomic>
#include <chrono>
#include <cstdint>

namespace viewer {

// Потокобезопасный буфер "последний кадр". Сеть пишет, UI читает.
// Старые кадры перезаписываются — не копим очередь, показываем самое свежее.
class FrameBuffer {
public:
    // Состояние соединения для индикации в UI
    enum class State : int {
        DISCONNECTED = 0,
        CONNECTING   = 1,
        STREAMING    = 2,
        ERROR        = 3,
    };

    void set_frame(const cv::Mat& frame) {
        std::lock_guard<std::mutex> lk(mtx_);
        frame.copyTo(frame_); // глубокая копия, чтобы поток-приёмник мог сразу отпустить свой Mat
        frames_received_++;
        last_frame_time_ = std::chrono::steady_clock::now();
    }

    // Забирает последний кадр. Возвращает false, если кадра ещё не было.
    bool get_frame(cv::Mat& out) {
        std::lock_guard<std::mutex> lk(mtx_);
        if (frame_.empty()) return false;
        frame_.copyTo(out);
        return true;
    }

    void set_state(State s) {
        state_.store(static_cast<int>(s), std::memory_order_relaxed);
    }
    State state() const {
        return static_cast<State>(state_.load(std::memory_order_relaxed));
    }

    uint64_t frames_received() const {
        std::lock_guard<std::mutex> lk(mtx_);
        return frames_received_;
    }

    // Снимок для UI: сколько кадров и когда последний раз приходил кадр
    struct Stats {
        uint64_t frames = 0;
        std::chrono::steady_clock::time_point last{};
    };
    Stats stats() const {
        std::lock_guard<std::mutex> lk(mtx_);
        return Stats{frames_received_, last_frame_time_};
    }

    // Сбрасываем при переподключении
    void reset_frames() {
        std::lock_guard<std::mutex> lk(mtx_);
        frames_received_ = 0;
        last_frame_time_ = {};
    }

private:
    mutable std::mutex mtx_;
    cv::Mat  frame_;
    uint64_t frames_received_ = 0;
    std::chrono::steady_clock::time_point last_frame_time_{};
    std::atomic<int> state_{static_cast<int>(State::DISCONNECTED)};
};

} // namespace viewer