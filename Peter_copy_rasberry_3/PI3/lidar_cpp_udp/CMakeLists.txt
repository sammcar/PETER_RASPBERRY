cmake_minimum_required(VERSION 3.10.0)
project(lidar_c VERSION 0.1.0 LANGUAGES C CXX)

# # Agregar todos los archivos fuente
# set(SOURCES 
#     include/serial_interface_linux/serial_interface_linux.cpp
# )


# # Agregar el directorio de encabezados
# include_directories(${CMAKE_SOURCE_DIR}/include/serial_interface_linux)

add_executable(lidar_c main.cpp)

