cmake_minimum_required(VERSION 3.10)
project(RobotFightCPP)

# Находим установленную OpenCV
find_package(OpenCV REQUIRED)

# Создаем исполняемый файл (замените main.cpp на ваши файлы)
add_executable(main main.cpp)

# Линкуем библиотеки OpenCV
target_link_libraries(main ${OpenCV_LIBS})