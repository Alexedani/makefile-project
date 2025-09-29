#ifndef LAB_H
#define LAB_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @file lab.h
 * @brief Header file for a generic list data structure supporting multiple implementations.
 */
typedef struct List List;

/**
 * @enum ListType
 * @brief Enumeration for selecting the list implementation type.
 */
typedef enum {
    LIST_LINKED_SENTINEL
} ListType;

/**
 * @typedef FreeFunc
 * @brief Function pointer type for freeing elements. If NULL, no action is taken.
 * Must be provided by the user when destroying the list or removing elements.
 *
 */
typedef void (*FreeFunc)(void *);


/**
 * @brief Create a new list of the specified type.
 * @param type The type of list to create (e.g., LIST_LINKED_SENTINEL).
 * @return Pointer to the newly created list, or NULL on failure.
 */
List *list_create(ListType type);

/**
 * @brief Destroy the list and free all associated memory.
 * @param list Pointer to the list to destroy.
 * @param free_func Function to free individual elements. If NULL, elements are not freed.
 */
void list_destroy(List *list, FreeFunc free_func);

/**
 * @brief Append an element to the end of the list.
 * @param list Pointer to the list.
 * @param data Pointer to the data to append.
 * @return true on success, false on failure.
 */
bool list_append(List *list, void *data);

/**
 * @brief Insert an element at a specific index.
 * @param list Pointer to the list.
 * @param index Index at which to insert the element.
 * @param data Pointer to the data to insert.
 * @return true on success, false on failure (e.g., index out of bounds).
 */
bool list_insert(List *list, size_t index, void *data);

/**
 * @brief Remove an element at a specific index.
 * @param list Pointer to the list.
 * @param index Index of the element to remove.
 * @return Pointer to the element, or NULL if index is out of bounds.
 */
void *list_remove(List *list, size_t index);

/**
 * @brief Get a pointer the element at a specific index.
 * @param list Pointer to the list.
 * @param index Index of the element to retrieve.
 * @return Pointer to the element, or NULL if index is out of bounds.
 */
void *list_get(const List *list, size_t index);

/**
 * @brief Get the current size of the list.
 * @param list Pointer to the list.
 * @return The number of elements in the list.
 */
size_t list_size(const List *list);

/**
 * @brief Check if the list is empty.
 * @param list Pointer to the list
 * @return true if the list is empty, false otherwise
 */
bool list_is_empty(const List *list);

/**
 * @brief Sort a subrange of the list in-place
 * @param list The list to sort
 * @param start Starting index
 * @param end Ending index 
 * @param cmp Comparison function
 * @return 0 on success, non-zero on failure
 */
typedef int (*CompareFunc)(const void *a, const void *b);

int sort(List *list, size_t start, size_t end, CompareFunc cmp);

/**
 * @brief Merge two sorted lists into a single sorted list.
 * @param a First list.
 * @param b Second list.
 * @param cmp Comparison function.
 * @return A new merged list.
 */
List *merge(List *a, List *b, CompareFunc cmp);

/**
 * @brief Compare integers.
 */
int compare_int(const void *a, const void *b);

/**
 * @brief Compare strings.
 */
int compare_str(const void *a, const void *b);

/**
 * @brief Check if a list is sorted.
 */
bool is_sorted(const List *list, CompareFunc cmp);


#endif // LAB_H