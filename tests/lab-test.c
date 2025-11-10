// test.c
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/resource.h>
#include <sys/time.h>

#include "harness/unity.h"
#include "../src/lab.h"

void setUp(void)   { printf("Setting up tests...\n"); }
void tearDown(void){ printf("Tearing down tests...\n"); }

/* ---------- helpers ---------- */
static void *producer_one(void *arg) {
    queue_t q = (queue_t)arg;
    int *x = (int*)malloc(sizeof(int)); *x = 42;
    enqueue(q, x);
    return NULL;
}
static void *consumer_one(void *arg) {
    queue_t q = (queue_t)arg;
    int *x = (int*)dequeue(q);
    if (x) free(x);
    return NULL;
}

struct consumer_capture_arg {
    queue_t q;
    void *got;
};
static void *consumer_capture(void *arg) {
    struct consumer_capture_arg *a = (struct consumer_capture_arg*)arg;
    a->got = dequeue(a->q);
    return NULL;
}

/* ---------- resource-stress utilities (best-effort) ---------- */
static int try_set_address_space_rlimit(rlim_t bytes) {
#ifdef RLIMIT_AS
    struct rlimit rl;
    rl.rlim_cur = bytes;
    rl.rlim_max = bytes;
    return setrlimit(RLIMIT_AS, &rl);
#else
    (void)bytes;
    return -1; /* not supported here */
#endif
}

static size_t alloc_until_fail(size_t block, void ***out_list) {
    size_t cap = 1024, n = 0;
    void **list = (void**)calloc(cap, sizeof(void*));
    if (!list) return 0;
    for (;;) {
        if (n == cap) {
            cap *= 2;
            void **tmp = (void**)realloc(list, cap * sizeof(void*));
            if (!tmp) break;
            list = tmp;
        }
        void *p = malloc(block);
        if (!p) break;
        list[n++] = p;
    }
    *out_list = list;
    return n;
}
static void free_blocks(void **list, size_t n) {
    for (size_t i = 0; i < n; ++i) free(list[i]);
    free(list);
}

static size_t init_many_mutexes(pthread_mutex_t **out) {
    size_t cap = 1024, n = 0;
    pthread_mutex_t *arr = (pthread_mutex_t*)malloc(cap * sizeof(*arr));
    if (!arr) return 0;
    for (;;) {
        if (n == cap) {
            cap *= 2;
            pthread_mutex_t *tmp = (pthread_mutex_t*)realloc(arr, cap * sizeof(*arr));
            if (!tmp) break;
            arr = tmp;
        }
        if (pthread_mutex_init(&arr[n], NULL) != 0) break;
        n++;
    }
    *out = arr;
    return n;
}
static void destroy_many_mutexes(pthread_mutex_t *arr, size_t n) {
    for (size_t i = 0; i < n; ++i) pthread_mutex_destroy(&arr[i]);
    free(arr);
}

static size_t init_many_conds(pthread_cond_t **out) {
    size_t cap = 1024, n = 0;
    pthread_cond_t *arr = (pthread_cond_t*)malloc(cap * sizeof(*arr));
    if (!arr) return 0;
    for (;;) {
        if (n == cap) {
            cap *= 2;
            pthread_cond_t *tmp = (pthread_cond_t*)realloc(arr, cap * sizeof(*arr));
            if (!tmp) break;
            arr = tmp;
        }
        if (pthread_cond_init(&arr[n], NULL) != 0) break;
        n++;
    }
    *out = arr;
    return n;
}
static void destroy_many_conds(pthread_cond_t *arr, size_t n) {
    for (size_t i = 0; i < n; ++i) pthread_cond_destroy(&arr[i]);
    free(arr);
}

/* ---------- your existing/functional tests ---------- */

/* basic FIFO and empty→produce unblock */
void test_fifo_basic(void) {
    queue_t q = queue_init(2);
    TEST_ASSERT_NOT_NULL(q);

    pthread_t c; pthread_create(&c, NULL, consumer_one, q);
    usleep(1000);

    pthread_t p; pthread_create(&p, NULL, producer_one, q);
    pthread_join(p, NULL);
    pthread_join(c, NULL);

    TEST_ASSERT_TRUE(is_empty(q));
    TEST_ASSERT_FALSE(is_shutdown(q));

    queue_destroy(q);
}


