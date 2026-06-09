cmake_minimum_required(VERSION 3.10)
project(camera_cpp VERSION 0.1.0 LANGUAGES C CXX)

# Busca la instalación de OpenCV
find_package(OpenCV REQUIRED)

# Incluye los directorios de encabezados de OpenCV
include_directories(${OpenCV_INCLUDE_DIRS})

# Agrega el ejecutable
add_executable(camera_cpp main.cpp)

# Vincula las bibliotecas de OpenCV con el ejecutable
target_link_libraries(camera_cpp ${OpenCV_LIBS})

# Imprime las variables para asegurarse de que CMake está encontrando OpenCV correctamente
message(STATUS "OpenCV_INCLUDE_DIRS = ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV_LIBS = ${OpenCV_LIBS}")
