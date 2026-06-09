import time  # <- agregar este import
import board
import busio
from adafruit_ina219 import INA219

i2c = busio.I2C(board.SCL, board.SDA)
ina = INA219(i2c)

while True:
    print(f"Voltaje bus:    {ina.bus_voltage:.2f} V")
    print(f"Voltaje shunt:  {ina.shunt_voltage:.4f} V")
    print(f"Corriente:      {ina.current:.2f} mA")
    print(f"Potencia:       {ina.power:.2f} mW")
    print("-" * 30)
    time.sleep(1)  # <- delay en segundos
