// tum_camera_view.cpp
#include <opencv2/opencv.hpp>
#include <iostream>
#include <csignal>

static volatile bool running = true;
void handle_signal(int) { running = false; }

int main() {
    std::signal(SIGINT, handle_signal);

    cv::VideoCapture cap("/dev/peter_cam", cv::CAP_V4L2);

    if (!cap.isOpened()) {
        std::cerr << "[ERROR] No se pudo abrir /dev/peter_cam\n";
        return 1;
    }

    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M','J','P','G'));
    cap.set(cv::CAP_PROP_FRAME_WIDTH,  1280);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    cap.set(cv::CAP_PROP_FPS,          30);
    cap.set(cv::CAP_PROP_BUFFERSIZE,   1);

    double w   = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    double h   = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double fps = cap.get(cv::CAP_PROP_FPS);
    std::cout << "[INFO] Camara lista: " << w << "x" << h << " @ " << fps << " fps\n";

    cv::Mat frame;
    double t0 = cv::getTickCount();
    int    fc  = 0;

    while (running) {
        if (!cap.read(frame) || frame.empty()) {
            std::cerr << "[WARN] Frame vacio, reintentando...\n";
            continue;
        }

        // --- Tu procesamiento va aqui ---

        if (++fc % 60 == 0) {
            double elapsed = (cv::getTickCount() - t0) / cv::getTickFrequency();
            std::printf("[INFO] FPS: %.1f\n", fc / elapsed);
            fc = 0;
            t0 = cv::getTickCount();
        }

        cv::imshow("TUM Camera", frame);
        if (cv::waitKey(1) == 27) break;
    }

    cap.release();
    cv::destroyAllWindows();
    std::cout << "[INFO] Camara liberada.\n";
    return 0;
}
