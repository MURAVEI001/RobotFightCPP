cmake_minimum_required(VERSION 3.16)
project(raspiGrid LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs highgui)

add_executable(raspiGrid
    src/main.cpp
    src/raspiGrid.cpp
)

target_include_directories(raspiGrid PRIVATE src ${OpenCV_INCLUDE_DIRS})
target_link_libraries(raspiGrid PRIVATE ${OpenCV_LIBS} pthread)

target_compile_options(raspiGrid PRIVATE -Wall -Wextra -Wpedantic)