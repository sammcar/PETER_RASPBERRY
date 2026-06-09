#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <array>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> // Para close()

constexpr int SIZE_DATOS = 500;
constexpr char UDP_IP[] = "192.168.185.254"; // IP de destino
constexpr int UDP_PORT = 8888;               // Puerto de destino

struct LidarData {
    uint64_t timestamp;
    double angle;
    int distance;
    int intensity;
};

int main() {
    std::cout << "🟢 Iniciando programa..." << std::endl;
    
    // Crear socket UDP
    int udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket < 0) {
        perror("❌ Error: No se pudo crear el socket UDP");
        return 1;
    }
    std::cout << "✅ Socket UDP creado exitosamente." << std::endl;

    // Configurar dirección del servidor UDP
    sockaddr_in serverAddr{};
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(UDP_PORT);
    if (inet_pton(AF_INET, UDP_IP, &serverAddr.sin_addr) <= 0) {
        std::cerr << "❌ Error: Dirección IP no válida." << std::endl;
        close(udpSocket);
        return 1;
    }
    std::cout << "✅ Dirección IP configurada correctamente: " << UDP_IP << ":" << UDP_PORT << std::endl;

    const char* command = "/home/peter/Desktop/ldlidar_stl_sdk-master/build/ldlidar_stl_node LD19 serialcom /dev/ttyUSB0";
    std::cout << "🟢 Ejecutando comando: " << command << std::endl;
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        std::cerr << "❌ Error: No se pudo ejecutar el comando." << std::endl;
        close(udpSocket); // Cerrar socket en caso de error
        return 1;
    }
    std::cout << "✅ Comando ejecutado correctamente." << std::endl;

    char buffer[512];
    std::array<LidarData, SIZE_DATOS> dataBuffer;
    int count = 0;
    const size_t dataSize = sizeof(LidarData) * SIZE_DATOS;
    socklen_t addrLen = sizeof(serverAddr);

    while (std::fgets(buffer, sizeof(buffer), pipe)) {
        std::cout << "🔹 Recibido: " << buffer;
        if (char* start = std::strstr(buffer, "stamp:")) {
            LidarData tempData;

            if (sscanf(start, "stamp:%lu,angle:%lf,distance(mm):%d,intensity:%d", 
                       &tempData.timestamp, &tempData.angle, 
                       &tempData.distance, &tempData.intensity) == 4 && tempData.distance > 0) {
                
                dataBuffer[count++] = tempData; // Guardar en buffer
                std::cout << "🟢 Dato registrado - Timestamp: " << tempData.timestamp 
                          << " Angle: " << tempData.angle 
                          << " Distance: " << tempData.distance 
                          << " Intensity: " << tempData.intensity << std::endl;

                if (count == SIZE_DATOS) {
                    // 🔹 Enviar datos por UDP
                    std::cout << "📤 Enviando " << SIZE_DATOS << " datos por UDP..." << std::endl;
                    if (sendto(udpSocket, dataBuffer.data(), dataSize, 0,
                               (struct sockaddr*)&serverAddr, addrLen) < 0) {
                        perror("❌ Error al enviar datos por UDP");
                    } else {
                        std::cout << "✅ Datos enviados exitosamente por UDP." << std::endl;
                    }
                    count = 0; // Reiniciar buffer
                }
            } else {
                std::cerr << "⚠️ Advertencia: Datos no válidos o distancia = 0." << std::endl;
            }
        }
    }

    std::cout << "🟢 Finalizando programa..." << std::endl;
    pclose(pipe);
    close(udpSocket);
    return 0;
}

