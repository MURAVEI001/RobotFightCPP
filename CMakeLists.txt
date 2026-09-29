cmake_minimum_required(VERSION 3.16)
project(ASRG CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

add_executable(ASRG
    src/main.cpp
    src/video_source.cpp
    src/camera_source.cpp
    src/camera_thread.cpp
    src/grid_builder.cpp
    src/motion_detection.cpp
    src/aruco_detection.cpp
    src/visualize.cpp
)

target_include_directories(ASRG PRIVATE src)
target_link_libraries(ASRG PRIVATE ${OpenCV_LIBS})

find_package(Threads REQUIRED)
target_link_libraries(ASRG PRIVATE Threads::Threads)