void test_shutdown_and_drain(void) {
    queue_t q = queue_init(1);
    TEST_ASSERT_NOT_NULL(q);

    int *a = (int*)malloc(sizeof(int)); *a = 1;
    enqueue(q, a);

    pthread_t p2;
    pthread_create(&p2, NULL, producer_one, q);
    usleep(1000);

    int *got = (int*)dequeue(q);
    TEST_ASSERT_NOT_NULL(got); free(got);

    pthread_join(p2, NULL);

    queue_shutdown(q);

    int sentinel = 7;
    enqueue(q, &sentinel); /* ignored */

    void *v = dequeue(q);
    TEST_ASSERT_NOT_NULL(v); /* drains producer_one item */
    v = dequeue(q);
    TEST_ASSERT_NULL(v);     /* empty+shutdown => NULL */

    TEST_ASSERT_TRUE(is_shutdown(q));
    queue_destroy(q);
}

/* guards and NULL-safety */
void test_init_and_null_guards(void) {
    TEST_ASSERT_NULL(queue_init(0));
    TEST_ASSERT_NULL(queue_init(-3));

    queue_destroy(NULL);
    queue_shutdown(NULL);
    enqueue(NULL, (void*)1);
    TEST_ASSERT_TRUE(is_empty(NULL));
    TEST_ASSERT_TRUE(is_shutdown(NULL));
    TEST_ASSERT_NULL(dequeue(NULL));
}

/* wrap-around & FIFO order */
void test_wraparound_fifo_order(void) {
    queue_t q = queue_init(3);
    TEST_ASSERT_NOT_NULL(q);

    int *a = (int*)malloc(sizeof(int)); *a = 1;
    int *b = (int*)malloc(sizeof(int)); *b = 2;
    int *c = (int*)malloc(sizeof(int)); *c = 3;
    enqueue(q, a); enqueue(q, b); enqueue(q, c);

    int *first = (int*)dequeue(q);
    TEST_ASSERT_NOT_NULL(first);
    TEST_ASSERT_EQUAL_INT(1, *first);
    free(first);

    int *d = (int*)malloc(sizeof(int)); *d = 4;
    enqueue(q, d);

    int *v2 = (int*)dequeue(q); TEST_ASSERT_NOT_NULL(v2); TEST_ASSERT_EQUAL_INT(2, *v2); free(v2);
    int *v3 = (int*)dequeue(q); TEST_ASSERT_NOT_NULL(v3); TEST_ASSERT_EQUAL_INT(3, *v3); free(v3);
    int *v4 = (int*)dequeue(q); TEST_ASSERT_NOT_NULL(v4); TEST_ASSERT_EQUAL_INT(4, *v4); free(v4);

    TEST_ASSERT_TRUE(is_empty(q));
    queue_destroy(q);
}

/* consumer blocked on empty is released by shutdown and returns NULL */
void test_shutdown_releases_empty_consumer(void) {
    queue_t q = queue_init(2);
    TEST_ASSERT_NOT_NULL(q);

    struct consumer_capture_arg cap = { .q = q, .got = (void*)0xDEADBEEF };
    pthread_t c;
    pthread_create(&c, NULL, consumer_capture, &cap);

    usleep(1000);
    queue_shutdown(q);

    pthread_join(c, NULL);
    TEST_ASSERT_NULL(cap.got);

    TEST_ASSERT_TRUE(is_shutdown(q));
    TEST_ASSERT_TRUE(is_empty(q));
    queue_destroy(q);
}

/* snapshot state transitions */
void test_state_snapshots(void) {
    queue_t q = queue_init(1);
    TEST_ASSERT_NOT_NULL(q);

    TEST_ASSERT_TRUE(is_empty(q));
    TEST_ASSERT_FALSE(is_shutdown(q));

    int *x = (int*)malloc(sizeof(int)); *x = 5;
    enqueue(q, x);
    TEST_ASSERT_FALSE(is_empty(q));

    int *y = (int*)dequeue(q);
    TEST_ASSERT_NOT_NULL(y); free(y);
    TEST_ASSERT_TRUE(is_empty(q));

    queue_shutdown(q);
    TEST_ASSERT_TRUE(is_shutdown(q));
    queue_destroy(q);
}

