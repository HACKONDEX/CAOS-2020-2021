#include <stdio.h>
#include <inttypes.h>

const int size_of_the_set = 62;
const int numbers_count_in_set = 10;
const int alphabet_size = 26;

void set_the_bit_1(uint64_t* set,unsigned int bit_number_from_right) {
    *set |= (1ULL << (bit_number_from_right - 1));
}

char get_the_bit(uint64_t set, unsigned int bit_number_from_right) {
    if ((set & (1ULL << (bit_number_from_right - 1))) == (uint64_t)(0ULL)) {
        return 0;
    }
    return 1;
}

void add_symbol_to_set(uint64_t* set, char symbol) {
    int code = (int)(symbol);
    if (code >= '0' && code <= '9') {
        set_the_bit_1(set, code - '0' + 1);
    } else if (code >= 'A' && code <= 'Z') {
        set_the_bit_1(set, code - 'A' + numbers_count_in_set + 1);
    } else {
        set_the_bit_1(set, code - 'a' + numbers_count_in_set + alphabet_size + 1);
    }
}

uint64_t do_operations_with_sets() {
    uint64_t result_set = 0ULL;
    uint64_t current_set = 0ULL;
    int symbol_code = 0;
    char symbol = 0;
    while (1) {
        symbol_code = getchar();
        symbol = (char)(symbol_code);
        if(symbol == '\n' || symbol_code == EOF) {
            break;
        }
        switch (symbol) {
            case '&':
                result_set &= current_set;
                current_set = 0;
                break;
            case '|':
                result_set |= current_set;
                current_set = 0;
                break;
            case '^':
                result_set ^= current_set;
                current_set = 0;
                break;
            case '~':
                result_set = ~result_set;
                current_set = 0;
                break;
            default:
                add_symbol_to_set(&current_set, symbol);
        }
    }
    return result_set;
}

void print_result_set(uint64_t result) {
    for(int i = 1; i <= size_of_the_set; ++i) {
        if (get_the_bit(result, i) == 1) {
            if (i >= 1 && i <= numbers_count_in_set) {
                printf("%i", i - 1);
            } else if (i >= 11 && i <= numbers_count_in_set + alphabet_size) {
                printf("%c", i + 'A' - numbers_count_in_set - 1);
            } else {
                printf("%c", i + 'a' - alphabet_size - numbers_count_in_set - 1);
            }
        }
    }
}

int main() {
    print_result_set(do_operations_with_sets());
    return 0;
}
