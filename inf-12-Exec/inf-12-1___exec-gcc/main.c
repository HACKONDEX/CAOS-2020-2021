#include <fcntl.h>
#include <inttypes.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

const int32_t MaxSize = 5000;
const char code_file_name[11] = "tmp_code.c";
const char executable_file_name[15] = "./tmp_c.exe";

struct Expression {
    int32_t buffer_size;
    int32_t max_buffer_size;
    char* buffer;
};

void initialize_expression(struct Expression* expr)
{
    expr->buffer_size = 0;
    expr->max_buffer_size = MaxSize;
    expr->buffer = calloc(MaxSize, sizeof(char));
}

void delete_expression(struct Expression* expr)
{
    free(expr->buffer);
}

void read_expression(struct Expression* expr)
{
    char* buffer_start_pt = expr->buffer;
    char* buffer = buffer_start_pt;
    while (1) {
        if (expr->max_buffer_size - expr->buffer_size < MaxSize) {
            expr->max_buffer_size *= 2;
            buffer_start_pt =
                realloc(buffer_start_pt, expr->max_buffer_size * sizeof(char));
            buffer = buffer_start_pt + expr->buffer_size;
        }
        int read_symbols_count = read(0, buffer, MaxSize - 1);
        if (read_symbols_count <= 0) {
            break;
        }
        expr->buffer_size += read_symbols_count;
        buffer += read_symbols_count;
    }
    buffer_start_pt[expr->buffer_size] = '\0';
    expr->buffer = buffer_start_pt;
}

void create_file_with_code(struct Expression* expr)
{
    char format[200] = "#include <math.h>\n"
                       "#include <stdio.h>\n"
                       "#include <stdlib.h>\n"
                       "int main() {\n"
                       "    printf(\"%%lld\", (long long)(%s));\n"
                       "    return 0;\n"
                       "}";
    FILE* c_code_file = fopen(code_file_name, "w+");
    fprintf(c_code_file, format, expr->buffer);
    fclose(c_code_file);
}

void create_executable_file()
{
    pid_t pid = fork();
    if (pid != 0) {
        int status = 0;
        waitpid(pid, &status, 0);
        return;
    }
    execlp("gcc", "gcc", "-o", executable_file_name + 2, code_file_name, NULL);
}

void run_exec_file()
{
    pid_t pid = fork();
    if (pid != 0) {
        int status = 0;
        waitpid(pid, &status, 0);
        return;
    }
    execlp(executable_file_name, executable_file_name, NULL);
}

int main()
{
    struct Expression expr;
    initialize_expression(&expr);

    read_expression(&expr);
    if (expr.buffer_size == 0) {
        delete_expression(&expr);
        return 0;
    }
    create_file_with_code(&expr);
    delete_expression(&expr);
    create_executable_file();
    run_exec_file();

    return 0;
}
