#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    double number_x;
    scanf("%lf", &number_x);
    long long number_y;
    scanf("%llx", &number_y);
    printf("%0.3lf", number_x + (double)(number_y)
                     + (double)(strtol(argv[1], NULL, 27)));
    return 0;
}
