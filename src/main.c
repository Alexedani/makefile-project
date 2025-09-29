#include "lab.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Thread arguments */
typedef struct {
    List *list;
    size_t start;
    size_t end;
    CompareFunc cmp;
} ThreadArgs;

static void *sort_thread(void *arg) {
    ThreadArgs *ta = (ThreadArgs*)arg;
    sort(ta->list, ta->start, ta->end, ta->cmp);
    return NULL;
}

static int *make_int(int v) {
    int *p = malloc(sizeof(int));
    *p = v;
    return p;
}

static char *make_string(size_t min_len, size_t max_len) {
    static const char alphabet[] = "abcdefghijklmnopqrstuvwxyz";
size_t len = min_len + (size_t)(rand() % (max_len - (int)min_len + 1));

    char *s = malloc(len + 1);
    for (size_t i = 0; i < len; i++) {
        s[i] = alphabet[rand() % 26];
    }
    s[len] = '\0';
    return s;
}

static void print_list(const List *list, const char *type, size_t max) {
    printf("[");
    size_t n = list_size(list);
    for (size_t i = 0; i < n && i < max; i++) {
        void *d = list_get(list, i);
        if (strcmp(type, "int") == 0) {
            printf("%d", *(int*)d);
        } else {
            printf("\"%s\"", (char*)d);
        }
        if (i < n - 1 && i < max - 1) {
            printf(", ");
        }
    }
    if (n > max) printf(", ...");
    printf("]\n");
}

#ifndef TEST   
int main(int argc, char **argv) {
    srand((unsigned)time(NULL));

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <int|string> <count>\n", argv[0]);
        return 1;
    }

    const char *type = argv[1];
    size_t n = strtoull(argv[2], NULL, 10);

    List *full = list_create(LIST_LINKED_SENTINEL);
    CompareFunc cmp = NULL;
    FreeFunc freer = free;

    if (strcmp(type, "int") == 0) {
        cmp = compare_int;
        for (size_t i = 0; i < n; i++) {
            list_append(full, make_int(rand() % 1000));
        }
    } else if (strcmp(type, "string") == 0) {
        cmp = compare_str;
        for (size_t i = 0; i < n; i++) {
            list_append(full, make_string(5, 15));
        }
    } else {
        fprintf(stderr, "type must be int or string\n");
        list_destroy(full, free);
        return 2;
    }

    printf("Unsorted sample: ");
    print_list(full, type, 15);

    // --- Split into two lists ---
    size_t mid = n / 2;
    List *left = list_create(LIST_LINKED_SENTINEL);
    List *right = list_create(LIST_LINKED_SENTINEL);

    for (size_t i = 0; i < mid; i++) {
        list_append(left, list_get(full, i));
    }
    for (size_t i = mid; i < n; i++) {
        list_append(right, list_get(full, i));
    }

    list_destroy(full, NULL);  // no need for original anymore

    // --- Threaded sort ---
    pthread_t t1, t2;
    ThreadArgs a = { .list = left,  .start = 0, .end = list_size(left),  .cmp = cmp };
    ThreadArgs b = { .list = right, .start = 0, .end = list_size(right), .cmp = cmp };

    pthread_create(&t1, NULL, sort_thread, &a);
    pthread_create(&t2, NULL, sort_thread, &b);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    // --- Merge ---
    List *sorted = merge(left, right, cmp);

    printf("Sorted sample:   ");
    print_list(sorted, type, 25);

    if (!is_sorted(sorted, cmp)) {
        fprintf(stderr, "ERROR: list not sorted!\n");
    } else {
        printf("List sorted successfully!\n");
    }

    list_destroy(sorted, freer);
    return 0;
}
#endif
