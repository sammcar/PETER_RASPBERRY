#ifndef LIDAR_UDP_STREAMER_HPP
#define LIDAR_UDP_STREAMER_HPP

#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include <cstdint>
#include <array>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

constexpr int SIZE_DATOS = 200;

struct LidarData {
    uint64_t timestamp;
    double angle;
    int distance;
    int intensity;
};

class LidarUDPStreamer {
public:
    LidarUDPStreamer(const std::string& command, const std::string& ip, int port);
    ~LidarUDPStreamer();

    LidarUDPStreamer(const LidarUDPStreamer&) = delete;            // Prohibir copias
    LidarUDPStreamer& operator=(const LidarUDPStreamer&) = delete; // Prohibir asignación

    void start();
    void stop();

private:
    void lidar_loop(); // Indica que no lanza excepciones

    std::string command_;
    std::string udp_ip_;
    int udp_port_;
    int udp_socket_ = -1; // Inicializar directamente
    std::atomic<bool> running_{false};
    std::thread lidar_thread_;
    struct sockaddr_in serverAddr_; // Ahora se configura en el constructor
};

#endif // LIDAR_UDP_STREAMER_HPP
