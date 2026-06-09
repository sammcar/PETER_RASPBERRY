#include <iostream>
#include <cstdlib>  // Para system()

int main() {

    // 🔹 Ajustar volumen al 100%
    system("amixer set Master 120%");
    // 🔹 Mensaje de espeak como variable
    std::string mensaje = "¡Hola! Eso que veo es un pikachu.";

    // 🔹 Construir el comando de espeak con la variable
    std::string comando_espeak = "espeak -v es+m4 -s 110 -p 100 -g 1 \"" + mensaje + "\"";

    // 🔹 Ejecutar espeak
    std::cout << "🗣️  Hablando con espeak...\n";
    int espeak_result = system(comando_espeak.c_str());

    if (espeak_result != 0) {
        std::cerr << "❌ Error al ejecutar espeak.\n";
        return 1;
    }


    // 🔹 Ruta del archivo MP3
    std::string mp3_file = "/home/sebas/Desktop/PETER-DEF/PI3/Audio/pikachu.mp3";

    // 🔹 Construir comando para reproducir el MP3
    std::string comando_mp3 = "mpg123 " + mp3_file;

    // 🔹 Reproducir el MP3
    std::cout << "🎵 Reproduciendo " << mp3_file << "...\n";
    int mp3_result = system(comando_mp3.c_str());

    if (mp3_result != 0) {
        std::cerr << "❌ Error al reproducir el archivo MP3.\n";
        return 1;
    }

    return 0;
}
