#include <opencv2/opencv.hpp>
#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <csignal>

// Definir constantes
constexpr int FRAME_WIDTH = 640;
constexpr int FRAME_HEIGHT = 480;
constexpr size_t PACKET_SIZE = 1400;  // Tamaño reducido del paquete UDP
constexpr char SERVER_IP[] = "192.168.185.254";  // IP del servidor
constexpr int SERVER_PORT = 8080;
constexpr int JPEG_QUALITY = 50;  // Calidad de compresión JPEG
constexpr int PUERTO_CAM = 2; // Puerto de la cámara

// Variables globales
int sockfd = -1;
cv::VideoCapture cap;

// Capturar CTRL+C para cerrar correctamente el socket
void signal_handler(int) {
    if (sockfd >= 0) {
        close(sockfd);
        std::cout << "\n🔴 Socket cerrado correctamente." << std::endl;
    }
    if (cap.isOpened()) {
        cap.release();
        std::cout << "📷 Cámara liberada." << std::endl;
    }
    exit(0);
}

/**
 * @brief Configura el socket UDP y devuelve su descriptor.
 */
int setup_udp_socket(const char* ip, int port, struct sockaddr_in& serverAddr) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("❌ Error al crear el socket UDP");
        return -1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serverAddr.sin_addr);

    int buffer_size = 512 * 1024;
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size)) < 0) {
        perror("⚠️ Error al configurar buffer del socket");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

/**
 * @brief Captura y envía un frame comprimido en paquetes UDP.
 */
bool send_frame(int sockfd, sockaddr_in& serverAddr, cv::VideoCapture& cap) {
    cv::Mat frame;
    cap >> frame;

    if (frame.empty()) {
        std::cerr << "⚠️ Error: No se pudo leer el frame. Intentando reiniciar la cámara..." << std::endl;
        cap.release();
        usleep(500000);
        cap.open(PUERTO_CAM);
        if (!cap.isOpened()) {
            std::cerr << "❌ No se pudo reabrir la cámara." << std::endl;
            return false;
        }
        return true;
    }

    // Comprimir la imagen
    std::vector<uchar> buffer;
    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, JPEG_QUALITY};
    if (!cv::imencode(".jpg", frame, buffer, params)) {
        std::cerr << "❌ Error al comprimir la imagen." << std::endl;
        return false;
    }

    size_t total_size = buffer.size();
    uint16_t total_packets = (total_size + PACKET_SIZE - 1) / PACKET_SIZE;
    int send_failures = 0;

    for (uint16_t i = 0; i < total_packets; i++) {
        size_t start = i * PACKET_SIZE;
        size_t chunk_size = std::min<size_t>(PACKET_SIZE, total_size - start);

        // Encabezado del paquete (2 bytes de ID, 2 bytes total de paquetes)
        std::vector<uchar> packet(4 + chunk_size);
        packet[0] = i & 0xFF;             // ID del paquete (byte bajo)
        packet[1] = (i >> 8) & 0xFF;      // ID del paquete (byte alto)
        packet[2] = total_packets & 0xFF; // Total de paquetes (byte bajo)
        packet[3] = (total_packets >> 8) & 0xFF; // Total de paquetes (byte alto)

        // Copiar datos al paquete
        std::memcpy(packet.data() + 4, buffer.data() + start, chunk_size);

        // Enviar paquete
        ssize_t sent = sendto(sockfd, packet.data(), packet.size(), 0,
                              (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        if (sent < 0) {
            perror("❌ Error al enviar datos");
            if (++send_failures > 5) {
                std::cerr << "🚨 Demasiados errores de envío. Reiniciando socket..." << std::endl;
                close(sockfd);
                sockfd = setup_udp_socket(SERVER_IP, SERVER_PORT, serverAddr);
                if (sockfd < 0) return false;
                send_failures = 0;
            }
        } else {
            send_failures = 0;
        }
    }
    return true;
}

int main() {
    signal(SIGINT, signal_handler);

    // Inicializar cámara con la nueva resolución 320x240
    cap.open(PUERTO_CAM, cv::CAP_V4L2);
    if (!cap.isOpened()) {
        std::cerr << "❌ Error: No se pudo abrir la cámara." << std::endl;
        return -1;
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

    struct sockaddr_in serverAddr;
    sockfd = setup_udp_socket(SERVER_IP, SERVER_PORT, serverAddr);
    if (sockfd < 0) return -1;

    while (true) {
        if (!send_frame(sockfd, serverAddr, cap)) break;
    }

    signal_handler(0);
    return 0;
}
