#include "lidar_udp.hpp"
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <array>

LidarUDPStreamer::LidarUDPStreamer(const std::string& command, const std::string& ip, int port)
    : command_(command), udp_ip_(ip), udp_port_(port), running_(false), udp_socket_(-1) {
    // Configurar dirección del servidor UDP una sola vez
    std::memset(&serverAddr_, 0, sizeof(serverAddr_));
    serverAddr_.sin_family = AF_INET;
    serverAddr_.sin_port = htons(udp_port_);
    
    if (inet_pton(AF_INET, udp_ip_.c_str(), &serverAddr_.sin_addr) <= 0) {
        std::cerr << "❌ Error: Dirección IP no válida.\n";
    }
}

LidarUDPStreamer::~LidarUDPStreamer() {
    stop();
}

void LidarUDPStreamer::start() {
    if (running_) return;  // Evitar múltiples llamadas a start()
    running_ = true;
    lidar_thread_ = std::thread(&LidarUDPStreamer::lidar_loop, this);
}

void LidarUDPStreamer::stop() {
    running_ = false;
    if (lidar_thread_.joinable()) {
        lidar_thread_.join();
    }
}

void LidarUDPStreamer::lidar_loop() {
    // Crear socket UDP
    udp_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket_ < 0) {
        perror("❌ Error: No se pudo crear el socket UDP");
        return;
    }

    FILE* pipe = popen(command_.c_str(), "r");
    if (!pipe) {
        std::cerr << "❌ Error: No se pudo ejecutar el comando.\n";
        close(udp_socket_);
        return;
    }

    char buffer[512];
    std::array<LidarData, SIZE_DATOS> dataBuffer;
    int count = 0;
    const size_t dataSize = sizeof(LidarData) * SIZE_DATOS;
    socklen_t addrLen = sizeof(serverAddr_);

    while (running_ && std::fgets(buffer, sizeof(buffer), pipe)) {
        if (char* start = std::strstr(buffer, "stamp:")) {
            LidarData tempData;

            if (sscanf(start, "stamp:%lu,angle:%lf,distance(mm):%d,intensity:%d", 
                       &tempData.timestamp, &tempData.angle, 
                       &tempData.distance, &tempData.intensity) == 4 && tempData.distance > 0) {
                
                dataBuffer[count++] = tempData; // Guardar en buffer

                if (count == SIZE_DATOS) {
                    if (sendto(udp_socket_, dataBuffer.data(), dataSize, 0,
                               (struct sockaddr*)&serverAddr_, addrLen) < 0) {
                        perror("❌ Error al enviar datos por UDP");
                    }
                    count = 0; // Reiniciar buffer
                }
            }
        }
    }

    pclose(pipe);
    close(udp_socket_);
}
