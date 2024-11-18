#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define AES_BLOCK_SIZE 16
#define AES_KEY_SIZE 32

// Key Derivation: A simple key derivation function
void derive_aes_key(const char *passphrase, unsigned char *aes_key) {
    size_t pass_len = strlen(passphrase);
    for (size_t i = 0; i < AES_KEY_SIZE; i++) {
        aes_key[i] = (i < pass_len) ? passphrase[i] : i;
    }
}

// XOR-based "AES-like" encryption (for demonstration)
void aes_encrypt(const unsigned char *plaintext, int plaintext_len, const unsigned char *key, unsigned char *ciphertext, unsigned char *iv) {
    for (int i = 0; i < plaintext_len; i++) {
        ciphertext[i] = plaintext[i] ^ key[i % AES_KEY_SIZE] ^ iv[i % AES_BLOCK_SIZE];
    }
}

// XOR-based "AES-like" decryption (same as encryption)
void aes_decrypt(const unsigned char *ciphertext, int ciphertext_len, const unsigned char *key, unsigned char *plaintext, const unsigned char *iv) {
    aes_encrypt(ciphertext, ciphertext_len, key, plaintext, iv); // Reuse encryption logic
}

// Base64 Encoding
char *base64_encode(const unsigned char *input, int length) {
    const char *base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char *output = malloc(((length + 2) / 3) * 4 + 1);
    int i, j, output_index = 0;

    for (i = 0; i < length; i += 3) {
        unsigned int value = input[i] << 16 | (i + 1 < length ? input[i + 1] << 8 : 0) | (i + 2 < length ? input[i + 2] : 0);
        for (j = 0; j < 4; j++) {
            if (i * 8 / 6 + j < length * 8 / 6) {
                output[output_index++] = base64_chars[(value >> (18 - j * 6)) & 0x3F];
            } else {
                output[output_index++] = '=';
            }
        }
    }
    output[output_index] = '\0';
    return output;
}

// Vigenère Cipher Functions
void vigenere_encrypt(const char *plaintext, const char *key, char *ciphertext) {
    int key_length = strlen(key);
    for (int i = 0, j = 0; plaintext[i] != '\0'; i++) {
        char base = isupper(plaintext[i]) ? 'A' : (islower(plaintext[i]) ? 'a' : 0);
        if (base) {
            ciphertext[i] = ((plaintext[i] - base + (toupper(key[j % key_length]) - 'A')) % 26) + base;
            j++;
        } else {
            ciphertext[i] = plaintext[i];
        }
    }
    ciphertext[strlen(plaintext)] = '\0';
}

void vigenere_decrypt(const char *ciphertext, const char *key, char *plaintext) {
    int key_length = strlen(key);
    for (int i = 0, j = 0; ciphertext[i] != '\0'; i++) {
        char base = isupper(ciphertext[i]) ? 'A' : (islower(ciphertext[i]) ? 'a' : 0);
        if (base) {
            plaintext[i] = ((ciphertext[i] - base - (toupper(key[j % key_length]) - 'A') + 26) % 26) + base;
            j++;
        } else {
            plaintext[i] = ciphertext[i];
        }
    }
    plaintext[strlen(ciphertext)] = '\0';
}

// Main Program
int main() {
    char plaintext[1024], vigenere_key[256], vigenere_cipher[1024];
    unsigned char aes_key[AES_KEY_SIZE], aes_cipher[2048], aes_decrypted[2048], iv[AES_BLOCK_SIZE] = {0};
    char passphrase[256];
    clock_t start, end;

    // Input Plaintext and Keys
    printf("Enter plaintext: ");
    fgets(plaintext, sizeof(plaintext), stdin);
    plaintext[strcspn(plaintext, "\n")] = '\0';

    printf("Enter Vigenère key: ");
    fgets(vigenere_key, sizeof(vigenere_key), stdin);
    vigenere_key[strcspn(vigenere_key, "\n")] = '\0';

    printf("Enter passphrase for AES: ");
    fgets(passphrase, sizeof(passphrase), stdin);
    passphrase[strcspn(passphrase, "\n")] = '\0';

    // Derive AES Key
    derive_aes_key(passphrase, aes_key);

    // Vigenère Encryption
    start = clock();
    vigenere_encrypt(plaintext, vigenere_key, vigenere_cipher);
    end = clock();
    printf("Vigenère Ciphertext: %s\n", vigenere_cipher);
    printf("Vigenère Encryption Time: %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);

    // AES Encryption
    start = clock();
    int plaintext_len = strlen(vigenere_cipher);
    aes_encrypt((unsigned char *)vigenere_cipher, plaintext_len, aes_key, aes_cipher, iv);
    end = clock();

    // Convert AES Ciphertext to Base64
    char *base64_cipher = base64_encode(aes_cipher, plaintext_len);
    printf("AES Ciphertext (Base64): %s\n", base64_cipher);
    printf("AES Encryption Time: %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);

    // AES Decryption
    start = clock();
    aes_decrypt(aes_cipher, plaintext_len, aes_key, aes_decrypted, iv);
    end = clock();
    aes_decrypted[plaintext_len] = '\0'; // Null-terminate the decrypted text
    printf("Decrypted Vigenère Ciphertext: %s\n", aes_decrypted);
    printf("AES Decryption Time: %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);

    // Vigenère Decryption
    char final_plaintext[1024];
    start = clock();
    vigenere_decrypt((char *)aes_decrypted, vigenere_key, final_plaintext);
    end = clock();
    printf("Decrypted Plaintext: %s\n", final_plaintext);
    printf("Vigenère Decryption Time: %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);

    // Free Memory
    free(base64_cipher);
    return 0;
}


