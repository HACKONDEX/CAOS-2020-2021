#include <stdio.h>
#include <malloc.h>
#include <smmintrin.h>

float dot_line_column(const float *left_matrix,
                      const float *right_matrix,
                      int line_start,
                      int column_start,
                      int length) {
    float answer = 0;
    int current_length = length;
    __m128 first;
    __m128 second;
    int index = 0;
    while (current_length >= 4) {
        first = _mm_loadu_ps(left_matrix + index + line_start);
        second = _mm_loadu_ps(right_matrix + index + column_start);
        answer += _mm_dp_ps(first, second, 0xF1)[0];
        current_length -= 4;
        index += 4;
    }
    for (int i = 0; i < current_length; ++i) {
        answer += left_matrix[line_start + index] * right_matrix[column_start + index];
        ++index;
    }
    return answer;
}

void print_multiply_result(float *left_matrix, float *right_matrix, int result_size, int length) {
    for (int i = 0; i < result_size; ++i) {
        for (int j = 0; j < result_size; ++j) {
            printf("%0.4f ", dot_line_column(left_matrix, right_matrix, i * length, j * length, length));
        }
        printf("\n");
    }
}

int main() {
    int line_count = 0;
    int column_count = 0;
    scanf("%i", &line_count);
    scanf("%i", &column_count);

    int elements_count = line_count * column_count;
    int size_4_divider = elements_count + ((4 - (elements_count % 4)) % 4);

    float *left_matrix = aligned_alloc(4, size_4_divider * sizeof(float));
    float *right_matrix = aligned_alloc(4, size_4_divider * sizeof(float));

    for (int i = 0, index = 0; i < line_count; ++i) {
        for (int j = 0; j < column_count; ++j) {
            scanf("%f", &left_matrix[index++]);
        }
    }

    for (int i = 0, index = 0; i < column_count; ++i) {
        for (int j = 0; j < line_count; ++j) {
            scanf("%f", &right_matrix[index + j * column_count]);
        }
        ++index;
    }

    print_multiply_result(left_matrix, right_matrix, line_count, column_count);

    free(right_matrix);
    free(left_matrix);

    return 0;
}

