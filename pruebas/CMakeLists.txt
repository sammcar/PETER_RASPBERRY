cmake_minimum_required(VERSION 3.10)
  project(pruebas)

  add_executable(imu_ina imu_ina.cpp)
  target_link_libraries(imu_ina m)
