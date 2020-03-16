#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>

#include "common.h"

typedef struct {
    char** items;
    int used;
    int size;
} Array;

Array* arr_new(const int size);
void arr_flush(Array* arr);
void arr_clear(Array* arr, const int start, int len);
void arr_push(Array* arr, char* element);
char* arr_get(Array* arr, int index);
void arr_del(Array* arr, int index);
void arr_set(Array* arr, const int index, char* element);
void arr_free(Array* arr);
int arr_indexOf(Array* arr, const char* term);
