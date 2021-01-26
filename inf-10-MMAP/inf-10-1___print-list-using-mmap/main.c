#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdio.h>

struct Item {
  int value;
  uint32_t next_pointer;
};

void print_list_elements(struct Item *list, int64_t items_count) {
    if (items_count == 0) {
        return;
    }
    int end_of_list = 0;
    int64_t current_index = 0;
    while (end_of_list == 0) {
        printf("%i\n", list[current_index].value);
        current_index = list[current_index].next_pointer / sizeof(struct Item);
        if (current_index == 0) {
            end_of_list = 1;
        }
    }
}

int main(int argc, char *argv[]) {
    int file_fd = open(argv[1], O_RDONLY);

    struct stat file_attributes;
    int success = fstat(file_fd, &file_attributes);

    void *mapped = mmap(NULL, file_attributes.st_size, PROT_READ, MAP_PRIVATE, file_fd, 0);
    struct Item *list = (struct Item *) (mapped);

    print_list_elements(list, file_attributes.st_size / sizeof(struct Item));

    munmap(mapped, file_attributes.st_size);
    close(file_fd);
    return 0;
}
