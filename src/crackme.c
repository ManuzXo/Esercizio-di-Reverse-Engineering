#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const char charset[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789"
    "!@#$%^&*()-_=+";

char *GenerateKey(size_t length) {
    size_t charsetSize = strlen(charset);
    char *key = malloc(length + 1);
    if (!key) return NULL;

    for (size_t i = 0; i < length; i++) {
        key[i] = charset[rand() % charsetSize];
    }
    key[length] = '\0';
    return key;
}

int main(void) {

    srand((unsigned)time(NULL));
    size_t keyLen = 16;
    char *key = GenerateKey(keyLen);

    if (!key) {
        printf("Errore allocazione memoria\n");
        return -1;
    }
    char buf[64];
    printf("Insert the Key: ");
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        printf("Error reading input.\n");
        free(key);
        return -1;
    }

    buf[strcspn(buf, "\n")] = '\0';

    if (strcmp(buf, key) == 0) {
        printf("Key is Correct\n");
        free(key);
        return 0;
    }

    printf("Error: the key is not correct!\n");
    free(key);
    return -1;
}
