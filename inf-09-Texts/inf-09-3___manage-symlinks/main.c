#include <stdio.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

const int MAX_EXTENSION_SIZE = 15;

void make_zero_terminated(char *str) {
    for (int i = 0; str[i] != 0; ++i) {
        if (str[i] == '\n') {
            str[i] = '\0';
            return;
        }
    }
}

void add_prefix(const char *file_name, char *symlink_name) {
    symlink_name[0] = '\0';
    strcat(symlink_name, "link_to_\0");
    strcat(symlink_name, file_name);
}

void make_file_name_basename(char *name) {
    int last_slash_index = 0;
    int is_there_any_slash = 0;
    for (int i = 0; name[i] != '\0'; ++i) {
        if (name[i] == '/') {
            last_slash_index = i;
            is_there_any_slash = 1;
        }
    }
    if (is_there_any_slash == 0) {
        return;
    }
    int i = last_slash_index + 1;
    int j = 0;
    while (name[i - 1] != '\0') {
        name[j] = name[i];
        ++j;
        ++i;
    }
}

int main() {
    char *file_name = calloc(PATH_MAX + 1, sizeof(char));
    char *absolute_path = calloc(PATH_MAX + 1, sizeof(char));
    char *symlink_name = calloc(PATH_MAX + MAX_EXTENSION_SIZE, sizeof(char));
    struct stat file_attributes;
    while (fgets(file_name, PATH_MAX + 1, stdin) != 0) {
        make_zero_terminated(file_name);
        if (lstat(file_name, &file_attributes) == 0) {
            if (S_ISLNK(file_attributes.st_mode)) {
                char *ptr = realpath(file_name, absolute_path);
                printf("%s\n", absolute_path);
            } else if (S_ISREG(file_attributes.st_mode)) {
                make_file_name_basename(file_name);
                add_prefix(file_name, symlink_name);
                symlink(file_name, symlink_name);
            }
        }
    }

    free(symlink_name);
    free(absolute_path);
    free(file_name);
    return 0;
}

