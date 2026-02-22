#include <iostream>
#include <fstream>
#include <cstdint>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <image_file> <boxes_file>\n";
        return 1;
    }

    std::ifstream imgFile(argv[1], std::ios::binary);
    if (!imgFile) {
        std::cerr << "Could not open image file.\n";
        return 1;
    }

    uint16_t width;
    uint16_t height;
    imgFile.read(reinterpret_cast<char*>(&width), sizeof(width));
    imgFile.read(reinterpret_cast<char*>(&height), sizeof(height));

    if (width == 0 || height == 0 || width > 1024 || height > 1024) {
        std::cerr << "Invalid image dimensions.\n";
        return 1;
    }

    size_t img_size = static_cast<size_t>(width) * height;
    uint8_t* image = new uint8_t[img_size];
    imgFile.read(reinterpret_cast<char*>(image), static_cast<std::streamsize>(img_size));
    imgFile.close();

    size_t sat_width = static_cast<size_t>(width) + 1;
    size_t sat_height = static_cast<size_t>(height) + 1;
    size_t sat_size = sat_width * sat_height;
    int* prefix_sum = new int[sat_size]();

    for (int y = 1; y <= height; ++y) {
        for (int x = 1; x <= width; ++x) {
            size_t idx = static_cast<size_t>(y) * sat_width + x;
            uint8_t img_val = image[static_cast<size_t>(y - 1) * width + (x - 1)];
            prefix_sum[idx] = img_val
                            + prefix_sum[static_cast<size_t>(y - 1) * sat_width + x]
                            + prefix_sum[static_cast<size_t>(y) * sat_width + x - 1]
                            - prefix_sum[static_cast<size_t>(y - 1) * sat_width + x - 1];
        }
    }

    delete[] image;

    std::ifstream boxesFile(argv[2]);
    if (!boxesFile) {
        std::cerr << "Could not open boxes file.\n";
        delete[] prefix_sum;
        return 1;
    }

    int N;
    boxesFile >> N;
    if (N <= 0) {
        std::cerr << "Invalid number of rectangles.\n";
        delete[] prefix_sum;
        return 1;
    }

    for (int i = 0; i < N; ++i) {
        int x1;
        int y1;
        int x2;
        int y2;
        boxesFile >> x1 >> y1 >> x2 >> y2;

        if (x1 < 1 || y1 < 1 || x2 > width || y2 > height || x1 > x2 || y1 > y2) {
            std::cout << "wrong box\n";
            continue;
        }

        size_t w = sat_width;
        int A = prefix_sum[static_cast<size_t>(y2) * w + x2];
        int B = prefix_sum[static_cast<size_t>(y2) * w + (x1 - 1)];
        int C = prefix_sum[static_cast<size_t>(y1 - 1) * w + x2];
        int D = prefix_sum[static_cast<size_t>(y1 - 1) * w + (x1 - 1)];
        int sum = A - B - C + D;
        std::cout << sum << '\n';
    }

    boxesFile.close();
    delete[] prefix_sum;
    return 0;
}
