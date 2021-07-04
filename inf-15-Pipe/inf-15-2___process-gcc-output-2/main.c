#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

const int max_len = 8192;
const char* error_str = ": error:";
const char* warning_str = ": warning:";

char line_number[20];

volatile int child_finish = 0;

static void sigchild_handler(int signum) {
    child_finish = 1;
}

struct Pair{
    int error_count;
    int warning_count;
};

int get_line_number() {
    int len = strlen(line_number);
    int half = (len / 2) + (len % 2);
    char c = 0;
    for(int i = 0; i < half; ++i) {
        c = line_number[i];
        line_number[i] = line_number[len -1 -i];
        line_number[len -1 -i] = c;
    }
    return (int)strtoll(line_number, NULL, 10);
}

int get_main_check_part(char* str, int start_index, int* delimetr_index) {
    int wrong_symbol = 0;
    int found_delimetr = 0;
    int found_number = 0;
    int j = 0;
    for(int i = start_index - 1; i >= 0; --i) {
        if(str[i] >= '0' && str[i] <= '9') {
            found_number = 1;
            line_number[j] = str[i];
            line_number[1 + j++] = '\0';
            continue;
        } else if (str[i] == ':') {
            found_delimetr = 1;
            *delimetr_index = i;
            break;
        }
        else {
            wrong_symbol = 1;
            break;
        }
    }
    if(wrong_symbol == 1 || found_delimetr == 0 || found_number == 0) {
        return 0;
    }
    return 1;
}

int check_for_regexp(char* str, char* substr, int* previous_line) {
    if(substr == NULL) {
        return 0;
    }
    int start_index = substr - str;
    int delimetr_index = 0;
    if(get_main_check_part(str, start_index, &delimetr_index) == 0) {
        return 0;
    }
    start_index = delimetr_index;
    if(get_main_check_part(str, start_index, &delimetr_index) == 0) {
        return 0;
    }
    int num = get_line_number();
    if(num != *previous_line) {
        *previous_line = num;
        return 1;
    }
    return 0;
}

void find_errors_warnings_count(struct Pair* answer, FILE* pipe_stream,
                                int* previous_error_line_number, int* previous_warning_line_number) {
    char buffer[max_len];
    char *str = NULL;
    char* substr = NULL;
    int finish_reading = 0;

    while (finish_reading != 1) {
        str = fgets(buffer, max_len, pipe_stream);
        if (str == NULL) {
            finish_reading = 1;
        } else {
            substr = strstr(str, error_str);
            answer->error_count += check_for_regexp(str, substr, previous_error_line_number);
            substr = strstr(str, warning_str);
            answer->warning_count += check_for_regexp(str, substr, previous_warning_line_number);
        }
    }
}

int main(int argc, char *argv[]) {
    sigaction(SIGCHLD,
              &(struct sigaction){
                  .sa_handler = sigchild_handler,
                  .sa_flags = SA_RESTART},
              NULL);

    const char *programm = argv[1];
    const char *gcc = "gcc";

    int pipe_fd[2];
    pipe(pipe_fd);

    int error_occurred = 0;
    pid_t child_pid = fork();
    if (child_pid == 0) {
        dup2(pipe_fd[1], 2);
        close(pipe_fd[0]);
        execlp(gcc, gcc, programm, NULL);
        error_occurred = 1;
    }

    if(error_occurred == 1) {
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        _exit(1);
    }

    close(pipe_fd[1]);
    FILE* pipe_stream = fdopen(pipe_fd[0], "rw");

    int previous_error_line_number = 0;
    int previous_warning_line_number = 0;
    struct Pair answer;
    answer.warning_count = 0;
    answer.error_count = 0;

    while (child_finish != 1) {
        find_errors_warnings_count(&answer, pipe_stream, &previous_error_line_number, &previous_warning_line_number);
    }

    int status = 0;
    waitpid(child_pid, &status, 0);

    find_errors_warnings_count(&answer, pipe_stream, &previous_error_line_number, &previous_warning_line_number);

    close(pipe_fd[0]);

    printf("%d\n", answer.error_count);
    printf("%d\n", answer.warning_count);

    return 0;
}

