#include <stdio.h>
#include <fcntl.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>

#define _FILE_OFFSET_BITS 64

const int64_t MaxSizeForQuickSort = 100000;
const int64_t MaxReadSize = 100000;
const int64_t CopyReadSize = 100000;

int64_t min(int64_t a, int64_t b) {
    return a <= b ? a : b;
}

int compare_integers(const void *a, const void *b) {
    return (*(int *) a - *(int *) b);
}

void copy_remaining_elements(int tmp_file_fd,
                             int file_fd,
                             int64_t current_index,
                             int64_t last_index,
                             int *array,
                             int64_t array_index,
                             int64_t array_size) {
    write(tmp_file_fd, array + array_index, (array_size - array_index) * sizeof(int));
    current_index += array_size - array_index;
    lseek(file_fd, current_index * 4, SEEK_SET);
    int64_t left_elements_count = last_index - current_index;
    int64_t read_elements_count = 0, current_read_count = 0;
    while (read_elements_count < left_elements_count) {
        current_read_count = read(file_fd, array, array_size * sizeof(int)) / 4;
        read_elements_count += current_read_count;
        write(tmp_file_fd, array, current_read_count * sizeof(int));
    }
}

void copy_sorted_elements_from_tmp_file(int file_fd, int tmp_file_fd, int64_t start, int64_t elements_count) {
    int64_t left_elements_count = elements_count;
    int *buffer = (int *) malloc(min(CopyReadSize, elements_count) * sizeof(int));
    lseek(file_fd, start * 4, SEEK_SET);
    lseek(tmp_file_fd, 0, SEEK_SET);
    int64_t read_bytes_count;
    while (left_elements_count > 0) {
        read_bytes_count = read(tmp_file_fd, buffer, min(CopyReadSize, left_elements_count) * sizeof(int));
        left_elements_count -= read_bytes_count / 4;
        write(file_fd, buffer, read_bytes_count);
    }
    free(buffer);
}

//// writes merged parts into tmp_file and copies from into our file
void merge(int file_fd, int64_t left, int64_t median, int64_t right) {
    int64_t left_index = left, right_index = median;
    int64_t left_array_size = 0, right_array_size = 0;
    int64_t left_array_index = 0, right_array_index = 0;
    int64_t merged_buffer_index = 0, merged_buffer_size = 2 * MaxReadSize;
    int64_t tmp_file_elements_count = 0;

    int tmp_file_fd = open("_tmp_file_", O_RDWR | O_CREAT, 0664);

    //// Load from file into buffers, merge into merged_buffer,write merged_buffer into tmp_file
    int *left_array = (int *) malloc(MaxReadSize * sizeof(int));
    int *right_array = (int *) malloc(MaxReadSize * sizeof(int));
    int *merged_buffer = (int *) malloc(2 * MaxReadSize * sizeof(int));

    lseek(file_fd, left_index * 4, SEEK_SET);
    left_array_size = read(file_fd, left_array, min(MaxReadSize, median - left_index) * sizeof(int)) / 4;
    lseek(file_fd, right_index * 4, SEEK_SET);
    right_array_size = read(file_fd, right_array, min(MaxReadSize, right - right_index) * sizeof(int)) / 4;

    //// while one of the parts is not over we will merge
    while (left_index < median && right_index < right) {
        //// if array for left part fully merged
        if (left_array_index >= left_array_size) {
            lseek(file_fd, left_index * 4, SEEK_SET);
            left_array_size = read(file_fd, left_array, min(MaxReadSize, median - left_index) * sizeof(int)) / 4;
            left_array_index = 0;
        }
        //// if array for right part fully merged
        if (right_array_index >= right_array_size) {
            lseek(file_fd, right_index * 4, SEEK_SET);
            right_array_size =
                read(file_fd, right_array, min(MaxReadSize, right - right_index) * sizeof(int)) / 4;
            right_array_index = 0;
        }
        //// if merged buffer is ful write into tmp_file
        if (merged_buffer_index >= merged_buffer_size) {
            tmp_file_elements_count += write(tmp_file_fd, merged_buffer, merged_buffer_index * sizeof(int)) / 4;
            merged_buffer_index = 0;
        }
        //// Directly merging elements
        if (left_array[left_array_index] < right_array[right_array_index]) {
            merged_buffer[merged_buffer_index] = left_array[left_array_index];
            ++left_index;
            ++left_array_index;
            ++merged_buffer_index;
        } else {
            merged_buffer[merged_buffer_index] = right_array[right_array_index];
            ++right_index;
            ++right_array_index;
            ++merged_buffer_index;
        }
    }

    //// write into tmp_file elements left in merged_buffer
    write(tmp_file_fd, merged_buffer, merged_buffer_index * sizeof(int));

    //// if left part is not over
    if (left_index < median) {
        copy_remaining_elements(tmp_file_fd,
                                file_fd,
                                left_index,
                                median,
                                left_array,
                                left_array_index,
                                left_array_size);
    }
    //// if right part is not over
    if (right_index < right) {
        copy_remaining_elements(tmp_file_fd,
                                file_fd,
                                right_index,
                                right,
                                right_array,
                                right_array_index,
                                right_array_size);
    }

    //// copy from tmp_file into file
    copy_sorted_elements_from_tmp_file(file_fd, tmp_file_fd, left, right - left);

    free(merged_buffer);
    free(right_array);
    free(left_array);
    close(tmp_file_fd);
}

void merge_sort(int file_fd, int64_t left, int64_t right) {
    if (right - left <= MaxSizeForQuickSort) {
        /// QuickSort
        int64_t elements_count = right - left;
        int *buffer = (int *) malloc(elements_count * sizeof(int));
        lseek(file_fd, (left * 4), SEEK_SET);
        read(file_fd, buffer, elements_count * sizeof(int));
        qsort(buffer, elements_count, sizeof(int), compare_integers);
        lseek(file_fd, left * 4, SEEK_SET);
        write(file_fd, buffer, elements_count * 4);
        free(buffer);
        return;
    }
    //// if part is very big do mergesort
    int64_t median = (right + left) / 2;
    merge_sort(file_fd, left, median);
    merge_sort(file_fd, median, right);
    merge(file_fd, left, median, right);
}

int main(int argc, char *argv[]) {
    int file_fd = open(argv[1], O_RDWR);
    int64_t bytes_count = lseek(file_fd, 0, SEEK_END);
    lseek(file_fd, 0, SEEK_SET);
    merge_sort(file_fd, 0, (bytes_count / 4));
    close(file_fd);
    return 0;
}
