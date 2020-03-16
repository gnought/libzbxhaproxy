#include "array.h"

Array* arr_new(const int size) {
    Array* arr = (Array*)zbx_malloc(NULL, sizeof(Array));
    arr->items = (char**)zbx_calloc(NULL, size, sizeof(char*));
    arr->used = 0;
    arr->size = size;
    return arr;
}

static Array* arr_resize(Array* arr, const int len) {
    if (arr->size == len) {
        return arr;
    }
    if (arr->size > len) {
        // reduce
        memset(arr->items + len, '\0', sizeof(char*) * arr->size);
        arr->items = (char**)zbx_realloc(arr->items, sizeof(char*) * len);
    } else {
        // expand
        arr->items = (char**)zbx_realloc(arr->items, sizeof(char*) * len);
        memset(arr->items + arr->size - 1, '\0', sizeof(char*) * len);
    }
    if (arr->used > len) {
        arr->used = len;
    }
    // new size
    arr->size = len;
    return arr;
}

void arr_flush(Array* arr) {
    for (int i = 0; i < arr->size; i++) {
        arr_del(arr, i);
    }
    // memset(arr->items, '\0', sizeof(char*) * arr->size);
    arr->used = 0;
}

void arr_push(Array* arr, char* element) {
    arr_set(arr, arr->used, element);
}

char* arr_get(Array* arr, int index) {
    if (index >= arr->size) {
        return NULL;
    }
    return arr->items[index];
}

void arr_del(Array* arr, int index) {
    if (index >= arr->size) {
        return;
    }
    zbx_free(arr->items[index]);
    arr->items[index] = NULL;
    arr->used--;
}

void arr_set(Array* arr, const int index, char* element) {
    if (index >= arr->size) {
        arr = arr_resize(arr, arr->size * (int)ceil((index + 1.0) / arr->size));
    }
    arr->items[index] = zbx_strdup(NULL, element);
    arr->used++;
}

void arr_free(Array* arr) {
    arr_flush(arr);
    arr->used = arr->size = 0;
    zbx_free(arr->items);
    arr->items = NULL;
    zbx_free(arr);
    arr = NULL;
}

static int arr_foreach(Array* arr, const char* key, int (*fp)(const char*, const char*)) {
    for (int i = 0; i < arr->size; i++) {
        if (key == NULL && arr->items[i] == NULL) return i;
        if (arr->items[i] == NULL) continue;
        if((*fp)(key, arr->items[i]) == 0) {
          return i;
        }
    }
    return -1;
}

// static void arr_foreach(Array* arr, void (*fp)(int index, const char*)) {
//     for (int i = 0; i < arr->size; i++) {
//         (*fp)(i, arr->items[i]);
//     }
// }

// first occurence
int arr_indexOf(Array* arr, const char* term) {
    return arr_foreach(arr, term, &strcmp);
}
