#include <stdio.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>

const int MAX_SIZE = PATH_MAX + 1024;

void make_zero_terminated(char *str) {
    for (int i = 0; str[i] != 0; ++i) {
        if (str[i] == '\n') {
            str[i] = '\0';
            return;
        }
    }
}

typedef enum {
    ISREALEXEC = 1,
    NOTREALEXEC = 0,
    ERROR = -1
} file_process_result;

int is_ELF_file(const char* buffer, int size) {
    if(size < 4) {
        return 0;
    }
    return buffer[0] == 0x7f && buffer[1] == 'E' && buffer[2] == 'L' && buffer[3] == 'F';
}

void make_zero_terminated_with_size(char * buffer, int size) {
    for(int i = 2; i < size; ++i) {
        if(buffer[i] == '\n' || buffer[i] == ' ') {
            buffer[i] = '\0';
            return;
        }
    }
    buffer[size] = '\0';
}

int find_index_of_not_symbol(char* buffer, int size) {
    for(int i = 2; i < size; ++i) {
        if(buffer[i] != ' ' && buffer[i] != '\n') {
            return i;
        }
    }
    return -1;
}

int is_interpreter_executable(char* buffer, int size) {
    if(size < 2) {
        return 0;
    } else if(buffer[0] == '#' && buffer[1] == '!') {
        int path_start = find_index_of_not_symbol(buffer, size);
        if(path_start == -1) {
            return 0;
        }
        make_zero_terminated_with_size(buffer + path_start, size - path_start);
        if(access(buffer + path_start, F_OK | X_OK) != -1) {
            return 1;
        }
    }
    return 0;
}

file_process_result check_for_real_executable_file(const char *file_name, char *buffer) {
    int file_fd = open(file_name, O_RDONLY);
    if (file_fd < 0) {
        return ERROR;
    }
    file_process_result file_type = NOTREALEXEC;
    int read_symbols_count = read(file_fd, buffer, MAX_SIZE);
    if (read_symbols_count < 0) {
        file_type = ERROR;
    } else if (read_symbols_count > 0) {
        if(is_ELF_file(buffer, read_symbols_count)) {
            file_type = ISREALEXEC;
        } else if(is_interpreter_executable(buffer, read_symbols_count)){
            file_type = ISREALEXEC;
        } else {
            file_type = NOTREALEXEC;
        }
    }

    close(file_fd);
    return file_type;
}

int main() {
    char *file_name = calloc(PATH_MAX + 1, sizeof(char));
    char *buffer = calloc(MAX_SIZE, sizeof(char));
    struct stat file_attributes;
    while (fgets(file_name, PATH_MAX + 1, stdin) != 0) {
        make_zero_terminated(file_name);
        if (access(file_name, X_OK) == 0) {
            if (check_for_real_executable_file(file_name, buffer) == NOTREALEXEC) {
                printf("%s\n", file_name);
            }
        }
    }
    free(buffer);
    free(file_name);
    return 0;
}
