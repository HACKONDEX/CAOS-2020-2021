#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <limits.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/types.h>

void make_zero_terminated(char *str) {
    for (int i = 0; str[i] != 0; ++i) {
        if (str[i] == '\n') {
            str[i] = '\0';
            break;
        }
    }
}

int main() {
    char *file_name = calloc(PATH_MAX, sizeof(char));
    int64_t regular_files_size_sum = 0;
    struct stat file_attributes;
    while (fgets(file_name, PATH_MAX, stdin) != 0) {
        make_zero_terminated(file_name);
        if (lstat(file_name, &file_attributes) == 0) {
            if (S_ISREG(file_attributes.st_mode)) {
                regular_files_size_sum += file_attributes.st_size;
            }
        }
    }
    printf("%" PRId64, regular_files_size_sum);
    free(file_name);
    return 0;
}
