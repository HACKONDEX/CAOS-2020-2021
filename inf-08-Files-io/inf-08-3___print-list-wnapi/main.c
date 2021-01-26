#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <inttypes.h>
//#include </usr/share/mingw-w64/include/winbase.h>

int print_list_elements(HANDLE fileHandle) {
    int return_code = 0;
    int value = 0;
    uint32_t next_pointer;
    int is_end_of_list = 0;
    DWORD bytes_read;
    BOOL success;
    while (is_end_of_list != 1) {
        success = ReadFile(fileHandle, &value, 4,
                           &bytes_read, NULL);
        success = ReadFile(fileHandle, &next_pointer, 4,
                           &bytes_read, NULL);
        if (success != 1) {
            return_code = -1;
            is_end_of_list = 1;
        }
        if (bytes_read != 0) {
            printf("%d\n", value);
            if (next_pointer == 0) {
                is_end_of_list = 1;
            } else {
                LARGE_INTEGER offset;
                offset.QuadPart = next_pointer;
                success = SetFilePointerEx(fileHandle, offset, NULL, FILE_BEGIN);
            }
        } else {
            is_end_of_list = 1;
        }
    }
    return return_code;
}

int main(int argc, char *argv[]) {
    assert(argc >= 2);
    HANDLE fileHandle = CreateFileA(
        argv[1], GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (print_list_elements(fileHandle) != 0) {
        CloseHandle(fileHandle);
        return 1;
    }
    CloseHandle(fileHandle);
    return 0;
}
