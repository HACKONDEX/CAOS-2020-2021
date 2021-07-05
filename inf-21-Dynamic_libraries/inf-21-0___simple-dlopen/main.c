#include <dlfcn.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    char *lib_name = argv[1];

    void *lib_handle = dlopen(lib_name, RTLD_NOW);
    if (!lib_handle) {
        fprintf(stderr, "dlopen: %s\n", dlerror());
        return 1;
    }

    double (*func)(double) = dlsym(lib_handle, argv[2]);

    float number = 0;
    while (scanf("%f", &number) > 0) {
        printf("%.3f\n", func(number));
    }
    dlclose(lib_handle);
    return 0;
}