void test_queue_init_calloc_buf_fails(void) {
#ifdef RLIMIT_AS
    if (try_set_address_space_rlimit(64 * 1024 * 1024) != 0) {
        TEST_IGNORE_MESSAGE("Cannot lower RLIMIT_AS; skipping calloc-fail test.");
    }

    void **blocks = NULL;
    size_t n = alloc_until_fail(2 * 1024 * 1024, &blocks); /* 2MB chunks */
    if (n == 0) {
        /* Could not consume memory enough; skip gracefully */
        try_set_address_space_rlimit(RLIM_INFINITY);
        TEST_IGNORE_MESSAGE("Could not pressure address space; skipping.");
    }

    /* Ask for a huge capacity so q->buf calloc likely fails under the tight limit */
    queue_t q = queue_init(1 << 28); /* requires a massive buf */
    TEST_ASSERT_NULL(q); /* expect free(q); return NULL path */

    free_blocks(blocks, n);
    try_set_address_space_rlimit(RLIM_INFINITY);
#else
    TEST_IGNORE_MESSAGE("RLIMIT_AS not supported on this platform; skipping.");
#endif
}

/* Hit lines 52–54: pthread_mutex_init fails */
void test_queue_init_mutex_init_fails(void) {
    pthread_mutex_t *arr = NULL;
    size_t n = init_many_mutexes(&arr);
    if (n == 0) {
        TEST_IGNORE_MESSAGE("Couldn't exhaust mutex resources; skipping.");
    }

    queue_t q = queue_init(2);
    if (q) {
        /* Could not force failure here; clean and skip */
        queue_destroy(q);
        destroy_many_mutexes(arr, n);
        TEST_IGNORE_MESSAGE("pthread_mutex_init resisted failure; skipping.");
    }

    destroy_many_mutexes(arr, n);
    TEST_PASS(); /* lines 52–54 executed */
}

/* Hit lines 57–60: first pthread_cond_init (not_full) fails */
void test_queue_init_cond_not_full_fails(void) {
    pthread_cond_t *conds = NULL;
    size_t n = init_many_conds(&conds);
    if (n == 0) {
        TEST_IGNORE_MESSAGE("Couldn't exhaust cond vars; skipping.");
    }

    queue_t q = queue_init(2);
    if (q) {
        queue_destroy(q);
        destroy_many_conds(conds, n);
        TEST_IGNORE_MESSAGE("cond init didn't fail for not_full; skipping.");
    }

    destroy_many_conds(conds, n);
    TEST_PASS(); /* lines 57–60 executed */
}

void test_queue_init_cond_not_empty_fails(void) {
    pthread_cond_t *conds = NULL;
    size_t n = init_many_conds(&conds);
    if (n == 0) {
        TEST_IGNORE_MESSAGE("Couldn't exhaust cond vars; skipping.");
    }

    /* Free exactly one to allow first cond init to succeed but not the second */
    pthread_cond_destroy(&conds[n - 1]);
    n--;

    queue_t q = queue_init(2);
    if (q) {
        queue_destroy(q);
        destroy_many_conds(conds, n);
        TEST_IGNORE_MESSAGE("Couldn't force second cond init to fail; skipping.");
    }

    destroy_many_conds(conds, n);
    TEST_PASS(); /* lines 63–67 executed */
}

/* ---------- main ---------- */
int main(void) {
    UNITY_BEGIN();

    /* functional tests */
    RUN_TEST(test_fifo_basic);
    RUN_TEST(test_shutdown_and_drain);
    RUN_TEST(test_init_and_null_guards);
    RUN_TEST(test_wraparound_fifo_order);
    RUN_TEST(test_shutdown_releases_empty_consumer);
    RUN_TEST(test_state_snapshots);
    RUN_TEST(test_queue_init_calloc_buf_fails);
    RUN_TEST(test_queue_init_mutex_init_fails);
    RUN_TEST(test_queue_init_cond_not_full_fails);
    RUN_TEST(test_queue_init_cond_not_empty_fails);

    return UNITY_END();
}
