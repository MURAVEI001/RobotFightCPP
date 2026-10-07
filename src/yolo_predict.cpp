#include <iostream>
#include <vector>
#include <algorithm>

#include "yolo_predict.hpp"

// === Единственное определение глобального конфига ===
cfgYOLO cfg;

// === Ленивая инициализация сессии (один раз на весь процесс) ===
YoloSession& getYoloSession()
{
    static YoloSession instance;   // создаётся при первом вызове
    return instance;
}

bool yoloInference(cv::Mat& frame)
{
    if (frame.empty()) return false;

    // Получаем (или создаём при первом вызове) сессию
    YoloSession& ys = getYoloSession();

    int width  = frame.cols;
    int height = frame.rows;

    // === Предобработка ===
    cv::Mat resized, blob;
    cv::resize(frame, resized, {cfg.input_size, cfg.input_size});
    cv::dnn::blobFromImage(resized, blob, 1.0 / 255.0,
                           {cfg.input_size, cfg.input_size},
                           cv::Scalar(), true, false);

    std::vector<int64_t> shape = {1, 3, cfg.input_size, cfg.input_size};
    auto input_tensor = Ort::Value::CreateTensor<float>(
        ys.mem_info, blob.ptr<float>(), blob.total(),
        shape.data(), shape.size());

    const char* in_names[]  = {cfg.input_name.c_str()};
    const char* out_names[] = {cfg.output_name.c_str()};
    auto outputs = ys.session->Run(Ort::RunOptions{nullptr},
                                   in_names, &input_tensor, 1,
                                   out_names, 1);

    // === Разбор выхода [1, 84, 8400] ===
    float* data = outputs[0].GetTensorMutableData<float>();
    auto out_shape = outputs[0].GetTensorTypeAndShapeInfo().GetShape();
    int nc = (int)out_shape[1] - 4;
    int nb = (int)out_shape[2];

    std::vector<cv::Rect> boxes;
    std::vector<float>    confs;
    std::vector<int>      class_ids;

    float sx = (float)width  / cfg.input_size;
    float sy = (float)height / cfg.input_size;

    for (int i = 0; i < nb; ++i) {
        float best = 0.0f;
        int cls = -1;
        for (int c = 0; c < nc; ++c) {
            float v = data[(4 + c) * nb + i];
            if (v > best) { best = v; cls = c; }
        }
        if (best < cfg.conf_threshold) continue;

        float cx = data[0 * nb + i] * sx;
        float cy = data[1 * nb + i] * sy;
        float w  = data[2 * nb + i] * sx;
        float h  = data[3 * nb + i] * sy;

        boxes.emplace_back((int)(cx - w / 2), (int)(cy - h / 2), (int)w, (int)h);
        confs.push_back(best);
        class_ids.push_back(cls);
    }

    // === NMS ===
    std::vector<int> keep;
    cv::dnn::NMSBoxes(boxes, confs, cfg.conf_threshold, cfg.iou_threshold, keep);

    return !keep.empty();
}