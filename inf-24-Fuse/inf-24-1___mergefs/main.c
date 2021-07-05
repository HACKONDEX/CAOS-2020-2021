#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <set>
#include <string.h>
#include <string>
#include <vector>

#define FUSE_USE_VERSION 30
#include <fuse.h>

typedef struct {
    char *options;
} my_options_t;

struct fuse_opt opt_specs[] = {
    {"--src %s", offsetof(my_options_t, options), 0},
    FUSE_OPT_END};

my_options_t my_options;

//////////////////// File System ///////////////////////

struct Katalog {
    explicit Katalog(std::string &&path_str) : path_(std::move(path_str)) {
        path_ = std::move(std::filesystem::absolute(path_));
        path_ = std::move(path_.parent_path());
    }

    std::filesystem::path path_;
};

struct FileSystem {

    std::vector<Katalog> katalogs;
    size_t count;
};

FileSystem file_system;

void init_file_system() {
    std::string options(my_options.options);
    std::string delimiter = ":";
    size_t start = 0;
    size_t end = 0;

    while ((end = options.find(delimiter, start)) < options.length()) {
        file_system.katalogs.emplace_back(std::move(options.substr(start, end - start)));
        start = end + 1;
    }
    file_system.katalogs.emplace_back(std::move(options.substr(start, options.length())));
    file_system.count = file_system.katalogs.size();
}

//////////////////// Callbacks /////////////////////////

size_t get_links_count(const std::filesystem::path &path) {
    size_t count = 0;
    for (auto &sub_file : std::filesystem::directory_iterator(path)) {
        if (is_directory(sub_file.path())) {
            ++count;
        }
    }
    return count;
}

int getattr_callback(const char *path,
                     struct stat *stbuf,
                     struct fuse_file_info *fi) {
    std::string path_str = path + 1;
    if (path[0] != '/') {
        return -ENOENT;
    }

    for (size_t i = 0; i < file_system.count; ++i) {
        std::filesystem::path cur_path = file_system.katalogs[i].path_ / path_str;
        if (exists(cur_path)) {
            if (is_directory(cur_path)) {
                size_t link_count = get_links_count(cur_path);
                link_count += 2;
                *stbuf = (struct stat){.st_nlink = link_count, .st_mode = S_IFDIR | 0555};
                return 0;
            } else if (is_regular_file(cur_path)) {
                *stbuf = (struct stat){.st_nlink = 1,
                                       .st_mode = S_IFREG | 0444,
                                       .st_size = (__off_t) file_size(cur_path)};
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

    std::string path_str = path + 1;
    if (path[0] != '/') {
        return -ENOENT;
    }

    std::set<std::string> file_names;
    file_names.insert(".");
    file_names.insert("..");

    for (size_t i = 0; i < file_system.count; ++i) {
        std::filesystem::path cur_path = file_system.katalogs[i].path_ / path_str;
        if (exists(cur_path)) {
            if (is_directory(cur_path)) {
                for (auto &sub_files : std::filesystem::directory_iterator(cur_path)) {
                    file_names.insert(sub_files.path().filename());
                }
            }
        }
    }

    for (auto &str : file_names) {
        filler(buf, str.c_str(), nullptr, 0, (enum fuse_fill_dir_flags) 0);
    }
    return 0;
}

std::optional<std::filesystem::path> find_last_modificated(std::vector<std::filesystem::path> &real_path) {
    if (real_path.empty()) {
        return std::nullopt;
    }
    size_t last_modificated_index = 0;
    auto max_time = std::filesystem::last_write_time(real_path[0]);

    for (size_t i = 1; i < real_path.size(); ++i) {
        auto cur_time = std::filesystem::last_write_time(real_path[i]);
        if (cur_time > max_time) {
            last_modificated_index = i;
            max_time = cur_time;
        }
    }

    return real_path[last_modificated_index];
}

int read_callback(const char *path,
                  char *buf,
                  size_t size,
                  off_t offset,
                  struct fuse_file_info *fi) {
    std::string path_str = path + 1;
    if (path[0] != '/') {
        return -ENOENT;
    }

    std::vector<std::filesystem::path> real_paths;

    for (size_t i = 0; i < file_system.count; ++i) {
        std::filesystem::path cur_path = file_system.katalogs[i].path_ / path_str;
        if (exists(cur_path)) {
            if (is_regular_file(cur_path)) {
                real_paths.emplace_back(cur_path);
            }
        }
    }

    std::optional<std::filesystem::path> right_path = find_last_modificated(real_paths);
    if (right_path.has_value()) {
        std::filesystem::path file_path = right_path.value();
        std::ifstream stream;
        stream.open(file_path.c_str(), std::ios_base::in);
        stream.seekg(offset);
        int bytes_count = std::min((int) file_size(file_path) - (int) offset, (int) size);
        stream.read(buf, bytes_count);
        stream.close();
        return bytes_count;
    } else {
        return -ENOENT;
    }
}

struct fuse_operations fuse_example_operations = {
    .getattr = getattr_callback,
    .read = read_callback,
    .readdir = readdir_callback,
};

int main(int argc, char *argv[]) {
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    memset(&my_options, 0, sizeof(my_options));
    fuse_opt_parse(&args, &my_options, opt_specs, nullptr);

    init_file_system();
    int ret = fuse_main(args.argc, args.argv, &fuse_example_operations, nullptr);

    fuse_opt_free_args(&args);
    return ret;
}

