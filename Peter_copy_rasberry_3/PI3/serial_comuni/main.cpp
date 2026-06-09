#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#define SERVER_PORT 8083

#define SERVER_IP "192.168.185.254" // Cambia a la IP del servidor
#define SERIAL_PORT "/dev/ttyACM0"   // Cambia según el puerto de tu dispositivo

// Función para configurar el puerto serial
int configurarSerial(const char* puerto) {
    int serial_fd = open(puerto, O_RDWR | O_NOCTTY | O_NDELAY);
    if (serial_fd == -1) {
        perror("Error al abrir el puerto serial");
        return -1;
    }

    struct termios options;
    tcgetattr(serial_fd, &options);

    cfsetispeed(&options, B9600); // Velocidad de entrada
    cfsetospeed(&options, B9600); // Velocidad de salida

    options.c_cflag = CS8 | CLOCAL | CREAD; // 8 bits, sin paridad, 1 bit de stop
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;

    tcflush(serial_fd, TCIFLUSH);
    tcsetattr(serial_fd, TCSANOW, &options);

    return serial_fd;
}

int main() {
    // Crear socket UDP
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "❌ Error al crear el socket UDP.\n";
        return 1;
    }

    // Configurar dirección del servidor
    struct sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr) <= 0) {
        std::cerr << "❌ Error con la dirección IP del servidor.\n";
        close(sockfd);
        return 1;
    }

     // Enviar mensaje de solicitud al servidor
    sendto(sockfd, "LISTO", 5, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    std::cout << "📡 Esperando mensaje del servidor...\n";
    // // Conectar al servidor
    // if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
    //     perror("Error en la conexión");
    //     return 1;
    // }

    std::cout << "Conectado al servidor.\n";

    // Configurar el puerto serial
    int serial_fd = configurarSerial(SERIAL_PORT);
    if (serial_fd == -1) {
        return 1;
    }

    char buffer[4];
    socklen_t serverLen = sizeof(serverAddr);


    while (true) {
        // Recibir dos caracteres en formato "x,y"
        memset(buffer, 0, sizeof(buffer));
        int received = recv(sockfd, buffer, 3, 0);  // Recibir 3 bytes: "x,y"
        if (received <= 0) {
            std::cerr << "Error en la recepción o conexión cerrada.\n";
            continue;;
        }

        std::cout << "Recibido: " << buffer << "\n";

        // Enviar los caracteres por el puerto serial
        write(serial_fd, buffer, 3);
        std::cout << "Enviado al puerto serial: " << buffer << "\n";

        // Enviar confirmación al servidor
        char confirmacion = '1';
        send(sockfd, &confirmacion, 1, 0);
        std::cout << "Confirmación enviada.\n";
    }

    close(serial_fd);
    close(sockfd);
    return 0;
}
