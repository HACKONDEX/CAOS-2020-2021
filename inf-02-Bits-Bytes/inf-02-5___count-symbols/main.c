#include <stdio.h>
#include <inttypes.h>

struct Triple {
  int32_t UTF8_count;
  int32_t ASCII_count;
  int32_t return_code;
};

int32_t get_the_bit(int32_t set, unsigned int bit_number_from_right) {
    if ((set & (1 << (bit_number_from_right - 1))) == 0ULL) {
        return 0;
    }
    return 1;
}

struct Triple get_symbols_classification_in_count() {
    struct Triple answer = {0};
    int32_t symbol = 0;
    int32_t any_started_utf = 0;
    int32_t byte_count_left = 0;
    int32_t is_end_of_file = 0;
    while (!is_end_of_file) {
        symbol = getchar();
        if (symbol == EOF) {
            is_end_of_file = 1;
        } else {
            if (any_started_utf == 1) {
                if (get_the_bit(symbol, 8) == 1 &&
                    get_the_bit(symbol, 7) == 0) {
                    --byte_count_left;
                    if (byte_count_left == 0) {
                        ++answer.UTF8_count;
                        any_started_utf = 0;
                    }
                } else {
                    answer.return_code = 1;
                    break;
                }
            } else {
                if (get_the_bit(symbol, 8) == 0) {
                    ++answer.ASCII_count;
                } else {
                    if (get_the_bit(symbol, 7) == 0) {
                        answer.return_code = 1;
                        break;
                    } else {
                        byte_count_left = 1;
                        any_started_utf = 1;
                        int i = 6;
                        while (get_the_bit(symbol, i) != 0) {
                            ++byte_count_left;
                            --i;
                            if (i == 3) {
                                break;
                            }
                        }
                        if (i <= 3) {
                            answer.return_code = 1;
                            break;
                        }
                    }
                }
            }
        }
    }
    return answer;
}

int main() {
    struct Triple symbols_count = get_symbols_classification_in_count();
    printf("%i ", symbols_count.ASCII_count);
    printf("%i", symbols_count.UTF8_count);
    return symbols_count.return_code;
}
