#ifndef UDP_VIDEO_STREAMER_HPP
#define UDP_VIDEO_STREAMER_HPP

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <string>       // 🔹 Añadir string para evitar posibles errores de compilación
#include <arpa/inet.h>
#include <unistd.h>

class UDPVideoStreamer {
public:
    UDPVideoStreamer(const std::string& server_ip, int server_port, int camera_index);
    ~UDPVideoStreamer();

    // 🔹 Deshabilitar la copia para evitar problemas con std::thread y std::atomic
    UDPVideoStreamer(const UDPVideoStreamer&) = delete;
    UDPVideoStreamer& operator=(const UDPVideoStreamer&) = delete;

    void start_streaming();
    void stop_streaming();

private:
    void stream_loop();
    int setup_udp_socket(const std::string& ip, int port, struct sockaddr_in& serverAddr);
    bool send_frame();

    std::string server_ip_;
    int server_port_;
    int camera_index_;

    int sockfd_;
    struct sockaddr_in serverAddr_;
    cv::VideoCapture cap_;

    std::vector<uchar> buffer_;  // 🔹 Evita la reasignación en cada frame

    std::thread streaming_thread_;
    std::atomic<bool> running_;
};

#endif // UDP_VIDEO_STREAMER_HPP
