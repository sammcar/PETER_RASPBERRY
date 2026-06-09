#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <array>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> // Para close()

constexpr int SIZE_DATOS = 200;
constexpr char UDP_IP[] = "192.168.161.141"; // IP de destino
constexpr int UDP_PORT = 8888;               // Puerto de destino

struct LidarData {
    uint64_t timestamp;
    double angle;
    int distance;
    int intensity;
};

int main() {
    // Crear socket UDP
    int udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket < 0) {
        perror("❌ Error: No se pudo crear el socket UDP");
        return 1;
    }

    // Configurar dirección del servidor UDP
    sockaddr_in serverAddr{};
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(UDP_PORT);
    if (inet_pton(AF_INET, UDP_IP, &serverAddr.sin_addr) <= 0) {
        std::cerr << "❌ Error: Dirección IP no válida.\n";
        close(udpSocket);
        return 1;
    }

    const char* command = "/home/peter/Desktop/ldlidar_stl_sdk-master/build/ldlidar_stl_node LD19 serialcom /dev/ttyUSB0";
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        std::cerr << "❌ Error: No se pudo ejecutar el comando.\n";
        close(udpSocket); // Cerrar socket en caso de error
        return 1;
    }

    char buffer[512];
    std::array<LidarData, SIZE_DATOS> dataBuffer;
    int count = 0;
    const size_t dataSize = sizeof(LidarData) * SIZE_DATOS;
    socklen_t addrLen = sizeof(serverAddr);

    while (std::fgets(buffer, sizeof(buffer), pipe)) {
        if (char* start = std::strstr(buffer, "stamp:")) {
            LidarData tempData;

            if (sscanf(start, "stamp:%lu,angle:%lf,distance(mm):%d,intensity:%d", 
                       &tempData.timestamp, &tempData.angle, 
                       &tempData.distance, &tempData.intensity) == 4 && tempData.distance > 0) {
                
                dataBuffer[count++] = tempData; // Guardar en buffer

                if (count == SIZE_DATOS) {
                    // 🔹 Enviar datos por UDP
                    if (sendto(udpSocket, dataBuffer.data(), dataSize, 0,
                               (struct sockaddr*)&serverAddr, addrLen) < 0) {
                        perror("❌ Error al enviar datos por UDP");
                    } else {
                        std::cout << "✅ Enviados " << SIZE_DATOS << " datos por UDP.\n";
                    }

                    count = 0; // Reiniciar buffer
                }
            }
        }
    }

    pclose(pipe);
    close(udpSocket);
    return 0;
}
