#ifndef BMP24_BMP_IMAGE_H
#define BMP24_BMP_IMAGE_H
#include "bmp_header.h"
#include <vector>
#include <iosfwd>

struct BmpImage {
    BmpHeader header;
    std::vector<std::byte> palette;
    std::vector<std::byte> bitmap;

    void read_header(std::ifstream &ifs);
    void read_palette(std::ifstream &ifs);
    void read_bitmap(std::ifstream &ifs);

    void print_info() const;

    void check_correctness() const;

    void write(std::ofstream &ofs);

    void make_black_and_white();

    BmpImage frame_random_color();

    void set_pixel(size_t i, size_t j, unsigned int value);
    unsigned int get_pixel(size_t i, size_t j) const;

    size_t add_color(unsigned int b, unsigned int g, unsigned int r);
};

#endif //BMP24_BMP_IMAGE_H
