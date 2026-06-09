#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#define PORT 8003
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
    int sock;
    struct sockaddr_in server_address;
    char buffer[2];

    // Crear el socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Error al crear el socket");
        return 1;
    }

    // Configurar dirección del servidor
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr) <= 0) {
        perror("Dirección inválida");
        return 1;
    }

    // Conectar al servidor
    if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Error en la conexión");
        return 1;
    }

    std::cout << "Conectado al servidor.\n";

    // Configurar el puerto serial
    int serial_fd = configurarSerial(SERIAL_PORT);
    if (serial_fd == -1) {
        return 1;
    }

    while (true) {
        // Recibir el carácter del servidor
        memset(buffer, 0, sizeof(buffer));
        recv(sock, buffer, 1, 0);
        std::cout << "Recibido: " << buffer[0] << "\n";

        // Enviar el carácter por el puerto serial
        write(serial_fd, buffer, 1);
        std::cout << "Enviado al puerto serial: " << buffer[0] << "\n";

        // Enviar confirmación al servidor
        char confirmacion = '1';
        send(sock, &confirmacion, 1, 0);
        std::cout << "Confirmación enviada.\n";
    }

    close(serial_fd);
    close(sock);
    return 0;
}
