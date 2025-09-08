#include <stdlib.h>
#include <stdio.h>
#include "harness/unity.h"
#include "../src/lab.h"

void setUp(void) {
}

void tearDown(void) {
}

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

    // Invalid get
    TEST_ASSERT_NULL(list_get(list, 0));

    // Invalid remove
    TEST_ASSERT_NULL(list_remove(list, 0));

    // Invalid insert (too high index)
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

    TEST_ASSERT_EQUAL_UINT32(3, list_size(list));

    list_destroy(list, custom_free);

    TEST_ASSERT_EQUAL_INT(3, free_count); // all freed
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_create_and_destroy);
    RUN_TEST(test_append_and_get);
    RUN_TEST(test_insert);
    RUN_TEST(test_remove);
    RUN_TEST(test_invalid_ops);
    RUN_TEST(test_destroy_with_free_func);
    return UNITY_END();
}