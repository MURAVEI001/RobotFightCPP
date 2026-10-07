cmake_minimum_required(VERSION 3.15)
project(ASRG)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Путь к ONNX Runtime (относительно CMakeLists.txt)
set(ONNXRUNTIME_ROOT "${CMAKE_SOURCE_DIR}/onnxruntime-osx-arm64-1.30.0")

# OpenCV через Homebrew
find_package(OpenCV REQUIRED)

include_directories(
    ${ONNXRUNTIME_ROOT}/include
    ${OpenCV_INCLUDE_DIRS}
)

link_directories(${ONNXRUNTIME_ROOT}/lib)

add_executable(ASRG
    src/mainVideo.cpp
    src/aruco_detection.cpp
    src/motion_detection.cpp
    src/yolo_predict.cpp)

target_link_libraries(ASRG
    onnxruntime
    ${OpenCV_LIBS}
)

# Чтобы при запуске находились .dylib из папки onnxruntime
set_target_properties(ASRG PROPERTIES
    BUILD_RPATH "${ONNXRUNTIME_ROOT}/lib"
    INSTALL_RPATH "${ONNXRUNTIME_ROOT}/lib"
)