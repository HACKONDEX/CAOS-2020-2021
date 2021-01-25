#include <stdio.h>
#include <string.h>

unsigned long long CountMinimalNeededBytesCount(char* c_format_number_16) {
    unsigned long long symbols_count = 0;
    unsigned long long not_zero_symbol_index_in_symbol_count = 0;
    for(unsigned long long i = strlen(c_format_number_16) - 1; i > 1; --i) {
        ++symbols_count;
        if(c_format_number_16[i] != '0') {
            not_zero_symbol_index_in_symbol_count = symbols_count;
        }
    }
    if(not_zero_symbol_index_in_symbol_count == 0) {
        return 1;
    }
    else if(not_zero_symbol_index_in_symbol_count % 2 == 0) {
        return not_zero_symbol_index_in_symbol_count / 2;
    }
    return (not_zero_symbol_index_in_symbol_count / 2) + 1;
}

int main(int args, char* argv[]) {
    for(int i = 1; i < args; ++i) {
        printf("%llu ", CountMinimalNeededBytesCount(argv[i]));
    }
    return 0;
}
