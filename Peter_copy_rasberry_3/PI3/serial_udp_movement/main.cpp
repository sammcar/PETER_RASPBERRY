#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#define PORT 8003
#define SERIAL_PORT "/dev/ttyACM0"

int configurarSerial(const char* puerto) {
    int serial_fd = open(puerto, O_RDWR | O_NOCTTY | O_NDELAY);
    if (serial_fd == -1) {
        perror("Error al abrir el puerto serial");
        return -1;
    }

    struct termios options;
    tcgetattr(serial_fd, &options);

    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);

    options.c_cflag = CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;

    tcflush(serial_fd, TCIFLUSH);
    tcsetattr(serial_fd, TCSANOW, &options);

    return serial_fd;
}

int main(int argc, char* argv[]) {
    const char* SERVER_IP = (argc >= 2) ? argv[1] : "11.11.41.5";

    int sock;
    struct sockaddr_in server_address;
    char buffer[2];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Error al crear el socket");
        return 1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr) <= 0) {
        perror("Dirección inválida");
        return 1;
    }

    if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Error en la conexión");
        return 1;
    }

    std::cout << "Conectado al servidor.\n";

    int serial_fd = configurarSerial(SERIAL_PORT);
    if (serial_fd == -1) {
        return 1;
    }

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        recv(sock, buffer, 1, 0);
        std::cout << "Recibido: " << buffer[0] << "\n";

        char serial_buf[2] = { buffer[0], '\n' };
        write(serial_fd, serial_buf, 2);
        std::cout << "Enviado al puerto serial: " << buffer[0] << "\\n\n";

        char confirmacion = '1';
        send(sock, &confirmacion, 1, 0);
        std::cout << "Confirmación enviada.\n";
    }

    close(serial_fd);
    close(sock);
    return 0;
}

