#!/bin/bash

# Hacer que todos los procesos ejecutados se detengan cuando el script reciba Ctrl + C
trap "pkill -P $$; exit" SIGINT SIGTERM

# Ejecutar el primer programa en segundo plano
./camera/build/camera_cpp &

# Esperar 2 segundos
sleep 2

# Ejecutar el segundo programa en segundo plano
./serial_comuni/build/serial &

# Esperar 1 segundo
sleep 1

# Ejecutar el tercer programa en segundo plano
./Audio/build/audio &

# Esperar a que los procesos terminen
wait
