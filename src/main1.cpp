#include <iostream>
#include <fstream>
#include <vector>
#include "bmp_image.h"

int solve(int argc, char *argv[]) {
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
    image.make_black_and_white();
    std::ofstream ofs(output_file, std::ios::binary);
    ofs.exceptions(std::ios::failbit | std::ios::badbit);
    if (!ofs.is_open()) {
        throw std::runtime_error("Unable to open output file");
    }
    image.write(ofs);

    return 0;
}

int main(int argc, char* argv[]) {
    try {
        return solve(argc, argv);
    }
    catch (std::exception& e) {
        std::cerr << e.what();
    }
}
