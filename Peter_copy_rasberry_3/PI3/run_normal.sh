#!/bin/bash

# Hacer que todos los procesos ejecutados se detengan cuando el script reciba Ctrl + C
trap "pkill -P $$; exit" SIGINT SIGTERM

# Ejecutar el segundo programa en segundo plano
./serial_udp_movement/build/serial_udp &

# Esperar a que los procesos terminen
wait
