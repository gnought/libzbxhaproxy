#include <stdlib.h>
#include <string.h>

typedef struct {
    char** items;
    int used;
    int size;
} Array;

Array* arr_init(int initialSize);
void arr_push(Array* arr, char* element);
void arr_free(Array* arr);
int arr_indexOf(Array* arr, char* term);
