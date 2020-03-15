#include "array.h"

Array* arr_init(int initialSize) {
    Array* arr = (Array*)malloc(sizeof(Array));
    arr->items = (char**)calloc(initialSize, sizeof(char*));
    arr->used = 0;
    arr->size = initialSize;
    return arr;
}

void arr_push(Array* arr, char* element) {
    if (arr->used == arr->size) {
        arr->size *= 2;
        arr->items = (char**)realloc(arr->items, arr->size * sizeof(char*));
    }
    arr->items[arr->used++] = element;
}

void arr_free(Array* arr) {
    arr->used = arr->size = 0;
    for (int i = 0; i < arr->used; i++) {
        free(arr->items[i]);
        arr->items[i] = NULL;
    }
    free(arr->items);
    arr->items = NULL;
    free(arr);
    arr = NULL;
}

int arr_indexOf(Array* arr, char* term) {
    for (int i = 0; i < arr->used; i++) {
        if (strcmp(term, arr->items[i]) == 0) {
            return i;
        }
    }
    return -1;
}
