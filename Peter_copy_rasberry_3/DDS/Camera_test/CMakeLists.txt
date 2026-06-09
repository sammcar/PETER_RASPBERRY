cmake_minimum_required(VERSION 3.16)
project(Camera_example LANGUAGES C CXX)  # Cambia a C++ para usar OpenCV

find_package(CycloneDDS REQUIRED)
find_package(OpenCV REQUIRED)  # Encuentra OpenCV

idlc_generate(TARGET CameraData_lib FILES "CameraData.idl" WARNINGS no-implicit-extensibility)

add_executable(Publicador src/publisher.cpp)  # Cambia a .cpp
add_executable(Suscriptor src/subscriber.cpp)  # Cambia a .cpp

target_link_libraries(Publicador CameraData_lib CycloneDDS::ddsc ${OpenCV_LIBS})
target_link_libraries(Suscriptor CameraData_lib CycloneDDS::ddsc ${OpenCV_LIBS})

include_directories(${OpenCV_INCLUDE_DIRS})  # Incluye los headers de OpenCV
