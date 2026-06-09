#include <opencv2/opencv.hpp>
#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <chrono>
#include <thread>

constexpr int FRAME_WIDTH = 640;
constexpr int FRAME_HEIGHT = 480;
constexpr int JPEG_QUALITY = 30;
constexpr size_t PACKET_SIZE = 1400;  // Máximo tamaño de paquete UDP
constexpr char SERVER_IP[] = "192.168.159.254";  // IP del servidor
constexpr int SERVER_PORT = 8080;
constexpr int PUERTO_CAM = 0;  // Índice de la cámara
constexpr int FRAME_INTERVAL_MS = 60;  // Intervalo entre frames en ms

// Inicializar socket UDP
int setup_udp_socket(struct sockaddr_in& serverAddr) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("❌ Error al crear socket UDP");
        return -1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    // Aumentar buffers de socket para mejorar estabilidad
    int buffer_size = 512 * 1024;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof(buffer_size));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size));

    return sockfd;
}

// Captura y envía un frame comprimido en paquetes UDP
bool send_frame(int sockfd, sockaddr_in& serverAddr, cv::VideoCapture& cap, int frame_count) {
    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) {
        std::cerr << "⚠️ Error: No se pudo capturar el frame." << std::endl;
        return false;
    }

    // Comprimir la imagen en JPEG
    static std::vector<uchar> buffer;
    buffer.clear();
    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, JPEG_QUALITY};
    if (!cv::imencode(".jpg", frame, buffer, params)) {
        std::cerr << "❌ Error al comprimir la imagen." << std::endl;
        return false;
    }

    size_t total_size = buffer.size();
    uint16_t total_packets = (total_size + PACKET_SIZE - 1) / PACKET_SIZE;

    static std::vector<uchar> packet(4 + PACKET_SIZE);  // Reutilizar el vector
    for (uint16_t i = 0; i < total_packets; i++) {
        size_t start = i * PACKET_SIZE;
        size_t chunk_size = std::min(PACKET_SIZE, total_size - start);

        // Encabezado del paquete (ID y total de paquetes)
        packet[0] = i & 0xFF;               // ID del paquete (byte bajo)
        packet[1] = (i >> 8) & 0xFF;        // ID del paquete (byte alto)
        packet[2] = total_packets & 0xFF;   // Total de paquetes (byte bajo)
        packet[3] = (total_packets >> 8) & 0xFF;  // Total de paquetes (byte alto)

        // Copiar datos al paquete con memcpy (más rápido que std::copy)
        std::memcpy(packet.data() + 4, buffer.data() + start, chunk_size);

        // Enviar paquete UDP
        ssize_t sent = sendto(sockfd, packet.data(), 4 + chunk_size, 0,
                              (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        if (sent < 0) {
            perror("❌ Error al enviar paquete");
            return false;
        }
    }

    // Mensaje de estado
    std::cout << "📡 Frame #" << frame_count 
              << " | Tamaño: " << total_size << " bytes"
              << " | Paquetes: " << total_packets << std::endl;
    return true;
}

int main() {
    cv::VideoCapture cap(PUERTO_CAM, cv::CAP_V4L2);
    if (!cap.isOpened()) {
        std::cerr << "❌ Error: No se pudo abrir la cámara." << std::endl;
        return -1;
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

    struct sockaddr_in serverAddr;
    int sockfd = setup_udp_socket(serverAddr);
    if (sockfd < 0) return -1;

    int frame_count = 0;

    while (true) {
        auto start_time = std::chrono::steady_clock::now();  // Inicio del ciclo
        frame_count++;

        if (!send_frame(sockfd, serverAddr, cap, frame_count)) break;

        // Calcular el tiempo restante para completar FRAME_INTERVAL_MS
        auto elapsed_time = std::chrono::steady_clock::now() - start_time;
        auto sleep_time = std::chrono::milliseconds(FRAME_INTERVAL_MS) - elapsed_time;

        if (sleep_time > std::chrono::milliseconds(0)) {
            std::this_thread::sleep_for(sleep_time);
        }
    }

    close(sockfd);
    return 0;
}
