#include "sort.h"
#include <string.h>

int cmp_normal(const void *a, const void *b) {
    return strcmp(((Command*)a)->name, ((Command*)b)->name);
}

int cmp_fav(const void *a, const void *b) {
    Command *A = (Command*)a;
    Command *B = (Command*)b;
    if (A->favorite != B->favorite)
        return B->favorite - A->favorite;
    return strcmp(A->name, B->name);
}

