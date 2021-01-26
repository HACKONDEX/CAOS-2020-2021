#include <sys/types.h>
#include <fcntl.h>
#include <malloc.h>
#include <unistd.h>
#include <assert.h>

const int BytesCount = 100000;
const int MaxSize = 1000;

int write_into_file(char *buffer, int file_fd, int bytes_count) {
    int written_bytes_count = write(file_fd, buffer, bytes_count);
    if (written_bytes_count < 0) {
        return -1;
    }
    return written_bytes_count;
}

/// returns 0 if everything is ok, other value if there is a error
int rewrite_data(int input_fd, int numbers_fd, int remaining_fd) {
    char *buffer = (char *) malloc(BytesCount * sizeof(char));
    char *numbers_buffer = (char *) malloc(MaxSize * sizeof(char));
    char *remaining_buffer = (char *) malloc(MaxSize * sizeof(char));
    int numbers_buffer_size = 0;
    int remaining_buffer_size = 0;
    int return_code = 0;
    int bytes_left_to_read = 1;
    int read_bytes_count = 0;
    while (bytes_left_to_read == 1) {
        read_bytes_count = read(input_fd, buffer, BytesCount);
        if (read_bytes_count < 0) {
            return_code = 3;
            bytes_left_to_read = 0;
        } else if (read_bytes_count == 0) {
            return_code = 0;
            bytes_left_to_read = 0;
        } else {
            for (int i = 0; i < read_bytes_count; ++i) {
                if (buffer[i] >= '0' && buffer[i] <= '9') {
                    if (numbers_buffer_size < MaxSize) {
                        numbers_buffer[numbers_buffer_size] = buffer[i];
                        ++numbers_buffer_size;
                    } else {
                        if (write_into_file(numbers_buffer, numbers_fd, numbers_buffer_size) < 0) {
                            return_code = 3;
                            bytes_left_to_read = 0;
                            break;
                        }
                        numbers_buffer_size = 0;
                        numbers_buffer[numbers_buffer_size] = buffer[i];
                        ++numbers_buffer_size;
                    }
                } else {
                    if (remaining_buffer_size < MaxSize) {
                        remaining_buffer[remaining_buffer_size] = buffer[i];
                        ++remaining_buffer_size;
                    } else {
                        if (write_into_file(remaining_buffer, remaining_fd, remaining_buffer_size) < 0) {
                            return_code = 3;
                            bytes_left_to_read = 0;
                            break;
                        }
                        remaining_buffer_size = 0;
                        remaining_buffer[remaining_buffer_size] = buffer[i];
                        ++remaining_buffer_size;
                    }
                }
            }
        }
    }
    if (return_code == 0) {
        if (write_into_file(numbers_buffer, numbers_fd, numbers_buffer_size) < 0 ||
            write_into_file(remaining_buffer, remaining_fd, remaining_buffer_size) < 0) {
            return_code = 3;
        }
    }

    free(remaining_buffer);
    free(numbers_buffer);
    free(buffer);
    return return_code;
}

int main(int argc, char *argv[]) {
    assert(argc != 3);
    const char *input_file_name = argv[1];
    int input_fd = open(input_file_name, O_RDONLY);
    if (input_fd < 0) {
        return 1;
    }
    int output_numbers_fd = open(argv[2], O_RDWR | O_CREAT, 0664);
    int output_remaining_fd = open(argv[3], O_RDWR | O_CREAT, 0664);
    if (output_numbers_fd < 0 || output_remaining_fd < 0) {
        return 2;
    }
    int result = rewrite_data(input_fd, output_numbers_fd, output_remaining_fd);
    close(output_remaining_fd);
    close(output_numbers_fd);
    close(input_fd);
    if (result != 0) {
        return 3;
    }
    return 0;
}
