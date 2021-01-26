#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>

//#define IMAXABS(x, y) (x * x >= y * y) ? abs(x) : abs(y)

int32_t get_number(int32_t i, int32_t j, int32_t matrix_size) {
    i = 2 * i - matrix_size + 1;
    j = 2 * j - matrix_size + 1;
    int32_t square_num = (abs(i) >= abs(j) ? abs(i) : abs(j));
    int32_t diag_dist = (i + j) / 2;
    if (i > j) {
        diag_dist = 2 * square_num - diag_dist;
    }
    return matrix_size * matrix_size - square_num * square_num - square_num + diag_dist;
}

struct SpiralMatrixBuilder {
  int32_t i;
  int32_t j;
  int32_t size;
  int32_t cell_size;
  char *buffer;
};

void initialize_matrix_builder(struct SpiralMatrixBuilder *builder,
                               int32_t matrix_size,
                               int32_t cell_size,
                               char *buffer) {
    builder->i = 0;
    builder->j = 0;
    builder->size = matrix_size;
    builder->cell_size = cell_size;
    builder->buffer = buffer;
}

void write_current_number(struct SpiralMatrixBuilder *builder) {
    int32_t current_number = get_number(builder->i, builder->j, builder->size);
    int32_t buffer_index = (builder->i * builder->size + builder->j) * builder->cell_size + builder->i;
    int32_t symbol_position = builder->cell_size - 1;
    while (current_number > 0) {
        builder->buffer[buffer_index + symbol_position] = (current_number % 10) + '0';
        --symbol_position;
        current_number /= 10;
    }
    while (symbol_position >= 0) {
        builder->buffer[buffer_index + symbol_position] = ' ';
        --symbol_position;
    }
}

void add_next_line_symbol(struct SpiralMatrixBuilder *builder) {
    builder->buffer[(builder->i * builder->size + builder->j) * builder->cell_size + builder->i] = '\n';
}

void build_spiral_matrix(struct SpiralMatrixBuilder *builder) {
    for (; builder->i < builder->size; ++builder->i) {
        for (builder->j = 0; builder->j < builder->size; ++builder->j) {
            write_current_number(builder);
        }
        add_next_line_symbol(builder);
    }
}

int main(int argc, char *argv[]) {

    int32_t matrix_size = atoi(argv[2]);
    int32_t cell_size = atoi(argv[3]);
    int32_t buffer_size = matrix_size * matrix_size * cell_size + matrix_size;
    int file_fd = open(argv[1], O_RDWR | O_CREAT, 0664);
    ftruncate(file_fd, buffer_size);
    void *mapped = mmap(NULL, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, file_fd, 0);
    char *buffer = (char *) (mapped);

    struct SpiralMatrixBuilder builder;
    initialize_matrix_builder(&builder, matrix_size, cell_size, buffer);
    build_spiral_matrix(&builder);

    munmap(mapped, buffer_size);
    close(file_fd);
    return 0;
}
