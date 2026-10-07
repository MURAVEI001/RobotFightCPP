#pragma once
#include <opencv2/opencv.hpp>
#include <onnxruntime_cxx_api.h>
#include <string>
#include <memory>

struct cfgYOLO
{
    std::string model_path = "/Users/muravei/projects/RobotFightCPP/best.onnx";
    float conf_threshold = 0.25f;
    float iou_threshold  = 0.45f;
    int   input_size     = 640;

    OrtLoggingLevel log_level = ORT_LOGGING_LEVEL_WARNING;
    std::string session_name = "yolo";
    GraphOptimizationLevel opt_level = GraphOptimizationLevel::ORT_ENABLE_ALL;

    std::string provider_name = "CoreML";
    std::string model_format  = "MLProgram";
    std::string compute_units = "ALL";

    std::string input_name  = "images";
    std::string output_name = "output0";
};

// Глобальный конфиг (extern-объявление, определение — в одном .cpp)
extern cfgYOLO cfg;

// Структура-обёртка для YOLO-сессии
struct YoloSession
{
    Ort::Env env;
    Ort::SessionOptions opts;
    std::unique_ptr<Ort::Session> session;   // создаётся лениво при первом вызове
    Ort::MemoryInfo mem_info;

    YoloSession()
        : env(cfg.log_level, cfg.session_name.c_str()),
          mem_info(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault))
    {
        opts.SetGraphOptimizationLevel(cfg.opt_level);
        opts.AppendExecutionProvider(cfg.provider_name.c_str(), {
            {"ModelFormat",    cfg.model_format},
            {"MLComputeUnits", cfg.compute_units}
        });
        session = std::make_unique<Ort::Session>(env, cfg.model_path.c_str(), opts);
    }
};

// Возвращает ссылку на единственный экземпляр сессии (создаётся при первом вызове)
YoloSession& getYoloSession();

// Инференс: возвращает true, если найден хотя бы один объект
bool yoloInference(cv::Mat& frame);