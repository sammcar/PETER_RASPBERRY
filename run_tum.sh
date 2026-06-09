#!/bin/bash

  LIDAR_BIN="/home/neuro/Desktop/Peter_copy_rasberry_3/PI3/lidar_cpp_udp/bui
  ld/lidar_c"
  CAM_BIN="/home/neuro/Desktop/Peter_copy_rasberry_3/PI3/camera/build/camera
  _cpp"
  IMU_BIN="/home/neuro/Desktop/pruebas/build/imu_ina"
  SERVER_IP="${1:-11.11.41.5}"
  CAM_PORT="${2:-0}"

  cleanup() {
      echo "Deteniendo procesos..."
      kill $PID_LIDAR $PID_CAM $PID_IMU 2>/dev/null
      wait $PID_LIDAR $PID_CAM $PID_IMU 2>/dev/null
      echo "Listo."
      exit 0
  }

  trap cleanup SIGINT SIGTERM

  echo "Iniciando lidar → $SERVER_IP..."
  $LIDAR_BIN $SERVER_IP &
  PID_LIDAR=$!

  echo "Iniciando camara en $CAM_PORT → $SERVER_IP..."
  $CAM_BIN $CAM_PORT $SERVER_IP &
  PID_CAM=$!

  echo "Iniciando IMU/INA → $SERVER_IP..."
  $IMU_BIN $SERVER_IP &
  PID_IMU=$!

  echo "Lidar PID: $PID_LIDAR"
  echo "Camara PID: $PID_CAM"
  echo "IMU PID: $PID_IMU"
  echo "Presiona Ctrl+C para detener."

  wait $PID_LIDAR $PID_CAM $PID_IMU
