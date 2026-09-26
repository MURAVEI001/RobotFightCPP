cmake_minimum_required(VERSION 3.20)
project(ASRG CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Оптимизация под Apple Silicon
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
    add_compile_options(-O3 -mcpu=apple-m1)  # M5 тоже поддерживает этот флаг
endif()

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

add_executable(ASRG
    src/main.cpp
    src/camera_thread.cpp
    src/grid_builder.cpp
    src/motion_detection.cpp
)

target_include_directories(ASRG PRIVATE include)
target_link_libraries(ASRG PRIVATE ${OpenCV_LIBS} Threads::Threads)