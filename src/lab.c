#include "lab.h"
#include <pthread.h>
#include <errno.h>

/* Opaque struct matches typedef in lab.h */
struct queue {
    int capacity;
    int count;
    int head;
    int tail;
    int shutdown;             /* 0 = running, 1 = shutting down */

    void **buf;               /* circular buffer of void* */

    pthread_mutex_t mtx;      /* monitor lock */
    pthread_cond_t  not_full; /* signaled when space is available */
    pthread_cond_t  not_empty;/* signaled when data is available */
};

/* Internal helper: next index in circular buffer */
/**
 * AI Use: Written By AI
 */
static inline int next_idx(int i, int cap) {
    return (i + 1) % cap;
}

/**
 * Initialize a new queue with fixed capacity.
 * AI Use: Written By AI
 */
queue_t queue_init(int capacity) {
    if (capacity <= 0) return NULL;

    struct queue *q = (struct queue *)calloc(1, sizeof(*q));
    if (!q) return NULL;

    q->buf = (void **)calloc((size_t)capacity, sizeof(void *));
    if (!q->buf) {
        free(q);
        return NULL;
    }

    q->capacity = capacity;
    q->count = 0;
    q->head = 0;
    q->tail = 0;
    q->shutdown = 0;

    /* default pthread attributes are fine (process-private) */
    if (pthread_mutex_init(&q->mtx, NULL) != 0) {
        free(q->buf);
        free(q);
        return NULL;
    }
    if (pthread_cond_init(&q->not_full, NULL) != 0) {
        pthread_mutex_destroy(&q->mtx);
        free(q->buf);
        free(q);
        return NULL;
    }
    if (pthread_cond_init(&q->not_empty, NULL) != 0) {
        pthread_cond_destroy(&q->not_full);
        pthread_mutex_destroy(&q->mtx);
        free(q->buf);
        free(q);
        return NULL;
    }

    return (queue_t)q;
}

/**
 * Frees all memory; wakes any waiting threads first.
 * Safe to call after queue_shutdown() or as last cleanup.
 * AI Use: Written By AI
 */
void queue_destroy(queue_t qh) {
    if (!qh) return;
    struct queue *q = (struct queue *)qh;

    pthread_mutex_lock(&q->mtx);
    q->shutdown = 1;
    pthread_cond_broadcast(&q->not_full);
    pthread_cond_broadcast(&q->not_empty);
    pthread_mutex_unlock(&q->mtx);

    pthread_cond_destroy(&q->not_full);
    pthread_cond_destroy(&q->not_empty);
    pthread_mutex_destroy(&q->mtx);

    /* NOTE: At this point, your main has joined all threads and freed
       any dequeued items. Any residual pointers in buf are application-
       managed; per your driver, producers stop before shutdown. */
    free(q->buf);
    free(q);
}

/**
 * Enqueue an element, blocking while full.
 * If queue is shutdown, returns immediately without enqueuing.
 * (Your driver never enqueues post-shutdown; this prevents deadlock if it did.)
 * AI Use: Written By AI
 */
void enqueue(queue_t qh, void *data) {
    if (!qh) return;
    struct queue *q = (struct queue *)qh;

    pthread_mutex_lock(&q->mtx);

    /* Block while full, but never block if shutdown was requested. */
    while (!q->shutdown && q->count == q->capacity) {
        pthread_cond_wait(&q->not_full, &q->mtx);
    }
    if (q->shutdown) {
        /* Drop item on the floor; the driver shuts down only after producers finish,
           so this path is just a safety valve. */
        pthread_mutex_unlock(&q->mtx);
        return;
    }

    q->buf[q->tail] = data;
    q->tail = next_idx(q->tail, q->capacity);
    q->count++;

    /* Wake one consumer. Using signal preserves throughput; broadcast also okay. */
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->mtx);
}

/**
 * Dequeue an element, blocking while empty.
 * After shutdown: drains remaining items; once empty, returns NULL.
 * AI Use: Written By AI
 */
void *dequeue(queue_t qh) {
    if (!qh) return NULL;
    struct queue *q = (struct queue *)qh;

    pthread_mutex_lock(&q->mtx);

    /* Block while empty and not shutdown */
    while (q->count == 0 && !q->shutdown) {
        pthread_cond_wait(&q->not_empty, &q->mtx);
    }

    /* If empty and shutdown, return NULL to let consumers exit */
    if (q->count == 0 && q->shutdown) {
        pthread_mutex_unlock(&q->mtx);
        return NULL;
    }

    /* Normal dequeue */
    void *item = q->buf[q->head];
    q->buf[q->head] = NULL; /* helps ASan/diagnostics */
    q->head = next_idx(q->head, q->capacity);
    q->count--;

    /* Make space visible to producers */
    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->mtx);

    return item;
}

/**
 * Set shutdown flag and wake all waiters.
 * After this call:
 *  - enqueue() returns immediately without adding new data
 *  - dequeue() returns remaining items; once empty, returns NULL
 * AI Use: Written By AI
 */
void queue_shutdown(queue_t qh) {
    if (!qh) return;
    struct queue *q = (struct queue *)qh;

    pthread_mutex_lock(&q->mtx);
    if (!q->shutdown) {
        q->shutdown = 1;
        pthread_cond_broadcast(&q->not_full);
        pthread_cond_broadcast(&q->not_empty);
    }
    pthread_mutex_unlock(&q->mtx);
}

/**
 * Returns true if the queue is currently empty (snapshot).
 * AI Use: Written By AI
 */
bool is_empty(queue_t qh) {
    if (!qh) return true;
    struct queue *q = (struct queue *)qh;

    pthread_mutex_lock(&q->mtx);
    bool empty = (q->count == 0);
    pthread_mutex_unlock(&q->mtx);
    return empty;
}

/**
 * Returns true if shutdown has been requested.
 * AI Use: Written By AI
 */
bool is_shutdown(queue_t qh) {
    if (!qh) return true;
    struct queue *q = (struct queue *)qh;

    pthread_mutex_lock(&q->mtx);
    bool s = (q->shutdown != 0);
    pthread_mutex_unlock(&q->mtx);
    return s;
}
