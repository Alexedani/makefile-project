#include <stdlib.h>
#include <stdio.h>
#include "harness/unity.h"
#include "../src/lab.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

void test_create_and_destroy(void) {
    List *list = list_create(LIST_LINKED_SENTINEL);
    TEST_ASSERT_NOT_NULL(list);
    TEST_ASSERT_TRUE(list_is_empty(list));
    TEST_ASSERT_EQUAL_UINT32(0, list_size(list));
    list_destroy(list, NULL);
}

void test_append_and_get(void) {
    List *list = list_create(LIST_LINKED_SENTINEL);
    int a = 10, b = 20, c = 30;

    TEST_ASSERT_TRUE(list_append(list, &a));
    TEST_ASSERT_TRUE(list_append(list, &b));
    TEST_ASSERT_TRUE(list_append(list, &c));

    TEST_ASSERT_EQUAL_UINT32(3, list_size(list));
    TEST_ASSERT_EQUAL_PTR(&a, list_get(list, 0));
    TEST_ASSERT_EQUAL_PTR(&b, list_get(list, 1));
    TEST_ASSERT_EQUAL_PTR(&c, list_get(list, 2));

    list_destroy(list, NULL);
}

void test_insert(void) {
    List *list = list_create(LIST_LINKED_SENTINEL);
    int a = 1, b = 2, c = 3, d = 4;

    list_append(list, &a); // [1]
    list_append(list, &c); // [1,3]
    TEST_ASSERT_TRUE(list_insert(list, 1, &b)); // [1,2,3]
    TEST_ASSERT_TRUE(list_insert(list, 3, &d)); // [1,2,3,4]

    TEST_ASSERT_EQUAL_UINT32(4, list_size(list));
    TEST_ASSERT_EQUAL_PTR(&a, list_get(list, 0));
    TEST_ASSERT_EQUAL_PTR(&b, list_get(list, 1));
    TEST_ASSERT_EQUAL_PTR(&c, list_get(list, 2));
    TEST_ASSERT_EQUAL_PTR(&d, list_get(list, 3));

    list_destroy(list, NULL);
}

void test_remove(void) {
    List *list = list_create(LIST_LINKED_SENTINEL);
    int a = 5, b = 6, c = 7;

    list_append(list, &a);
    list_append(list, &b);
    list_append(list, &c);

    void *removed = list_remove(list, 1); // remove middle
    TEST_ASSERT_EQUAL_PTR(&b, removed);
    TEST_ASSERT_EQUAL_UINT32(2, list_size(list));
    TEST_ASSERT_EQUAL_PTR(&a, list_get(list, 0));
    TEST_ASSERT_EQUAL_PTR(&c, list_get(list, 1));

    removed = list_remove(list, 0); // remove head
    TEST_ASSERT_EQUAL_PTR(&a, removed);
    TEST_ASSERT_EQUAL_UINT32(1, list_size(list));

    removed = list_remove(list, 0); // remove last
    TEST_ASSERT_EQUAL_PTR(&c, removed);
    TEST_ASSERT_TRUE(list_is_empty(list));

    list_destroy(list, NULL);
}

void test_invalid_ops(void) {
    List *list = list_create(LIST_LINKED_SENTINEL);

    TEST_ASSERT_NULL(list_get(list, 0));
    TEST_ASSERT_NULL(list_remove(list, 0));

    int x = 42;
    TEST_ASSERT_FALSE(list_insert(list, 5, &x));

    list_destroy(list, NULL);
}

static int free_count = 0;
void custom_free(void *ptr) {
    if (ptr) {
        free(ptr);
        free_count++;
    }
}

void test_destroy_with_free_func(void) {
    free_count = 0;
    List *list = list_create(LIST_LINKED_SENTINEL);

    int *a = malloc(sizeof(int));
    int *b = malloc(sizeof(int));
    int *c = malloc(sizeof(int));
    *a = 10; *b = 20; *c = 30;

    list_append(list, a);
    list_append(list, b);
    list_append(list, c);

    list_destroy(list, custom_free);

    TEST_ASSERT_EQUAL_INT(3, free_count); // all freed
}

/* === New Function Tests === */

void test_sort_ints(void) {
    List *list = list_create(LIST_LINKED_SENTINEL);

    int *a = malloc(sizeof(int)); *a = 5;
    int *b = malloc(sizeof(int)); *b = 9;
    int *c = malloc(sizeof(int)); *c = 1;

    list_append(list, a);
    list_append(list, b);
    list_append(list, c);

    sort(list, 0, list_size(list), compare_int);

    TEST_ASSERT_TRUE(is_sorted(list, compare_int));

    list_destroy(list, custom_free);
}

void test_sort_strings(void) {
    List *list = list_create(LIST_LINKED_SENTINEL);

    list_append(list, strdup("pear"));
    list_append(list, strdup("apple"));
    list_append(list, strdup("orange"));

    sort(list, 0, list_size(list), compare_str);

    TEST_ASSERT_TRUE(is_sorted(list, compare_str));

    list_destroy(list, free);
}

void test_merge_lists(void) {
    List *a = list_create(LIST_LINKED_SENTINEL);
    List *b = list_create(LIST_LINKED_SENTINEL);

    int *x = malloc(sizeof(int)); *x = 10;
    int *y = malloc(sizeof(int)); *y = 7;
    int *z = malloc(sizeof(int)); *z = 3;

    list_append(a, x);
    list_append(b, y);
    list_append(b, z);

    sort(a, 0, list_size(a), compare_int);
    sort(b, 0, list_size(b), compare_int);

    List *merged = merge(a, b, compare_int);

    TEST_ASSERT_EQUAL_UINT32(3, list_size(merged));
    TEST_ASSERT_TRUE(is_sorted(merged, compare_int));

    list_destroy(merged, custom_free);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_create_and_destroy);
    RUN_TEST(test_append_and_get);
    RUN_TEST(test_insert);
    RUN_TEST(test_remove);
    RUN_TEST(test_invalid_ops);
    RUN_TEST(test_destroy_with_free_func);
    RUN_TEST(test_sort_ints);
    RUN_TEST(test_sort_strings);
    RUN_TEST(test_merge_lists);
    return UNITY_END();
}
