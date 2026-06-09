#include <iostream>
#include <cstdlib>  // Para system()

int main() {
    // 🔹 Ajustar volumen al 100%
    system("amixer set Master 120%");

    // 🔹 Ruta del archivo MP3
    std::string mp3_file = "/home/peter/Desktop/audio_cpp/drake2.mp3";

    // 🔹 Reproducir el audio
    std::string command = "mpg123 " + mp3_file;  
    int result = system(command.c_str());

    if (result != 0) {
        std::cerr << "❌ Error al reproducir el archivo MP3.\n";
    } else {
        std::cout << "🎵 Reproduciendo " << mp3_file << "...\n";
    }

    return 0;
}
