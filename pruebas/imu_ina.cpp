#include <iostream>
  #include <cstdint>
  #include <cstring>
  #include <cmath>
  #include <chrono>
  #include <thread>
  #include <csignal>
  #include <fcntl.h>
  #include <unistd.h>
  #include <sys/ioctl.h>
  #include <linux/i2c-dev.h>
  #include <sys/socket.h>
  #include <arpa/inet.h>

  #define BMI160_ADDR  0x69
  #define INA226_ADDR  0x40
  #define I2C_BUS      "/dev/i2c-1"

  #define BMI160_REG_CHIP_ID   0x00
  #define BMI160_REG_CMD       0x7E
  #define BMI160_REG_GYR_DATA  0x0C
  #define BMI160_REG_ACC_DATA  0x12
  #define BMI160_CMD_ACC_ON    0x11
  #define BMI160_CMD_GYR_ON    0x15
  #define ACC_SCALE   (9.81f / 16384.0f)
  #define GYRO_SCALE  (1.0f  / 16.4f)

  #define INA226_REG_CONFIG   0x00
  #define INA226_REG_CURRENT  0x04
  #define INA226_REG_BUS      0x02
  #define INA226_REG_CALIB    0x05
  #define SHUNT_OHMS   0.1f
  #define MAX_AMPS     3.0f
  #define CURRENT_LSB  (MAX_AMPS / 32768.0f)
  #define CALIB_VAL    ((uint16_t)(0.00512f / (CURRENT_LSB * SHUNT_OHMS)))

  #define SERVER_PORT  9999
  #define LOOP_HZ      100

  struct SensorPacket {
      uint64_t timestamp_ns;
      float roll;
      float pitch;
      float acc_x, acc_y, acc_z;
      float gyr_x, gyr_y, gyr_z;
      float current_A;
      float voltage_V;
  };

  int i2c_fd  = -1;
  int udp_fd  = -1;
  bool running = true;

  void signal_handler(int) { running = false; }

  bool i2c_write(uint8_t addr, uint8_t reg, uint8_t val) {
      if (ioctl(i2c_fd, I2C_SLAVE, addr) < 0) return false;
      uint8_t buf[2] = {reg, val};
      return write(i2c_fd, buf, 2) == 2;
  }

  bool i2c_read(uint8_t addr, uint8_t reg, uint8_t* data, int len) {
      if (ioctl(i2c_fd, I2C_SLAVE, addr) < 0) return false;
      if (write(i2c_fd, &reg, 1) != 1) return false;
      return read(i2c_fd, data, len) == len;
  }

  bool i2c_write16(uint8_t addr, uint8_t reg, uint16_t val) {
      if (ioctl(i2c_fd, I2C_SLAVE, addr) < 0) return false;
      uint8_t buf[3] = {reg, (uint8_t)(val >> 8), (uint8_t)(val & 0xFF)};
      return write(i2c_fd, buf, 3) == 3;
  }

  int16_t i2c_read16_signed(uint8_t addr, uint8_t reg) {
      uint8_t buf[2];
      if (!i2c_read(addr, reg, buf, 2)) return 0;
      return (int16_t)((buf[0] << 8) | buf[1]);
  }

  bool bmi160_init() {
      uint8_t chip_id;
      if (!i2c_read(BMI160_ADDR, BMI160_REG_CHIP_ID, &chip_id, 1)) {
          std::cerr << "BMI160: error de lectura" << std::endl;
          return false;
      }
      if (chip_id != 0xD1) {
          std::cerr << "BMI160: chip_id inesperado: 0x"
                    << std::hex << (int)chip_id << std::endl;
          return false;
      }
      i2c_write(BMI160_ADDR, BMI160_REG_CMD, BMI160_CMD_ACC_ON);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      i2c_write(BMI160_ADDR, BMI160_REG_CMD, BMI160_CMD_GYR_ON);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      std::cout << "BMI160 OK" << std::endl;
      return true;
  }

  bool bmi160_read(float& ax, float& ay, float& az,
                   float& gx, float& gy, float& gz,
                   float& roll, float& pitch) {
      uint8_t raw[6];
      if (!i2c_read(BMI160_ADDR, BMI160_REG_GYR_DATA, raw, 6)) return false;
      gx = (int16_t)(raw[1] << 8 | raw[0]) * GYRO_SCALE;
      gy = (int16_t)(raw[3] << 8 | raw[2]) * GYRO_SCALE;
      gz = (int16_t)(raw[5] << 8 | raw[4]) * GYRO_SCALE;
      if (!i2c_read(BMI160_ADDR, BMI160_REG_ACC_DATA, raw, 6)) return false;
      ax = (int16_t)(raw[1] << 8 | raw[0]) * ACC_SCALE;
      ay = (int16_t)(raw[3] << 8 | raw[2]) * ACC_SCALE;
      az = (int16_t)(raw[5] << 8 | raw[4]) * ACC_SCALE;
      roll  = atan2f(ay, az) * 180.0f / M_PI;
      pitch = atan2f(-ax, sqrtf(ay*ay + az*az)) * 180.0f / M_PI;
      return true;
  }

  bool ina226_init() {
      if (!i2c_write16(INA226_ADDR, INA226_REG_CONFIG, 0x4527)) return
  false;
      if (!i2c_write16(INA226_ADDR, INA226_REG_CALIB,  CALIB_VAL)) return
  false;
      std::cout << "INA226 OK (calib=" << CALIB_VAL << ")" << std::endl;
      return true;
  }

  bool ina226_read(float& current_A, float& voltage_V) {
      int16_t raw_current = i2c_read16_signed(INA226_ADDR,
  INA226_REG_CURRENT);
      int16_t raw_voltage = i2c_read16_signed(INA226_ADDR, INA226_REG_BUS);
      current_A = raw_current * CURRENT_LSB;
      voltage_V = raw_voltage * 0.00125f;
      return true;
  }

  int udp_setup(const char* ip, sockaddr_in& addr) {
      int fd = socket(AF_INET, SOCK_DGRAM, 0);
      if (fd < 0) { perror("socket UDP"); return -1; }
      memset(&addr, 0, sizeof(addr));
      addr.sin_family = AF_INET;
      addr.sin_port   = htons(SERVER_PORT);
      inet_pton(AF_INET, ip, &addr.sin_addr);
      std::cout << "UDP -> " << ip << ":" << SERVER_PORT << std::endl;
      return fd;
  }

  int main(int argc, char* argv[]) {
      const char* SERVER_IP = (argc >= 2) ? argv[1] : "11.11.41.5";

      signal(SIGINT, signal_handler);

      i2c_fd = open(I2C_BUS, O_RDWR);
      if (i2c_fd < 0) { perror("Error abriendo I2C"); return -1; }

      if (!bmi160_init() || !ina226_init()) return -1;

      sockaddr_in server_addr;
      udp_fd = udp_setup(SERVER_IP, server_addr);
      if (udp_fd < 0) return -1;

      const auto period = std::chrono::microseconds(1000000 / LOOP_HZ);
      SensorPacket pkt;
      uint32_t count = 0;

      std::cout << "Enviando a " << LOOP_HZ << "Hz ("
                << sizeof(SensorPacket) << " bytes/paquete)" << std::endl;

      while (running) {
          auto t0 = std::chrono::steady_clock::now();

          pkt.timestamp_ns = (uint64_t)std::chrono::duration_cast<
              std::chrono::nanoseconds>(
              std::chrono::system_clock::now().time_since_epoch()).count();

          float ax, ay, az, gx, gy, gz, roll, pitch;
          bmi160_read(ax, ay, az, gx, gy, gz, roll, pitch);
          pkt.acc_x = ax; pkt.acc_y = ay; pkt.acc_z = az;
          pkt.gyr_x = gx; pkt.gyr_y = gy; pkt.gyr_z = gz;
          pkt.roll  = roll;
          pkt.pitch = pitch;

          float current, voltage;
          ina226_read(current, voltage);
          pkt.current_A = current;
          pkt.voltage_V = voltage;

          sendto(udp_fd, &pkt, sizeof(pkt), 0,
                 (sockaddr*)&server_addr, sizeof(server_addr));

          if (++count % 100 == 0)
              std::cout << count << " pkts | "
                        << "roll=" << pkt.roll << " "
                        << "pitch=" << pkt.pitch << " "
                        << "I=" << pkt.current_A << "A "
                        << "V=" << pkt.voltage_V << "V" << std::endl;

          auto elapsed = std::chrono::steady_clock::now() - t0;
          if (elapsed < period)
              std::this_thread::sleep_for(period - elapsed);
      }

      close(i2c_fd);
      close(udp_fd);
      return 0;
  }
