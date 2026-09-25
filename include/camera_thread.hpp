#pragma once
#include <atomic>
#include "camera_buffer.hpp"

void cameraThread(int index, CameraBuffer* buf, std::atomic<bool>* stop);