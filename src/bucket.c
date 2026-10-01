#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NUM_BALDES 27 

void insertionSort(char *bucket[], int size) {
    for (int i = 1; i < size; ++i) {
        char *key = bucket[i];
        int j = i - 1;
        while (j >= 0 && strcmp(bucket[j], key) > 0) {
            bucket[j + 1] = bucket[j];
            j--;
        }
        bucket[j + 1] = key;
    }
}

int indiceBalde(const char *titulo) {
    unsigned char c = (unsigned char) titulo[0];
    if (!isalpha(c))
        return 0;                      /* número, símbolo ou acento: '#' */
    return toupper(c) - 'A' + 1;       /* A = 1, B = 2, ..., Z = 26 */
}

void bucketSort(char *arr[], int n) {
    char *baldes[NUM_BALDES][n];
    int cont[NUM_BALDES] = {0};

    for (int i = 0; i < n; i++) {
        int b = indiceBalde(arr[i]);
        baldes[b][cont[b]] = arr[i];
        cont[b]++;
    }

    int k = 0;
    for (int b = 0; b < NUM_BALDES; b++) {
        insertionSort(baldes[b], cont[b]);
        for (int j = 0; j < cont[b]; j++)
            arr[k++] = baldes[b][j];
    }
}