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

/**
 * AI Use: Assisted By AI
 * Worker thread that sorts a subrange of the list.
 */
static void *sort_thread(void *arg) {
    ThreadArgs *ta = (ThreadArgs*)arg;
    sort(ta->list, ta->start, ta->end, ta->cmp);
    return NULL;
}

/**
 * AI Use: No AI
 * Allocates and initializes an int on the heap.
 */
static int *make_int(int v) {
    int *p = malloc(sizeof(int));
    *p = v;
    return p;
}

/**
 * AI Use: Assisted By AI
 * Generates a random lowercase string with length in [min_len, max_len].
 */
static char *make_string(size_t min_len, size_t max_len) {
    static const char alphabet[] = "abcdefghijklmnopqrstuvwxyz";

    if (max_len < min_len) {
        // swap or clamp if bad input; here we just swap
        size_t tmp = max_len;
        max_len = min_len;
        min_len = tmp;
    }

    size_t range = max_len - min_len + 1;
    size_t len = min_len + (size_t)(rand() % (int)range);

    char *s = malloc(len + 1);
    if (!s) {
        return NULL;
    }

    for (size_t i = 0; i < len; i++) {
        s[i] = alphabet[rand() % 26];
    }
    s[len] = '\0';
    return s;
}


/**
 * AI Use: Assisted By AI
 * Prints up to 'max' elements from the list for debugging.
 */
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

/**
 * AI Use: Assisted By AI
 * Entry point: parses args, builds list, runs two sort threads, merges and verifies.
 */
#ifndef TEST
int main(int argc, char **argv) {
    srand((unsigned)time(NULL));

    const char *type = NULL;
    size_t n = 0;

    if (argc == 3) {
        // Normal usage: myapp <int|string> <count>
        type = argv[1];
        n = strtoull(argv[2], NULL, 10);
    } else if (argc == 1) {
        // No arguments: assume leak-check / debug run.
        // This makes targets like `make leak` work without changing the Makefile.
        fprintf(stderr, "No arguments provided, defaulting to: int 10000\n");
        type = "int";
        n = 10000;
    } else {
        fprintf(stderr, "Usage: %s <int|string> <count>\n", argv[0]);
        return 1;
    }

    List *full = list_create(LIST_LINKED_SENTINEL);
    if (!full) {
        fprintf(stderr, "Failed to create list\n");
        return 1;
    }

    CompareFunc cmp = NULL;
    FreeFunc freer = free;

    if (strcmp(type, "int") == 0) {
        cmp = compare_int;
        for (size_t i = 0; i < n; i++) {
            int *val = make_int(rand() % 1000);
            if (!val || !list_append(full, val)) {
                fprintf(stderr, "Failed to append int\n");
                list_destroy(full, free);
                return 1;
            }
        }
    } else if (strcmp(type, "string") == 0) {
        cmp = compare_str;
        for (size_t i = 0; i < n; i++) {
            char *s = make_string(5, 15);
            if (!s || !list_append(full, s)) {
                fprintf(stderr, "Failed to append string\n");
                list_destroy(full, free);
                return 1;
            }
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

    if (!left || !right) {
        fprintf(stderr, "Failed to create sublists\n");
        list_destroy(full, freer);
        if (left) list_destroy(left, NULL);
        if (right) list_destroy(right, NULL);
        return 1;
    }

    for (size_t i = 0; i < mid; i++) {
        list_append(left, list_get(full, i));
    }
    for (size_t i = mid; i < n; i++) {
        list_append(right, list_get(full, i));
    }

    // We just moved *pointers* into left/right; full's nodes are now unused.
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

    // left and right are now empty shells; destroy their list metadata + sentinels
    list_destroy(left, NULL);
    list_destroy(right, NULL);

    printf("Sorted sample:   ");
    print_list(sorted, type, 25);

    if (!is_sorted(sorted, cmp)) {
        fprintf(stderr, "ERROR: list not sorted!\n");
    } else {
        printf("List sorted successfully!\n");
    }

    // Frees all nodes and data (int* or char*)
    list_destroy(sorted, freer);
    return 0;
}
#endif

