#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <assert.h>
#include <stdio.h>

/// Prints list's element's, returns 0 if everything is ok
int print_list_elements(int file_fd) {
    int return_code = 0;
    int value = 0;
    uint32_t next_pointer = 0;
    int is_end_of_list = 0;
    int value_bytes_count = 0;
    int pointer_bytes_count = 0;
    while (is_end_of_list != 1) {
        value_bytes_count = read(file_fd, &value, 4);
        pointer_bytes_count = read(file_fd, &next_pointer, 4);
        if (value_bytes_count != 4 || pointer_bytes_count != 4) {
            return_code = -1;
            is_end_of_list = 1;
        } else {
            printf("%d\n", value);
            if (next_pointer == 0) {
                is_end_of_list = 1;
            } else {
                lseek(file_fd, next_pointer, SEEK_SET);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    assert(argc >= 2);
    int file_fd = open(argv[1], O_RDONLY);
    if (file_fd < 0) {
        return 1;
    }
    if (print_list_elements(file_fd) != 0) {
        close(file_fd);
        return 1;
    }
    close(file_fd);
    return 0;
}
