#include <opencv2/opencv.hpp>
#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <csignal>
#include <chrono>
#include <thread>
#include <sys/socket.h>

constexpr int FRAME_WIDTH = 640;  // Reducir resolución para menos carga
constexpr int FRAME_HEIGHT = 480;
constexpr int FPS = 22;  // Limitar FPS para reducir carga
constexpr char SERVER_IP[] = "192.168.185.254";
constexpr int SERVER_PORT = 8080;
constexpr int JPEG_QUALITY = 40;  // Baja calidad para menor uso de ancho de banda
constexpr int PUERTO_CAM = 2;
constexpr size_t CHUNK_SIZE = 4096;

int sockfd = -1;
cv::VideoCapture cap;

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

int setup_tcp_socket(const char* ip, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("❌ Error al crear el socket TCP");
        return -1;
    }

    int flag = 1;
    setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));

    struct sockaddr_in serverAddr{};
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serverAddr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("❌ Error al conectar con el servidor");
        close(sockfd);
        return -1;
    }

    std::cout << "✅ Conectado al servidor en " << SERVER_IP << ":" << SERVER_PORT << std::endl;
    return sockfd;
}

bool send_frame(int sockfd, cv::VideoCapture& cap) {
    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) {
        std::cerr << "⚠️ Error: No se pudo capturar el frame." << std::endl;
        return false;
    }

    // Reducir tamaño de imagen antes de comprimir
    cv::resize(frame, frame, cv::Size(FRAME_WIDTH, FRAME_HEIGHT));

    std::vector<uchar> buffer;
    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, JPEG_QUALITY};
    if (!cv::imencode(".jpg", frame, buffer, params)) {
        std::cerr << "❌ Error al comprimir la imagen." << std::endl;
        return false;
    }

    uint32_t total_size = htonl(buffer.size());
    if (send(sockfd, &total_size, sizeof(total_size), 0) < 0) {
        perror("❌ Error al enviar el tamaño de la imagen");
        return false;
    }

    size_t bytes_sent = 0;
    while (bytes_sent < buffer.size()) {
        size_t chunk_size = std::min(CHUNK_SIZE, buffer.size() - bytes_sent);
        ssize_t sent = send(sockfd, buffer.data() + bytes_sent, chunk_size, 0);
        if (sent < 0) {
            perror("❌ Error al enviar fragmento de la imagen");
            return false;
        }
        bytes_sent += sent;
    }

    return true;
}

int main() {
    signal(SIGINT, signal_handler);

    cap.open(PUERTO_CAM, cv::CAP_V4L2);
    if (!cap.isOpened()) {
        std::cerr << "❌ Error: No se pudo abrir la cámara." << std::endl;
        return -1;
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);
    cap.set(cv::CAP_PROP_FPS, FPS);  // Limitar FPS

    int frame_counter = 0;
    while (true) {
        if (sockfd < 0) {
            sockfd = setup_tcp_socket(SERVER_IP, SERVER_PORT);
            if (sockfd < 0) {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                continue;
            }
        }

        if (!send_frame(sockfd, cap)) {
            std::cerr << "🔄 Intentando reconectar..." << std::endl;
            close(sockfd);
            sockfd = -1;
        }

        // Reducir uso de CPU limitando la velocidad de transmisión
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        if (++frame_counter % 10 == 0) {
            std::cout << "📤 10 frames enviados." << std::endl;
        }
    }

    signal_handler(0);
    return 0;
}
