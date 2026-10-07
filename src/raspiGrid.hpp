#pragma once

#include <opencv2/opencv.hpp>

#include <chrono>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

class RaspiGrid {
public:
    struct Config {
        // Геометрия одной ячейки.
        int cell_width  = 1920;
        int cell_height = 1080;

        // Форма сетки — ЛЮБАЯ (не обязательно квадратная).
        int cols = 4;
        int rows = 1;

        // UDP.
        uint16_t    port         = 5000;
        std::string bind_address = "0.0.0.0";

        // camera_id -> индекс ячейки (row * cols + col).
        // Если camera_id не указан — используется сам camera_id.
        std::map<uint16_t, int> camera_to_cell;

        // Сколько миллисекунд может висеть недособранный кадр,
        // прежде чем будет отброшен.
        int frame_timeout_ms = 200;
    };

    explicit RaspiGrid(const Config& cfg);
    ~RaspiGrid();

    RaspiGrid(const RaspiGrid&)            = delete;
    RaspiGrid& operator=(const RaspiGrid&) = delete;

    // Забирает все накопившиеся датаграммы, дособирает кадры,
    // раскладывает их по ячейкам. Не блокирует. Вызывать в цикле.
    void poll();

    // Полотно целиком: (rows * cell_height) × (cols * cell_width), CV_8UC3.
    cv::Mat&       grid()       { return grid_; }
    const cv::Mat& grid() const { return grid_; }

    // --- Доступ к ячейкам ---

    // Вид на ячейку по линейному индексу (row * cols + col).
    // Если idx вне диапазона — пустой Mat.
    cv::Mat cellView(int idx);

    // Вид на ячейку по (row, col). Если вне диапазона — пустой Mat.
    cv::Mat cellView(int row, int col);

    // Вид на ячейку по camera_id (через camera_to_cell).
    cv::Mat cellView(uint16_t camera_id);

    // --- Информация о сетке ---

    int cols()       const { return cfg_.cols; }
    int rows()       const { return cfg_.rows; }
    int cellsCount() const { return cfg_.cols * cfg_.rows; }

    // Меняет форму сетки. Пересоздаёт полотно, сбрасывает готовность
    // ячеек и незавершённые сборки. Ширина/высота ячейки не меняются.
    void resizeGrid(int cols, int rows);

    // Приходил ли когда-нибудь кадр от этой камеры.
    bool cellReady(uint16_t camera_id) const;

    // Метрики — удобно печатать в конце.
    uint64_t framesDecoded()      const { return frames_decoded_; }
    uint64_t framesDropped()      const { return frames_dropped_; }
    uint64_t datagramsReceived()  const { return datagrams_received_; }

private:
    struct FrameAssembly {
        bool         active       = false;
        uint32_t     frame_id     = 0;
        uint32_t     total_size   = 0;
        std::vector<uint8_t>  buffer;
        std::vector<bool>     chunk_seen;
        size_t       chunks_seen  = 0;
        size_t       chunks_total = 0;
        std::chrono::steady_clock::time_point started_at{};
    };

    void handleDatagram(const uint8_t* data, size_t len);
    void processAssembly(uint16_t camera_id, FrameAssembly& fa);
    void blitToGrid(uint16_t camera_id, const cv::Mat& img);
    int  cellIndexFor(uint16_t camera_id) const;

    // Пересоздаёт grid_ и cell_ready_ под текущие cfg_.cols / cfg_.rows.
    void rebuildGrid();

    Config  cfg_;
    int     fd_{-1};
    cv::Mat grid_;

    std::map<uint16_t, FrameAssembly> assemblies_;
    std::vector<bool>                 cell_ready_;

    uint64_t frames_decoded_     = 0;
    uint64_t frames_dropped_     = 0;
    uint64_t datagrams_received_ = 0;
};