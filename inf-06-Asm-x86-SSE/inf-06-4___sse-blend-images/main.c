#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <immintrin.h>

const float norma = 255.0;

struct BMPHeaderUsefulInfo {
  // 8 + 8 + 32 + 32 bits of useless information
  uint32_t bytes_count_before_image_pointer;
  // 32
  int32_t width;
  int32_t height;
  // 16 + 16 + 32 + 32 + 32 + 32 + 32 + 32
  uint32_t redMask;
  uint32_t greenMask;
  uint32_t blueMask;
  uint32_t alphaMask;
};

struct ComponentOrder {
  int alpha;
  int blue;
  int green;
  int red;
};

uint64_t get_bytes_count_of_file(FILE* file) {
    uint64_t bytes_count = 0;
    fseek(file, 0, SEEK_END);
    bytes_count = ftell(file);
    rewind(file);
    return bytes_count;
}

int get_component_index_from_bitmask(uint32_t bitmask) {
    if(bitmask == 0xffu) return 0;
    else if((bitmask >> 8u) == 0xffu) return 1;
    else if((bitmask >> 16u) == 0xffu) return 2;
    else if((bitmask >> 24u) == 0xffu) return 3;
}

void get_data_from_file(char* data, FILE* file, uint64_t bytes_count) {
    fread(data, sizeof(char), bytes_count, file);
    rewind(file);
}

void get_header_from_file(struct BMPHeaderUsefulInfo* header, FILE* file, char* data) {
    data += 10; // 10 useless bytes
    header->bytes_count_before_image_pointer = *(uint32_t*)(data);
    data += 8; // 4 bytes and bytes_count_before_image_pointer
    header->width = *(int32_t*)(data);
    data += 4;
    header->height = *(int32_t*)(data);
    data += 32; // 28 useless bytes
    header->redMask = *(uint32_t*)(data);
    data += 4;
    header->greenMask = *(uint32_t*)(data);
    data += 4;
    header->blueMask = *(uint32_t*)(data);
    data += 4;
    header->alphaMask = *(uint32_t*)(data);
}

void build_result_image(char* filename, char* data, uint64_t bytes_count) {
    FILE* file = fopen(filename, "wb");
    fwrite(data, sizeof(char), bytes_count, file);
    fclose(file);
}

__m128 get_component_in_xmm(const char* image, int index) {
    uint8_t converted_bits[4];
    for(int i = 0; i < 4; ++i) {
        converted_bits[i] = (uint8_t)image[i * 4 + index];
    }
    float converted_to_float[4];
    for(int i = 0; i < 4; ++i) {
        converted_to_float[i] = converted_bits[i] / 255.0;
    }
    return _mm_set_ps(converted_to_float[0],
                      converted_to_float[1],
                      converted_to_float[2],
                      converted_to_float[3]);
}

void write_new_composition_into_final_image(char* back, __m128 composition_xmm, int index) {
    for(int i = 0; i < 4; ++i) {
        back[i * 4 + index] = (char)((uint8_t)composition_xmm[3 - i]);
    }
}

void composite_colours(char* back, char* front, int index, __m128 back_alpha_xmm, __m128 front_alpha_xmm) {
    __m128 front_colour_xmm = get_component_in_xmm(front, index);
    __m128 back_colour_xmm = get_component_in_xmm(back, index);

    front_colour_xmm = _mm_mul_ps(front_colour_xmm, front_alpha_xmm);
    back_colour_xmm = _mm_mul_ps(back_colour_xmm, back_alpha_xmm);

    __m128 composition_xmm = _mm_add_ps(front_colour_xmm, back_colour_xmm);
    back_colour_xmm = _mm_mul_ps(back_colour_xmm, front_alpha_xmm);
    composition_xmm = _mm_sub_ps(composition_xmm, back_colour_xmm);
    __m128 norma_xmm = _mm_set_ps(norma, norma, norma, norma); // norma == 255.0
    composition_xmm = _mm_mul_ps(composition_xmm, norma_xmm);
    write_new_composition_into_final_image(back, composition_xmm, index);
}

void composite_alphas(char* back, __m128 front_alpha_xmm, __m128 back_alpha_xmm, int alpha_index) {
    __m128 composition_xmm = _mm_add_ps(front_alpha_xmm, back_alpha_xmm);
    back_alpha_xmm = _mm_mul_ps(back_alpha_xmm, front_alpha_xmm);
    composition_xmm = _mm_sub_ps(composition_xmm, back_alpha_xmm);
    __m128 norma_xmm = _mm_set_ps(norma, norma, norma, norma); // norma == 255.0
    composition_xmm = _mm_mul_ps(composition_xmm, norma_xmm);
    write_new_composition_into_final_image(back, composition_xmm, alpha_index);
}

void composite_images(char* destination, char* source, struct ComponentOrder* order, uint64_t pixels_count) {
    while (pixels_count >= 4) { // destination - background, source - front
        __m128 back_alpha_xmm = get_component_in_xmm(destination, order->alpha);
        __m128 front_alpha_xmm = get_component_in_xmm(source, order->alpha);
        composite_colours(destination, source, order->blue, back_alpha_xmm, front_alpha_xmm);
        composite_colours(destination, source, order->green, back_alpha_xmm, front_alpha_xmm);
        composite_colours(destination, source, order->red, back_alpha_xmm, front_alpha_xmm);
        composite_alphas(destination, front_alpha_xmm, back_alpha_xmm, order->alpha);
        destination += 16;
        source += 16;
        pixels_count -= 4;
    }
}

int main(int argc, char* argv[]) {
    FILE* destination_file = fopen(argv[1], "rb");
    FILE* source_file = fopen(argv[2], "rb");

    uint64_t destination_bytes_count = get_bytes_count_of_file(destination_file);
    char* destination = (char*)calloc(destination_bytes_count + 1, sizeof(char));
    get_data_from_file(destination, destination_file, destination_bytes_count);

    uint64_t source_bytes_count = get_bytes_count_of_file(source_file);
    char* source = (char*)calloc(source_bytes_count + 1, sizeof(char));
    get_data_from_file(source, source_file, source_bytes_count);

    struct BMPHeaderUsefulInfo destination_header;
    get_header_from_file(&destination_header, destination_file, destination);
    struct BMPHeaderUsefulInfo source_header;
    get_header_from_file(&source_header, source_file, source);

    fclose(source_file);
    fclose(destination_file);

    struct ComponentOrder colours_order;
    colours_order.alpha = get_component_index_from_bitmask(destination_header.alphaMask);
    colours_order.blue  = get_component_index_from_bitmask(destination_header.blueMask);
    colours_order.green = get_component_index_from_bitmask(destination_header.greenMask);
    colours_order.red = get_component_index_from_bitmask(destination_header.redMask);

    char* front_image_pointer = source + source_header.bytes_count_before_image_pointer;
    char* back_image_pointer = destination + destination_header.bytes_count_before_image_pointer;

    composite_images(back_image_pointer, front_image_pointer, &colours_order,
                     destination_header.height * destination_header.width);
    build_result_image(argv[3], destination, destination_bytes_count);

    free(destination);
    free(source);

    return 0;
}


