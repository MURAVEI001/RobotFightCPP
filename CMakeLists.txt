cmake_minimum_required(VERSION 3.10)
project(RobotFightCPP CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")

find_package(Threads REQUIRED)
find_package(OpenCV  REQUIRED)

add_executable(RobotFightCPP
    src/main.cpp
    src/FrameReceiver.cpp
    src/FrameProcessor.cpp
)

target_include_directories(RobotFightCPP PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(RobotFightCPP PRIVATE Threads::Threads ${OpenCV_LIBS})