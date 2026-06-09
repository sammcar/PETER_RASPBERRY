#!/bin/bash

# Hacer que todos los procesos ejecutados se detengan cuando el script reciba Ctrl + C
trap "pkill -P $$; exit" SIGINT SIGTERM

# Ejecutar el primer programa en segundo plano
./camera/build/camera_cpp &

# Esperar 2 segundos
sleep 2

# Ejecutar el segundo programa en segundo plano
./serial_udp_movement/build/serial_udp &

# Esperar 1 segundo
sleep 1

# Ejecutar el tercer programa en segundo plano
./lidar_cpp_udp/build/lidar_c &

# Esperar a que los procesos terminen
wait
