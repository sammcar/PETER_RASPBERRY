#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    // Abrir la cámara (0 generalmente es la primera cámara USB)
    cv::VideoCapture cap(2);  // Asegura que la cámara se abra

    // Verificar si la cámara se abrió correctamente
    if (!cap.isOpened()) {
        std::cerr << "Error: No se pudo abrir la cámara." << std::endl;
        return -1;
    }

    cv::Mat frame;

    while (true) {
        // Capturar frame por frame
        cap >> frame;  // Leer un nuevo frame

        // Verificar si el frame fue leído correctamente
        if (frame.empty()) {
            std::cerr << "Error: No se pudo leer el frame." << std::endl;
            break;
        }

        // Mostrar el frame
        cv::imshow("Cámara USB", frame);

        // Salir del bucle si se presiona la tecla 'q'
        if (cv::waitKey(1) == 'q') {
            break;
        }
    }

    // Liberar la cámara y cerrar las ventanas
    cap.release();
    cv::destroyAllWindows();

    return 0;
}
