#include <iostream>
#include <cstdlib>      // Para system()
#include <cstring>      // Para memset()
#include <arpa/inet.h>  // Para sockets UDP
#include <unistd.h>     // Para close()

#define SERVER_IP "192.168.185.254"
#define SERVER_PORT 8001

void pokemon(std::string text, int num){
    // Ejecutar espeak
std::string command = "espeak -v es+m4 -s 100 -p 100 -g 1 \"" + text + "\"";
std::cout << "🗣️ Reproduciendo mensaje...\n";
system(command.c_str());
if (num == 1){
 // 🔹 Ruta del archivo MP3
 std::string mp3_file = "/home/sebas/Desktop/PETER-DEF/PC/Audio/pikachu.mp3";

 // 🔹 Construir comando para reproducir el MP3
 std::string comando_mp3 = "mpg123 " + mp3_file;

 // 🔹 Reproducir el MP3
 std::cout << "🎵 Reproduciendo " << mp3_file << "...\n";
 int mp3_result = system(comando_mp3.c_str());

 if (mp3_result != 0) {
     std::cerr << "❌ Error al reproducir el archivo MP3.\n";

 }
}

return;
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

    char buffer[1024];
    socklen_t serverLen = sizeof(serverAddr);

    while (true) {
        // Esperar respuesta del servidor
        int received = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&serverAddr, &serverLen);
        if (received < 0) {
            std::cerr << "❌ Error al recibir datos.\n";
            close(sockfd);
            return 1;
        }

        buffer[received] = '\0';  // Convertir a string válido

        std::cout << "📨 Mensaje recibido: " << buffer << "\n";

        // Separar número y texto
        std::string message(buffer);
        size_t commaPos = message.find(',');
        if (commaPos != std::string::npos) {
            int number = std::stoi(message.substr(0, commaPos));
            std::string text = message.substr(commaPos + 1);

            // Mostrar número y texto
            std::cout << "Número: " << number << "\nTexto: " << text << "\n";
            pokemon(text, number);
         
        } else {
            std::cerr << "❌ Formato inválido.\n";
        }

        // Enviar "LISTO" para indicar que está listo para el siguiente mensaje
        sendto(sockfd, "LISTO", 5, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        std::cout << "✅ Listo para recibir otro mensaje.\n";
    }

    // Cerrar el socket
    close(sockfd);
    return 0;
}
