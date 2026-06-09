import board
import busio
import adafruit_ads1x15.ads1115 as ADS
from adafruit_ads1x15.analog_in import AnalogIn
from adafruit_ads1x15.ads1115 import ADS1115
import time

i2c = busio.I2C(board.SCL, board.SDA)
ads = ADS1115(i2c)
ads.gain = 1  # Rango ±4.096V

chan = AnalogIn(ads, 0)

# Calibración automática al inicio
print("Calibrando... no conectes carga")
time.sleep(2)
lecturas = [chan.voltage for _ in range(20)]
voltaje_cero = sum(lecturas) / len(lecturas)
print(f"Voltaje cero calibrado: {voltaje_cero:.3f} V")
print("Listo!\n")

while True:
    lecturas = [chan.voltage for _ in range(10)]
    voltaje = sum(lecturas) / len(lecturas)
    corriente = abs((voltaje - voltaje_cero) / 0.1)  # abs() para evitar negativos
    print(f"Voltaje: {voltaje:.3f} V | Corriente: {corriente:.3f} A")
    time.sleep(0.5)
