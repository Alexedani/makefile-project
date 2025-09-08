#include "lab.h"
#include <stdlib.h>
#include <stdio.h>

/**
 * @file lab.c
 * @brief Implementation of a circular doubly linked list with a sentinel node.
 */

/* 
 * Internal node structure for the list
 */
typedef struct Node {
    void *data;
    struct Node *next;
    struct Node *prev;
} Node;

/* 
 * The List structure definition (hidden from lab.h).
 */
struct List {
    ListType type;
    size_t size;
    Node *sentinel;
};

/**
 * Create a new list of the specified type.
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
 * Destroy the list and free all associated memory.
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
 * Append an element to the end of the list.
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
 * Insert an element at a specific index.
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
 * Remove an element at a specific index.
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
 * Get a pointer to the element at a specific index.
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
 * Get the current size of the list.
 * AI Use: Written By AI
 */
size_t list_size(const List *list) {
    return list ? list->size : 0;
}

/**
 * Check if the list is empty.
 * AI Use: Written By AI
 */
bool list_is_empty(const List *list) {
    return !list || list->size == 0;
}