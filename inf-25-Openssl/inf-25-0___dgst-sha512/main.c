#include <assert.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <unistd.h>

#define BLOCKSIZE 64

int main() {
    SHA512_CTX context;
    if (SHA512_Init(&context) != 1) {
        assert(0);
    }

    char buffer[BLOCKSIZE];
    size_t bytes_count = 0;
    while ((bytes_count = read(0, buffer, BLOCKSIZE)) > 0) {
        if (SHA512_Update(&context, buffer, bytes_count) != 1) {
            assert(0);
        }
        if (bytes_count < BLOCKSIZE) {
            break;
        }
    }

    unsigned char checksum[SHA512_DIGEST_LENGTH];
    SHA512_Final(checksum, &context);

    printf("0x");
    for (size_t i = 0; i < SHA512_DIGEST_LENGTH; i++) {
        printf("%.2x", checksum[i]);
    }
    return 0;
}

