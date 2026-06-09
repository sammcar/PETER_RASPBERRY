#include <dds/dds.h>
#include <opencv2/opencv.hpp>
#include <chrono>
#include <thread>
#include <vector>
#include <iostream>
#include "CameraData.h"  // Generado a partir del IDL

#define DEFAULT_CAM_NUM 0
#define DEFAULT_JPEG_QUALITY 35
#define DEFAULT_PUBLISH_RATE 100  // Milisegundos entre publicaciones
#define DEFAULT_RELIABILITY 0  // 0 = Best Effort, 1 = Reliable

int main(int argc, char* argv[]) {
    int cam_num = (argc > 1) ? std::stoi(argv[1]) : DEFAULT_CAM_NUM;
    int jpeg_quality = (argc > 2) ? std::stoi(argv[2]) : DEFAULT_JPEG_QUALITY;
    int publish_rate = (argc > 3) ? std::stoi(argv[3]) : DEFAULT_PUBLISH_RATE;
    int reliability = (argc > 4) ? std::stoi(argv[4]) : DEFAULT_RELIABILITY;

    dds_entity_t participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
    if (participant < 0) return -1;

    dds_qos_t *qos = dds_create_qos();
    dds_qset_history(qos, DDS_HISTORY_KEEP_LAST, 1);
    dds_qset_reliability(qos, reliability ? DDS_RELIABILITY_RELIABLE : DDS_RELIABILITY_BEST_EFFORT, DDS_SECS(1));

    dds_entity_t topic = dds_create_topic(participant, &ImageData_desc, "CameraImage", qos, NULL);
    dds_entity_t writer = dds_create_writer(participant, topic, qos, NULL);
    dds_delete_qos(qos);  // Liberar QoS

    if (topic < 0 || writer < 0) {
        dds_delete(participant);
        return -1;
    }

    cv::VideoCapture cap(cam_num);
    if (!cap.isOpened()) {
        dds_delete(participant);
        return -1;
    }

    std::cout << "📸 Publicador iniciado (Cam: " << cam_num 
              << ", JPEG: " << jpeg_quality 
              << ", Rate: " << publish_rate << "ms"
              << ", Reliability: " << (reliability ? "Reliable" : "Best Effort") << ")"
              << std::endl;

    cv::Mat frame, resized_frame;
    std::vector<int> compression_params = {cv::IMWRITE_JPEG_QUALITY, jpeg_quality};
    std::vector<uchar> encoded_image;
    encoded_image.reserve(640 * 480);  // Prealocar memoria para reducir reasignaciones

    while (true) {
        auto start_time = std::chrono::steady_clock::now();

        if (!cap.read(frame)) continue;  // Captura sin mensaje de error repetitivo

        cv::resize(frame, resized_frame, cv::Size(640, 480), 0, 0, cv::INTER_AREA);

        encoded_image.clear();  // Reusar memoria en lugar de crear un nuevo vector
        if (!cv::imencode(".jpg", resized_frame, encoded_image, compression_params)) continue;

        ImageData img_msg;
        img_msg.width = resized_frame.cols;
        img_msg.height = resized_frame.rows;
        img_msg.data._length = encoded_image.size();
        img_msg.data._buffer = encoded_image.data();

        dds_write(writer, &img_msg);

        std::this_thread::sleep_for(std::chrono::milliseconds(publish_rate) - 
                                    (std::chrono::steady_clock::now() - start_time));
    }

    dds_delete(participant);
    return 0;
}
