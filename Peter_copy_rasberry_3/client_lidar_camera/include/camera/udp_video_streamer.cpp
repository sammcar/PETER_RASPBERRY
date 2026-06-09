#include "udp_video_streamer.hpp"

constexpr int FRAME_WIDTH = 640;
constexpr int FRAME_HEIGHT = 480;
constexpr size_t PACKET_SIZE = 4096;
constexpr int JPEG_QUALITY = 60;
constexpr int MAX_RETRIES = 5; // Número máximo de reintentos antes de detener la transmisión

UDPVideoStreamer::UDPVideoStreamer(const std::string& server_ip, int server_port, int camera_index)
    : server_ip_(server_ip), server_port_(server_port), camera_index_(camera_index), running_(false), sockfd_(-1) {}

UDPVideoStreamer::~UDPVideoStreamer() {
    stop_streaming();
    if (sockfd_ >= 0) {
        close(sockfd_);
    }
}

void UDPVideoStreamer::start_streaming() {
    if (running_) return;

    // Configurar socket UDP
    sockfd_ = setup_udp_socket(server_ip_, server_port_, serverAddr_);
    if (sockfd_ < 0) {
        std::cerr << "❌ Error: No se pudo configurar el socket UDP." << std::endl;
        return;
    }

    // Abrir cámara
    cap_.open(camera_index_);
    if (!cap_.isOpened()) {
        std::cerr << "❌ Error: No se pudo abrir la cámara." << std::endl;
        close(sockfd_);
        return;
    }
    cap_.set(cv::CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
    cap_.set(cv::CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

    // Reservar memoria para evitar reallocaciones
    buffer_.reserve(FRAME_WIDTH * FRAME_HEIGHT * 3);

    // Iniciar hilo de transmisión
    running_ = true;
    streaming_thread_ = std::thread(&UDPVideoStreamer::stream_loop, this);
}

void UDPVideoStreamer::stop_streaming() {
    running_ = false;
    if (streaming_thread_.joinable()) {
        streaming_thread_.join();
    }
    if (cap_.isOpened()) {
        cap_.release();
    }
}

void UDPVideoStreamer::stream_loop() {
    int retries = 0;
    while (running_) {
        if (!send_frame()) {
            retries++;
            std::cerr << "⚠️ Advertencia: Error en el envío del frame. Reintento " << retries << "/" << MAX_RETRIES << std::endl;
            if (retries >= MAX_RETRIES) {
                std::cerr << "❌ Error crítico: No se pudo transmitir el video tras varios intentos." << std::endl;
                break;
            }
        } else {
            retries = 0; // Resetear intentos si la transmisión es exitosa
        }
    }
}

bool UDPVideoStreamer::send_frame() {
    cv::Mat frame;
    cap_ >> frame;
    if (frame.empty()) return false;

    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, JPEG_QUALITY};
    if (!cv::imencode(".jpg", frame, buffer_, params)) return false;

    size_t total_size = buffer_.size();
    socklen_t addr_len = sizeof(serverAddr_);

    for (size_t start = 0; start < total_size; start += PACKET_SIZE) {
        size_t chunk_size = std::min<size_t>(PACKET_SIZE, total_size - start);
        ssize_t sent_bytes = sendto(sockfd_, buffer_.data() + start, chunk_size, 0, 
                                    (struct sockaddr*)&serverAddr_, addr_len);

        if (sent_bytes < 0) {
            perror("❌ Error en sendto");
            return false;
        }
    }
    return true;
}

int UDPVideoStreamer::setup_udp_socket(const std::string& ip, int port, struct sockaddr_in& serverAddr) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("❌ Error al crear el socket UDP");
        return -1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) <= 0) {
        std::cerr << "❌ Error: Dirección IP no válida." << std::endl;
        close(sockfd);
        return -1;
    }

    // Configurar buffer de envío
    int buffer_size = 512 * 1024;
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size)) < 0) {
        perror("⚠️ Advertencia: No se pudo configurar el buffer de envío");
    }

    return sockfd;
}
