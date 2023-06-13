#include <iostream>
#include <fstream>
#include <random>
#include "bmp_image.h"

int solve(int argc, char *argv[]) {
    if (argc != 3) {
        throw std::runtime_error("Not enough arguments. Pass input_file.bmp and output_file.bmp");
    }

    std::string input_file = argv[1];
    std::string output_file = argv[2];
    const char *logo_path = "..\\images\\cat-logo.bmp";

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
    BmpImage logo;
    {
        std::ifstream ifs(logo_path, std::ios::binary);

        if (!ifs.is_open()) {
            throw std::runtime_error("Unable to open logo file");
        }
        ifs.exceptions(std::ios::failbit | std::ios::badbit);
        logo.read_header(ifs);
        logo.check_correctness();
        logo.print_info();
        logo.read_palette(ifs);
        logo.read_bitmap(ifs);
    }

    if (image.header.bitmap_width < logo.header.bitmap_width || image.header.bitmap_height < logo.header.bitmap_height) {
        throw std::runtime_error("the logo is bigger than the image");
    }

    size_t color{};
    if (image.header.colors_in_palette > 0) {
        std::cerr << "Image is using a palette\n";
        if ((1 << image.header.bits_per_pixel) > image.header.colors_in_palette) {
            std::cerr << "We can add one more color to the palette (black)\n";
            color = image.add_color(255, 255, 255);
        } else {
            std::cerr << "We can't add more colors to the palette, so choose random\n";
            auto uid = std::uniform_int_distribution<>(0u, (1u << image.header.bits_per_pixel) - 1u);
            std::random_device gen;
            color = uid(gen);
        }
    } else {
        // if bits < 0 => color will be cut
        color = -1;  // black
    }

    BmpImage output = image;
    const int x0 = 0, y0 = 0;  // padding
    for (int i = 0; i < logo.header.bitmap_height; ++i) {
        for (int j = 0; j < logo.header.bitmap_width; ++j) {
            if (logo.get_pixel(i, j) == 0) {
                output.set_pixel(x0 + i, y0 + j, color);
            }
        }
    }

    std::ofstream ofs(output_file, std::ios::binary);
    ofs.exceptions(std::ios::failbit | std::ios::badbit);
    if (!ofs.is_open()) {
        throw std::runtime_error("Unable to open output file");
    }
    output.write(ofs);

    return 0;
}

int main(int argc, char *argv[]) {
    try {
        return solve(argc, argv);
    }
    catch (std::exception& e) {
        std::cerr << e.what();
    }
}