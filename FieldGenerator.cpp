#include <cassert>
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
        std::uniform_int_distribution<> dis(0, 10);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                if (x == 0 || x == width - 1 || y == 0 || y == height - 1) {
                    file << '#';  // Walls on the borders
                } else {
                    switch (dis(gen)) {
                        case 0: file << '#'; break;  // Wall
                        case 1:
                        case 2:
                        case 3: file << '.'; break;  // Water
                        default: file << ' '; break;  // Empty
                    }
                }
            }
            if (y < height - 1)
            file << '\n';
        }

        file.close();
    }
};


int main (int argc, char** argv) {
    assert(argc == 3);
    FieldGenerator::generateField("test_field", std::stoi(argv[2]), std::stoi(argv[1]));
    return 0;
}