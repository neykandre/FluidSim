#include <fstream>
#include <random>
#include <string>

class FieldGenerator {
public:
    static void generateField(const std::string& filename, int width, int height) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open file for writing");
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 2);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                if (x == 0 || x == width - 1 || y == 0 || y == height - 1) {
                    file << '#';  // Walls on the borders
                } else {
                    switch (dis(gen)) {
                        case 0: file << '#'; break;  // Wall
                        case 1: file << '.'; break;  // Water
                        case 2: file << ' '; break;  // Empty
                    }
                }
            }
            file << '\n';
        }

        file.close();
    }
};


int main () {
    FieldGenerator::generateField("field1", 1920, 1080);
    return 0;
}