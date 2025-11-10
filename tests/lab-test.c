#include <stdlib.h>
#include <pthread.h>
#include "harness/unity.h"
#include "../src/lab.h"

static void free_int(void *p) { free(p); }

void setUp(void) {}
void tearDown(void) {}

/* Basic enqueue/dequeue FIFO */
void test_fifo_basic(void) {
    queue_t q = queue_init(2);
    TEST_ASSERT_NOT_NULL(q);

    int *a = malloc(sizeof(int)); *a = 1;
    int *b = malloc(sizeof(int)); *b = 2;

    enqueue(q, a);
    enqueue(q, b);
    TEST_ASSERT_FALSE(is_empty(q));

    int *x = (int*)dequeue(q);
    int *y = (int*)dequeue(q);
    TEST_ASSERT_EQUAL_INT(1, *x);
    TEST_ASSERT_EQUAL_INT(2, *y);
    free(x); free(y);

    TEST_ASSERT_TRUE(is_empty(q));
    queue_destroy(q);
}

/* Shutdown semantics: after shutdown, dequeue drains then returns NULL */
void test_shutdown_and_drain(void) {
    queue_t q = queue_init(3);
    TEST_ASSERT_NOT_NULL(q);

    for (int i=0;i<3;i++){
        int *p = malloc(sizeof(int)); *p = i;
        enqueue(q, p);
    }

    queue_shutdown(q);

    for (int i=0;i<3;i++){
        int *p = (int*)dequeue(q);
        TEST_ASSERT_NOT_NULL(p);
        free(p);
    }

    /* Empty + shutdown => NULL */
    TEST_ASSERT_NULL(dequeue(q));
    TEST_ASSERT_TRUE(is_shutdown(q));
    queue_destroy(q);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_fifo_basic);
    RUN_TEST(test_shutdown_and_drain);
    return UNITY_END();
}
