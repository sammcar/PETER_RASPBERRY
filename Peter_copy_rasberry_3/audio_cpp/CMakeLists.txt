cmake_minimum_required(VERSION 3.10.0)
project(audio_cpp VERSION 0.1.0 LANGUAGES C CXX)

# Buscar la librería ALSA
find_package(PkgConfig REQUIRED)

add_executable(audio_cpp main.cpp)

# Enlazar la librería ALSA
target_include_directories(audio_cpp PRIVATE ${VLC_INCLUDE_DIRS})
target_link_libraries(audio_cpp PRIVATE ${VLC_LIBRARIES})