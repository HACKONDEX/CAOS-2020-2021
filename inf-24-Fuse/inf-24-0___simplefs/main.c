#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define FUSE_USE_VERSION 30
#include <fuse.h>

////////////////////// FUSE Part ////////////////////////////////

typedef struct {
    char *option_file_name;
} my_options_t;

struct fuse_opt opt_specs[] = {
    {"--src %s", offsetof(my_options_t, option_file_name), 0},
    FUSE_OPT_END};

my_options_t my_options;

////////////////// File System /////////////////////////////

typedef struct {
    size_t size;
    size_t start_pos;
    char file_name[FILENAME_MAX];
} file_t;

struct file_system_t {
    int64_t files_count;
    file_t *files;
    char *content;
    size_t content_offset;
    size_t file_len;
};

struct file_system_t file_system;

void get_files_count(FILE *file) {
    int files_count = 0;
    fscanf(file, "%d\n", &files_count);
    file_system.files_count = files_count;
}

void get_files_and_content_offset(FILE *file) {
    file_system.files = calloc(file_system.files_count, sizeof(file_t));

    size_t content_length = 0;

    for (size_t i = 0; i < file_system.files_count; ++i) {
        long long file_size = 0;
        fscanf(file, "%s %lld\n", file_system.files[i].file_name, &file_size);
        file_system.files[i].start_pos = content_length;
        file_system.files[i].size = file_size;
        content_length += file_size;
    }
    fscanf(file, "\n");
}

void get_file_content(FILE *file) {
    long seek_offset = ftell(file);
    file_system.content_offset = seek_offset;
    fflush(stdout);
    int fd = open(my_options.option_file_name, O_RDONLY);
    lseek(fd, 0, SEEK_END);
    int64_t offset = lseek(fd, 0, SEEK_END);
    file_system.file_len = offset;
    file_system.content = mmap(NULL,
                               file_system.file_len * sizeof(char),
                               PROT_READ, MAP_PRIVATE,
                               fd, 0);
    close(fd);
}

void init_file_system() {
    FILE *sample_file = fopen(my_options.option_file_name, "r");
    get_files_count(sample_file);
    get_files_and_content_offset(sample_file);
    get_file_content(sample_file);
    fclose(sample_file);
}

void free_file_system() {
    free(file_system.files);
    munmap(file_system.content, file_system.file_len);
}

//////////////////////// Callbakcs /////////////////////////////////

int getattr_callback(const char *path,
                     struct stat *stbuf,
                     struct fuse_file_info *fi) {
    (void) fi;
    if (strcmp(path, "/") == 0) {
        *stbuf = (struct stat){.st_nlink = 2, .st_mode = S_IFDIR | 0755};
        return 0;
    }
    if (path[0] == '/') {
        for (size_t i = 0; i < file_system.files_count; ++i) {
            if (strcmp(path + 1, file_system.files[i].file_name) == 0) {
                *stbuf = (struct stat){.st_nlink = 1, .st_mode = S_IFREG | 0444, .st_size = (__off_t) file_system.files[i].size};
                return 0;
            }
        }
    }
    return -ENOENT;
}

int readdir_callback(
    const char *path, void *buf,
    fuse_fill_dir_t filler,
    off_t offset,
    struct fuse_file_info *fi,
    enum fuse_readdir_flags flags) {
    if (strcmp(path, "/") != 0) {
        return -ENOENT;
    }
    filler(buf, ".", NULL, 0, (enum fuse_fill_dir_flags) 0);
    filler(buf, "..", NULL, 0, (enum fuse_fill_dir_flags) 0);
    for (size_t i = 0; i < file_system.files_count; ++i) {
        filler(buf, file_system.files[i].file_name, NULL, 0, (enum fuse_fill_dir_flags) 0);
    }
    return 0;
}

int read_callback(const char *path,
                  char *buf,
                  size_t size,
                  off_t offset,
                  struct fuse_file_info *fi) {
    if (strcmp(path, "/") == 0) {
        return -EISDIR;
    }
    if (path[0] == '/') {
        for (size_t i = 0; i < file_system.files_count; ++i) {
            if (strcmp(path + 1, file_system.files[i].file_name) == 0) {
                int bytes_count = size < file_system.files[i].size - offset ? size : file_system.files[i].size;
                memcpy(buf, file_system.content + file_system.content_offset + file_system.files[i].start_pos + offset,
                       bytes_count);
                return bytes_count;
            }
        }
    }
    return -EIO;
}

//////////////// Fuse Part ///////////////////////

struct fuse_operations fuse_example_operations = {
    .getattr = getattr_callback,
    .read = read_callback,
    .readdir = readdir_callback,
};

int main(int argc, char *argv[]) {
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    memset(&my_options, 0, sizeof(my_options));
    fuse_opt_parse(&args, &my_options, opt_specs, NULL);

    init_file_system();

    int ret = fuse_main(args.argc, args.argv, &fuse_example_operations, NULL);

    free_file_system();

    fuse_opt_free_args(&args);
    return ret;
}

