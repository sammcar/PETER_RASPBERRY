from BMI160_i2c import Driver
import time

sensor = Driver(0x69)

# Factores de conversión (rango por defecto)
ACC_FACTOR  = 16384.0  # LSB/g
GYRO_FACTOR = 262.4    # LSB/°/s

print("Leyendo BMI160... (Ctrl+C para detener)\n")

try:
    while True:
        data = sensor.getMotion6()

        gyro_x = data[0] / GYRO_FACTOR
        gyro_y = data[1] / GYRO_FACTOR
        gyro_z = data[2] / GYRO_FACTOR

        acc_x = data[3] / ACC_FACTOR
        acc_y = data[4] / ACC_FACTOR
        acc_z = data[5] / ACC_FACTOR

        print("=== BMI160 ===")
        print(f"Acelerómetro (g)    X: {acc_x:>7.3f}  Y: {acc_y:>7.3f}  Z: {acc_z:>7.3f}")
        print(f"Giroscopio  (°/s)   X: {gyro_x:>7.3f}  Y: {gyro_y:>7.3f}  Z: {gyro_z:>7.3f}")
        print()

        time.sleep(0.5)

except KeyboardInterrupt:
    print("Detenido.")
