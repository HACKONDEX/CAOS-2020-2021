#include <stdio.h>
#include <string.h>

struct Normalizer {
  char *path;
  int was_slash;
  int was_double_point;
  int was_point;
  int was_symbol;
  int remove_symbols_count;
  int i; // index
};

void initialize_normalizer(struct Normalizer *prev, char *path_) {
    prev->path = path_;
    prev->was_double_point = 0;
    prev->was_point = 0;
    prev->was_slash = 0;
    prev->was_symbol = 1;
    prev->remove_symbols_count = 0;
    prev->i = 0;
}

/// Shifts path to left and returns new current index
int remove_symbols_and_shift(char *path, int start, int symbols_count) {
    if (symbols_count <= 0) {
        return start;
    }
    int i = start + 1;
    int j = start - symbols_count + 1;
    while (path[i - 1] != '\0') {
        path[j] = path[i];
        ++j;
        ++i;
    }
    return start - symbols_count;
}

int process_double_point_slash(char *path, int slash_index) {
    if (slash_index >= 5) {
        /// if previous block is "../"
        if (path[slash_index - 5] == '.' && path[slash_index - 4] == '.' && path[slash_index - 3] == '/') {
            return slash_index;
        }
    }
    int start_pos = 0;
    for (int i = slash_index - 4; i >= 0; --i) {
        if (path[i] == '/') {
            start_pos = i + 1;
            break;
        }
    }
    remove_symbols_and_shift(path, slash_index, slash_index + 1 - start_pos);
    return start_pos - 1;
}

void process_symbol(struct Normalizer *prev) {
    int try_remove = 0;
    int shift = 0;
    if (prev->was_slash) {
        prev->was_slash = 0;
        try_remove = 1;
        shift = 1;
    } else if (prev->was_point) {
        prev->was_point = 0;
        try_remove = 1;
        shift = 2;
    } else if (prev->was_double_point) {
        prev->was_double_point = 0;
        try_remove = 1;
        shift = 3;
    }
    if (try_remove) {
        try_remove = 0;
        prev->i = remove_symbols_and_shift(prev->path, prev->i - shift, prev->remove_symbols_count) + shift;
        prev->remove_symbols_count = 0;
        if (prev->path[prev->i] == '/') {
            prev->was_slash = 1;
        } else {
            prev->was_symbol = 1;
        }
    }
}

void after_process_check(struct Normalizer *prev) {
    if (prev->path[prev->i] == '/') {
        prev->was_slash = 1;
    } else if (prev->path[prev->i] == '.') {
        prev->was_point = 1;
    } else {
        prev->was_symbol = 1;
    }
}

void process_slash(struct Normalizer *prev) {
    if (prev->was_symbol) {
        prev->was_symbol = 0;
        prev->was_slash = 1;
    } else if (prev->was_double_point) {
        prev->was_double_point = 0;
        if (prev->remove_symbols_count > 0) {
            prev->i = remove_symbols_and_shift(prev->path, prev->i - 3, prev->remove_symbols_count);
            prev->remove_symbols_count = 0;
            after_process_check(prev);
            return;
        }
        prev->i = process_double_point_slash(prev->path, prev->i);
        after_process_check(prev);
    } else if (prev->was_point) {
        prev->was_slash = 1;
        prev->remove_symbols_count += 2;
        prev->was_point = 0;
    } else if (prev->was_slash) {
        prev->remove_symbols_count += 1;
    }
}

void process_point(struct Normalizer *prev) {
    if (prev->was_point) {
        prev->was_point = 0;
        prev->was_double_point = 1;
    } else if (prev->was_double_point) {
        prev->was_double_point = 0;
        prev->remove_symbols_count = 0;
        prev->was_point = 0;
        prev->was_symbol = 1;
        if (prev->remove_symbols_count > 0) {
            prev->i = remove_symbols_and_shift(prev->path, prev->i - 3, prev->remove_symbols_count);
            prev->remove_symbols_count = 0;
            if (prev->path[prev->i] == '/') {
                prev->was_slash = 1;
                prev->was_symbol = 0;
            }
        }
    } else if (prev->was_slash) {
        prev->was_point = 1;
        prev->was_slash = 0;
    }
}

extern void normalize_path(char *path) {
    struct Normalizer prev;
    initialize_normalizer(&prev, path);
    for (; path[prev.i] != '\0'; ++prev.i) {
        switch (path[prev.i]) {
            case '/':
                process_slash(&prev);
                break;
            case '.':
                process_point(&prev);
                break;
            default:
                process_symbol(&prev);
                break;
        }
    }
    remove_symbols_and_shift(path, prev.i - 1, prev.remove_symbols_count);
}
