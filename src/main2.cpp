#include <iostream>
#include <fstream>
#include "bmp_image.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        throw std::runtime_error("Not enough arguments. Pass input_file.bmp and output_file.bmp");
    }

    std::string input_file = argv[1];
    std::string output_file = argv[2];

    BmpImage image;
    {
        std::ifstream ifs(input_file, std::ios::binary);

        if (!ifs.is_open()) {
            throw std::runtime_error("Unable to open input file");
        }
        ifs.exceptions(std::ios::failbit | std::ios::badbit);
        image.read_header(ifs);
        image.check_correctness();
        image.print_info();
        image.read_palette(ifs);
        image.read_bitmap(ifs);
    }
    auto image2 = image.frame_random_color();

    std::ofstream ofs(output_file, std::ios::binary);
    ofs.exceptions(std::ios::failbit | std::ios::badbit);
    if (!ofs.is_open()) {
        throw std::runtime_error("Unable to open output file");
    }

    image2.write(ofs);

    return 0;
}
