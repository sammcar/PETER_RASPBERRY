#include <cstdio>
  #include <cstdlib>
  #include <cstdint>
  #include <cstring>
  #include <iostream>
  #include <array>
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <unistd.h>

  constexpr int SIZE_DATOS = 500;
  constexpr int UDP_PORT = 8888;

  struct LidarData {
      uint64_t timestamp;
      double angle;
      int distance;
      int intensity;
  };

  int main(int argc, char* argv[]) {
      const char* UDP_IP = (argc >= 2) ? argv[1] : "11.11.41.5";

      int udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
      if (udpSocket < 0) {
          perror("Error: No se pudo crear el socket UDP");
          return 1;
      }

      sockaddr_in serverAddr{};
      std::memset(&serverAddr, 0, sizeof(serverAddr));
      serverAddr.sin_family = AF_INET;
      serverAddr.sin_port = htons(UDP_PORT);
      if (inet_pton(AF_INET, UDP_IP, &serverAddr.sin_addr) <= 0) {
          std::cerr << "Error: IP no valida." << std::endl;
          close(udpSocket);
          return 1;
      }
      std::cout << "Enviando a: " << UDP_IP << ":" << UDP_PORT << std::endl;

      const char* command =

  "/home/neuro/Desktop/ldlidar_stl_sdk-master/build/ldlidar_stl_node"
          " LD19 serialcom /dev/lidar";

      FILE* pipe = popen(command, "r");
      if (!pipe) {
          std::cerr << "Error: No se pudo ejecutar el comando." <<
  std::endl;
          close(udpSocket);
          return 1;
      }
      std::cout << "Lidar iniciado." << std::endl;

      char buffer[512];
      std::array<LidarData, SIZE_DATOS> dataBuffer;
      int count = 0;
      const size_t dataSize = sizeof(LidarData) * SIZE_DATOS;
      socklen_t addrLen = sizeof(serverAddr);

      while (std::fgets(buffer, sizeof(buffer), pipe)) {
          char* start = std::strstr(buffer, "stamp:");
          if (!start) continue;

          LidarData tempData;
          int parsed = sscanf(
              start,
              "stamp:%lu,angle:%lf,distance(mm):%d,intensity:%d",
              &tempData.timestamp,
              &tempData.angle,
              &tempData.distance,
              &tempData.intensity
          );

          if (parsed == 4 && tempData.distance > 0) {
              dataBuffer[count++] = tempData;
              if (count == SIZE_DATOS) {
                  if (sendto(udpSocket, dataBuffer.data(), dataSize, 0,
                             (struct sockaddr*)&serverAddr, addrLen) < 0) {
                      perror("Error al enviar UDP");
                  }
                  count = 0;
              }
          }
      }

      pclose(pipe);
      close(udpSocket);
      return 0;
  }
