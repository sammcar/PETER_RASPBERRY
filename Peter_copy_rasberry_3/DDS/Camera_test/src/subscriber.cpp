#include <dds/dds.h>
#include <opencv2/opencv.hpp>
#include "CameraData.h"  // Generado a partir del IDL

#define DEFAULT_RELIABILITY 1  // 0 = Best Effort, 1 = Reliable

int main(int argc, char* argv[]) {
    // Configuración de confiabilidad
    int reliability = DEFAULT_RELIABILITY;
    if (argc > 1) reliability = std::stoi(argv[1]);

    // Crear participante DDS
    dds_entity_t participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
    if (participant < 0) {
        std::cerr << "Error: No se pudo crear el participante DDS. Código: " << participant << std::endl;
        return -1;
    }

    // Configurar QoS para confiabilidad
    dds_qos_t *qos = dds_create_qos();
    dds_qset_reliability(qos, reliability ? DDS_RELIABILITY_RELIABLE : DDS_RELIABILITY_BEST_EFFORT, DDS_SECS(1));

    // Crear topic
    dds_entity_t topic = dds_create_topic(participant, &ImageData_desc, "CameraImage", qos, NULL);
    if (topic < 0) {
        std::cerr << "Error: No se pudo crear el topic. Código: " << topic << std::endl;
        dds_delete(participant);
        return -1;
    }

    // Crear reader con QoS configurado
    dds_entity_t reader = dds_create_reader(participant, topic, qos, NULL);
    dds_delete_qos(qos);  // Liberar QoS después de asignarlo
    if (reader < 0) {
        std::cerr << "Error: No se pudo crear el reader. Código: " << reader << std::endl;
        dds_delete(participant);
        return -1;
    }

    std::cout << "📥 Subscriptor de imágenes iniciado (Reliability: "
              << (reliability ? "Reliable" : "Best Effort") << ")" << std::endl;

    // Buffer para recibir imágenes
    ImageData img_msg{};
    void *samples[1] = {&img_msg};
    dds_sample_info_t info[1];

    while (true) {
        // Intentar recibir datos
        dds_return_t ret = dds_take(reader, samples, info, 1, 1);
        if (ret > 0 && info[0].valid_data) {
            if (!img_msg.data._buffer || img_msg.data._length == 0) {
                std::cerr << "⚠️ Imagen recibida con buffer vacío." << std::endl;
                continue;
            }

            // Convertir buffer a vector y decodificar
            std::vector<uchar> encoded_image(img_msg.data._buffer, img_msg.data._buffer + img_msg.data._length);
            cv::Mat frame = cv::imdecode(encoded_image, cv::IMREAD_COLOR);

            if (!frame.empty()) {
                std::cout << "✅ Imagen recibida: " << img_msg.width << "x" << img_msg.height 
                          << " (" << img_msg.data._length << " bytes)" << std::endl;
                cv::imshow("📷 Imagen Recibida", frame);
                if (cv::waitKey(1) == 27) break;  // Presionar ESC para salir
            } else {
                std::cerr << "⚠️ Error al decodificar la imagen." << std::endl;
            }
        }

        // Pequeña pausa para evitar alto consumo de CPU
        dds_sleepfor(DDS_MSECS(30));
    }

    // Liberar recursos
    dds_delete(reader);
    dds_delete(participant);
    return 0;
}
