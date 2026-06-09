cmake_minimum_required(VERSION 3.10)
project(tum_camera)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)

add_compile_options(-O2 -march=armv8-a)

set(OpenCV_DIR /home/neuro/opencv/build)

find_package(OpenCV REQUIRED)

add_executable(tum_camera tum_camera_view.cpp)
target_link_libraries(tum_camera ${OpenCV_LIBS})
