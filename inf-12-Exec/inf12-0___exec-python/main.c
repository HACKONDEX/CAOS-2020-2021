#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

const int MaxSize = 1024;

int main()
{
    char buffer[MaxSize];
    char* file_name = "tmptmptmp.py";
    fgets(buffer, MaxSize, stdin);
    int fd = open(file_name, O_RDWR | O_CREAT, 0777);
    write(fd, "print(", 6);
    char* end = memchr(buffer, '\n', MaxSize);
    if (end != NULL) {
        end[0] = '\0';
    }
    write(fd, buffer, strlen(buffer));
    write(fd, ")\n\n", 2);
    close(fd);
    execlp("python3", "python3", file_name, NULL);
    return 0;
}
