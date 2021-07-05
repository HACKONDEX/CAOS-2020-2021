#include <assert.h>
#include <openssl/evp.h>
#include <string.h>
#include <unistd.h>

#define MAXSIZE 4096
#define SALTSIZE 8
#define SHA256KEYSIZE 32
#define SHA256IVSIZE 16

struct data_t {
    unsigned char *buffer;
    size_t buffer_size;
    size_t size;
};

void data_t_init(struct data_t *data) {
    data->size = 0;
    data->buffer_size = MAXSIZE;
    data->buffer = calloc(sizeof(unsigned char), data->buffer_size);
}

void data_t_destroy(struct data_t *data) {
    free(data->buffer);
}

void data_t_resize(struct data_t *data) {
    data->buffer_size *= 2;
    data->buffer = realloc(data->buffer, data->buffer_size);
}

void get_encrypted_data(struct data_t *data) {
    size_t bytes_count = 0;
    while ((bytes_count = read(0, data->buffer + data->size, MAXSIZE)) > 0) {
        data->size += bytes_count;
        if (data->buffer_size - data->size < MAXSIZE) {
            data_t_resize(data);
        }
        if (bytes_count < MAXSIZE) {
            break;
        }
    }
}

void get_salt(unsigned char *salt, struct data_t *data) {
    if (data->size < 16) {
        assert(0);
    }

    unsigned char *buffer = data->buffer;
    const size_t salt_start_pos = 8;
    const size_t salt_end_pos = 16;
    for (size_t i = salt_start_pos; i < salt_end_pos; ++i) {
        salt[i - salt_start_pos] = buffer[i];
    }
}

void decode(unsigned char *password, struct data_t *data) {
    unsigned char salt[SALTSIZE];
    get_salt(salt, data);

    unsigned char key[SHA256KEYSIZE];
    unsigned char iv[SHA256IVSIZE];
    EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha256(), salt,
                   password, strlen(password), 1, key, iv);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit(ctx, EVP_aes_256_cbc(), key, iv);

    unsigned char decoded_data[2 * EVP_CIPHER_CTX_block_size(ctx)];
    unsigned char *encrypted_data = data->buffer + 16;
    int max_size = data->size;
    int block_size = sizeof(decoded_data) - EVP_CIPHER_CTX_block_size(ctx);
    int cur_size = 2 * SALTSIZE;
    int length = 0;
    while (max_size - cur_size >= block_size) {
        EVP_DecryptUpdate(ctx, decoded_data, &length, encrypted_data, block_size);
        encrypted_data += block_size;
        cur_size += block_size;
        write(1, decoded_data, length);
    }
    if (max_size - cur_size != 0) {
        EVP_DecryptUpdate(ctx, decoded_data, &length, encrypted_data, max_size - cur_size);
        write(1, decoded_data, length);
    }

    EVP_DecryptFinal(ctx, decoded_data, &length);
    write(1, decoded_data, length);

    EVP_CIPHER_CTX_free(ctx);
}

int main(int argc, char *argv[]) {
    unsigned char *password = argv[1];

    struct data_t encrypted_data;
    data_t_init(&encrypted_data);
    get_encrypted_data(&encrypted_data);

    decode(password, &encrypted_data);

    data_t_destroy(&encrypted_data);
    return 0;
}

