#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <array>

#define SIZE_DATOS 400
struct LidarData {
    uint64_t timestamp;
    double angle;
    int distance;
    int intensity;
};

int main() {
    const char* command = "/home/sebas/Desktop/pi3/Lidar/ldlidar_stl_sdk-master/build/ldlidar_stl_node LD19 serialcom /dev/ttyUSB0";
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        std::cerr << "❌ Error: No se pudo ejecutar el comando.\n";
        return 1;
    }

    char buffer[512];
    std::array<LidarData, SIZE_DATOS> dataBuffer;
    int count = 0;

    while (std::fgets(buffer, sizeof(buffer), pipe)) {
        if (char* start = std::strstr(buffer, "stamp:")) {
            LidarData tempData; // Variable temporal para almacenar los datos

            if (sscanf(start, "stamp:%lu,angle:%lf,distance(mm):%d,intensity:%d", 
                       &tempData.timestamp, &tempData.angle, 
                       &tempData.distance, &tempData.intensity) == 4) {
                
                // ✅ Validar que la distancia no sea 0
                if (tempData.distance > 0) {
                    dataBuffer[count++] = tempData;

                    if (count == SIZE_DATOS) {
                        std::cout << "🔹 Imprimiendo lote de 200 datos:\n";
                        for (const auto& data : dataBuffer) {
                            std::cout << "Timestamp: " << data.timestamp
                                      << " | Ángulo: " << data.angle
                                      << " | Distancia: " << data.distance
                                      << " | Intensidad: " << data.intensity << '\n';
                        }
                        count = 0; // Reiniciar contador
                    }
                }
            }
        }
    }

    pclose(pipe);
    return 0;
}
