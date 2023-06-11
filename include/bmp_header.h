#ifndef BMP24_BMPHEADER_H
#define BMP24_BMPHEADER_H

#include <cstdint>
;
#pragma pack(push, 1)
struct BmpHeader {
    /// HEADER BEGIN
    char header_field[2];
    std::uint32_t file_size;
    std::uint32_t reserved;
    std::uint32_t offset;
    /// DIB BEGIN
    std::uint32_t header_size;
    std::int32_t bitmap_width;   // intentionally signed
    std::int32_t bitmap_height;  // intentionally signed
    std::uint16_t color_planes;
    std::uint16_t bits_per_pixel;
    std::uint32_t compression_method;
    std::uint32_t image_size;            // can be dummy zero
    std::int32_t horizontal_resolution;  // intentionally signed
    std::int32_t vertical_resolution;    // intentionally signed
    std::uint32_t colors_in_palette;
    std::uint32_t number_of_important_colors;
};
#pragma pack(pop)

#endif //BMP24_BMPHEADER_H
