#include "lab.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * @file lab.c
 * @brief Implementation of a circular doubly linked list with a sentinel node.
 */

typedef struct Node {
    void *data;
    struct Node *next;
    struct Node *prev;
} Node;

struct List {
    ListType type;
    size_t size;
    Node *sentinel;
};

/**
 * AI Use: Written By AI
 */
List *list_create(ListType type) {
    List *list = malloc(sizeof(List));
    if (!list) return NULL;

    Node *sentinel = malloc(sizeof(Node));
    if (!sentinel) {
        free(list);
        return NULL;
    }

    sentinel->data = NULL;
    sentinel->next = sentinel;
    sentinel->prev = sentinel;

    list->type = type;
    list->size = 0;
    list->sentinel = sentinel;
    return list;
}

/**
 * AI Use: Written By AI
 */
void list_destroy(List *list, FreeFunc free_func) {
    if (!list) return;

    Node *cur = list->sentinel->next;
    while (cur != list->sentinel) {
        Node *next = cur->next;
        if (free_func) free_func(cur->data);
        free(cur);
        cur = next;
    }

    free(list->sentinel);
    free(list);
}

/**
 * AI Use: Written By AI
 */
bool list_append(List *list, void *data) {
    if (!list) return false;

    Node *node = malloc(sizeof(Node));
    if (!node) return false;
    node->data = data;

    Node *tail = list->sentinel->prev;
    tail->next = node;
    node->prev = tail;
    node->next = list->sentinel;
    list->sentinel->prev = node;

    list->size++;
    return true;
}

/**
 * AI Use: Written By AI
 */
bool list_insert(List *list, size_t index, void *data) {
    if (!list || index > list->size) return false;

    Node *node = malloc(sizeof(Node));
    if (!node) return false;
    node->data = data;

    Node *cur = list->sentinel;
    for (size_t i = 0; i < index; i++) {
        cur = cur->next;
    }

    Node *next = cur->next;
    cur->next = node;
    node->prev = cur;
    node->next = next;
    next->prev = node;

    list->size++;
    return true;
}

/**
 * AI Use: Written By AI
 */
void *list_remove(List *list, size_t index) {
    if (!list || index >= list->size) return NULL;

    Node *cur = list->sentinel->next;
    for (size_t i = 0; i < index; i++) {
        cur = cur->next;
    }

    void *data = cur->data;
    cur->prev->next = cur->next;
    cur->next->prev = cur->prev;
    free(cur);

    list->size--;
    return data;
}

/**
 * AI Use: Written By AI
 */
void *list_get(const List *list, size_t index) {
    if (!list || index >= list->size) return NULL;

    Node *cur = list->sentinel->next;
    for (size_t i = 0; i < index; i++) {
        cur = cur->next;
    }
    return cur->data;
}

/**
 * AI Use: Written By AI
 */
size_t list_size(const List *list) {
    return list ? list->size : 0;
}

/**
 * AI Use: Written By AI
 */
bool list_is_empty(const List *list) {
    return !list || list->size == 0;
}

/* === Comparators === */

/**
 * AI Use: Written By AI
 */
int compare_int(const void *a, const void *b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    if (ia > ib) return -1;   // descending
    else if (ia < ib) return 1;
    else return 0;
}

/**
 * AI Use: Written By AI
 */
int compare_str(const void *a, const void *b) {
    const char *sa = (const char*)a;
    const char *sb = (const char*)b;
    return strcmp(sa, sb);    // ascending
}

/**
 * AI Use: Written By AI
 */
static int compare_int_qsort(const void *a, const void *b) {
    const int *ia = *(const int **)a;
    const int *ib = *(const int **)b;
    return compare_int(ia, ib);
}

/**
 * AI Use: Written By AI
 */
static int compare_str_qsort(const void *a, const void *b) {
    const char *sa = *(const char **)a;
    const char *sb = *(const char **)b;
    return compare_str(sa, sb);
}

/* === Sort & Merge === */

/**
 * AI Use: Written By AI
 */
int sort(List *list, size_t start, size_t end, CompareFunc cmp) {
    if (!list || start >= end || end > list->size) return -1;

    size_t count = end - start;
    void **arr = malloc(count * sizeof(void*));
    if (!arr) return -1;

    Node *cur = list->sentinel->next;
    for (size_t i = 0; i < start; i++) cur = cur->next;

    Node *walker = cur;
    for (size_t i = 0; i < count; i++) {
        arr[i] = walker->data;
        walker = walker->next;
    }

    if (cmp == compare_int)
        qsort(arr, count, sizeof(void*), compare_int_qsort);
    else if (cmp == compare_str)
        qsort(arr, count, sizeof(void*), compare_str_qsort);

    walker = cur;
    for (size_t i = 0; i < count; i++) {
        walker->data = arr[i];
        walker = walker->next;
    }

    free(arr);
    return 0;
}

/**
 * AI Use: Written By AI
 */
List *merge(List *a, List *b, CompareFunc cmp) {
    if (!a || !b) return NULL;

    List *out = list_create(LIST_LINKED_SENTINEL);
    if (!out) return NULL;

    Node *ia = a->sentinel->next;
    Node *ib = b->sentinel->next;

    while (ia != a->sentinel && ib != b->sentinel) {
        if (cmp(ia->data, ib->data) <= 0) {
            list_append(out, ia->data);
            ia = ia->next;
        } else {
            list_append(out, ib->data);
            ib = ib->next;
        }
    }
    while (ia != a->sentinel) {
        list_append(out, ia->data);
        ia = ia->next;
    }
    while (ib != b->sentinel) {
        list_append(out, ib->data);
        ib = ib->next;
    }

    // cleanup node shells from a and b
    Node *cur;
    cur = a->sentinel->next;
    while (cur != a->sentinel) {
        Node *next = cur->next;
        free(cur);
        cur = next;
    }
    a->sentinel->next = a->sentinel->prev = a->sentinel;
    a->size = 0;

    cur = b->sentinel->next;
    while (cur != b->sentinel) {
        Node *next = cur->next;
        free(cur);
        cur = next;
    }
    b->sentinel->next = b->sentinel->prev = b->sentinel;
    b->size = 0;

    return out;
}

/**
 * AI Use: Written By AI
 */
bool is_sorted(const List *list, CompareFunc cmp) {
    if (!list || list->size < 2) return true;
    Node *cur = list->sentinel->next;
    while (cur->next != list->sentinel) {
        if (cmp(cur->data, cur->next->data) > 0) return false;
        cur = cur->next;
    }
    return true;
}
