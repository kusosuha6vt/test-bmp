#include "bmp_image.h"
#include <fstream>
#include <iostream>
#include <random>
#include <thread>

void BmpImage::read_header(std::ifstream &ifs) {
    ifs.read(reinterpret_cast<char *>(&header), sizeof(header));
}

void BmpImage::read_palette(std::ifstream &ifs) {
    const std::size_t COLOR_SIZE = 4;
    palette.resize(COLOR_SIZE * header.colors_in_palette);
    if (!palette.empty()) {
        ifs.read(reinterpret_cast<char *>(&palette[0]), palette.size());
    }
}

void BmpImage::read_bitmap(std::ifstream &ifs) {
    ifs.seekg(header.offset);  // there can be a gap
    std::size_t row_size = (header.bits_per_pixel * header.bitmap_width + 31) / 32 * 4;
    std::size_t absHeight = std::abs(header.bitmap_height);  // it can be <0
    bitmap.resize(row_size * absHeight);
    ifs.read(reinterpret_cast<char *>(&bitmap[0]), bitmap.size());
}

void BmpImage::check_correctness() const {
    if (header.header_size != 40) {
        throw std::runtime_error("Unsupported header size");
    }
    if (header.compression_method != 0) {
        throw std::runtime_error("Unsupported compression method");
    }
}

void BmpImage::print_info() const {
    std::cerr << "header field: " << header.header_field[0] << header.header_field[1] << ", height: "
              << header.bitmap_height << ", width: " << header.bitmap_width <<
              ", HEADER SIZE: " << header.header_size << ", bits per pixel: " << header.bits_per_pixel
              << ", colors in table: " << header.colors_in_palette <<
              ", compression: " << header.compression_method << ", bits per pixel: " << header.bits_per_pixel <<
              ", offset: " << header.offset << "\n";
}

void BmpImage::write(std::ofstream &ofs) {
    ofs.write(reinterpret_cast<char *>(&header), sizeof header);
    if (!palette.empty()) {
        ofs.write(reinterpret_cast<char *>(&palette[0]), palette.size());
    }
    std::vector<char> gap(header.offset - sizeof(header) - palette.size());
    ofs.write(&gap[0], gap.size());
    ofs.write(reinterpret_cast<char *>(&bitmap[0]), bitmap.size());
}

void BmpImage::set_pixel(size_t i, size_t j, unsigned int value) {
    size_t row_size = (header.bits_per_pixel * header.bitmap_width + 31) / 32 * 4;
    // row: i * row_size
    // j * bits_per_pixel / 4
    // j * bits_per_pixel % 4
    size_t a = i * row_size + j * header.bits_per_pixel / 8;
    if (header.bits_per_pixel <= 8) {
        size_t b = j * header.bits_per_pixel % 8;
        auto mask = (unsigned char)(((1u << header.bits_per_pixel) - 1u) << (8 - header.bits_per_pixel - b));
        value = (value << (8 - header.bits_per_pixel - b));
        bitmap[a] = std::byte(((unsigned char)bitmap[a] & (unsigned char)~mask) | (mask & value));
    } else if (header.bits_per_pixel == 16) {
        // little-endian
        auto *p = (uint16_t *)(&bitmap[a]);
        *p = value;
    } else if (header.bits_per_pixel == 24) {
        // little-endian
        auto *p1 = (uint8_t *)(&bitmap[a]);
        auto *p2 = (uint16_t *)(p1 + 1);
        *p1 = value % 256u;
        *p2 = (value >> 8u);
    } else {
        throw std::runtime_error("this number of pixels (" + std::to_string(header.bits_per_pixel) + ") is not supported");
    }
}

void BmpImage::make_black_and_white() {
    if (header.colors_in_palette == 0) {
        std::cerr << "Black and white is not supported when palette is empty\n";
        return;
    }
    for (int i = 0; i < header.colors_in_palette; i++) {
        char *bgr = reinterpret_cast<char *>(&palette[i * 4]);
        int new_color = (int(bgr[0]) + bgr[1] + bgr[2]) / 3;
        bgr[0] = bgr[1] = bgr[2] = char(new_color);
    }
}

BmpImage BmpImage::frame_random_color() {
    static std::random_device gen;
    BmpImage other = *this;

    size_t random_color = std::uniform_int_distribution<>(0, header.colors_in_palette - 1)(gen);
    other.header.bitmap_height = (header.bitmap_height < 0 ? -1 : 1) * (std::abs(header.bitmap_height) + 30);
    other.header.bitmap_width = header.bitmap_width + 30;
    int n = (other.header.bitmap_width * header.bits_per_pixel + 31) / 32 * 4 * std::abs(other.header.bitmap_height);
    other.bitmap.assign(n, std::byte{});
    for (int i = 0; i < std::abs(other.header.bitmap_height); ++i) {
        for (int j = 0; j < other.header.bitmap_width; ++j) {
            if (std::min({
                i + 1, j + 1, other.header.bitmap_width - j, std::abs(other.header.bitmap_height) - i
            }) <= 15) {
                other.set_pixel(i, j, random_color);
            } else {
                other.set_pixel(i, j, get_pixel(i - 15, j - 15));
            }
        }
    }

    other.header.file_size += other.bitmap.size() - bitmap.size();  // new pixels
    other.header.image_size = other.bitmap.size();
    return other;
}

unsigned int BmpImage::get_pixel(size_t i, size_t j) const {
    size_t row_size = (header.bits_per_pixel * header.bitmap_width + 31) / 32 * 4;
    // row: i * row_size
    // j * bits_per_pixel / 4
    // j * bits_per_pixel % 4
    size_t a = i * row_size + j * header.bits_per_pixel / 8;
    if (header.bits_per_pixel <= 8) {
        size_t b = j * header.bits_per_pixel % 8;
        auto mask = (1u << header.bits_per_pixel) - 1u;
        return ((unsigned char)bitmap[a] >> (8 - header.bits_per_pixel - b)) & mask;
    } else if (header.bits_per_pixel == 16) {
        // little-endian
        auto *p = (uint16_t *)(&bitmap[a]);
        return *p;
    } else if (header.bits_per_pixel == 24) {
        // little-endian
        auto *p1 = (uint8_t *)(&bitmap[a]);
        auto *p2 = (uint16_t *)(p1 + 1);
        return *p1 + (*p2 << 8);
    } else {
        throw std::runtime_error("this number of pixels (" + std::to_string(header.bits_per_pixel)
        + ") is not supported");
    }
}

size_t BmpImage::add_color(unsigned int b, unsigned int g, unsigned int r) {
    if (this->header.colors_in_palette == 0) {
        throw std::runtime_error("can't add colors if palette is empty");
    }
    header.offset += 4;
    size_t result = header.colors_in_palette;
    header.colors_in_palette++;
    header.file_size += 4;
    header.number_of_important_colors++;

    palette.push_back((std::byte)b);
    palette.push_back((std::byte)g);
    palette.push_back((std::byte)r);
    palette.push_back((std::byte)0);

    return result;
}
