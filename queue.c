#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE
#endif

#include <stdlib.h>
#include <threads.h>

#include "queue.h"

struct queue_node {
    void *item;
    struct queue_node *next;
};

struct queue {
    struct queue_node *head;
    struct queue_node *tail;
    size_t size;
};

static struct queue data_queue;
static struct queue dequeue_order;
static size_t visited_items;

static mtx_t queue_mtx;

static inline struct queue_node *queue_node_init(void *item) {
    struct queue_node *node = malloc(sizeof(struct queue_node));
    node->item = item;
    node->next = NULL;
    return node;
}

static inline void init_queue_struct(struct queue *q) {
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
}

static inline void queue_enqueue(struct queue *q, struct queue_node *new_tail) {
    if (q->head == NULL) {
        q->head = new_tail;
    } else {
        q->tail->next = new_tail;
    }
    q->tail = new_tail;
    q->size++;
}

static inline void *pop_queue_head(struct queue *q) {
    void *item;
    struct queue_node *prev_head;
    prev_head = q->head;
    item = q->head->item;
    if ((q->head = q->head->next) == NULL) {
        q->tail = NULL;
    }
    q->size--;
    free(prev_head);
    return item;
}

static inline bool empty(struct queue *q) {
    return q->size == 0;
}

void initQueue(void) {
    init_queue_struct(&data_queue);
    init_queue_struct(&dequeue_order);
    visited_items = 0;
    mtx_init(&queue_mtx, mtx_plain);
}

void destroyQueue(void) {
    void *item;
    while (tryDequeue(&item)) {
    }
    mtx_destroy(&queue_mtx);
}

void enqueue(void *item) {
    struct queue_node *new_tail_ptr = queue_node_init(item);
    cnd_t *queue_not_empty_cnd_ptr;
    mtx_lock(&queue_mtx);
    queue_enqueue(&data_queue, new_tail_ptr);
    if (!empty(&dequeue_order)) {
        queue_not_empty_cnd_ptr = pop_queue_head(&dequeue_order);
        cnd_signal(queue_not_empty_cnd_ptr);
        mtx_unlock(&queue_mtx);
    } else {
        mtx_unlock(&queue_mtx);
    }
}

void *dequeue(void) {
    void *item;
    cnd_t queue_not_empty_cnd;
    struct queue_node *new_tail_ptr;
    mtx_lock(&queue_mtx);
    while (empty(&data_queue)) {
        cnd_init(&queue_not_empty_cnd);
        new_tail_ptr = queue_node_init(&queue_not_empty_cnd);
        queue_enqueue(&dequeue_order, new_tail_ptr);
        cnd_wait(&queue_not_empty_cnd, &queue_mtx);
        cnd_destroy(&queue_not_empty_cnd);
    }

    item = pop_queue_head(&data_queue);
    visited_items++;
    mtx_unlock(&queue_mtx);
    return item;
}

bool tryDequeue(void **item_ptr) {
    if (data_queue.head == NULL) {
        return false;
    }

    mtx_lock(&queue_mtx);
    *item_ptr = pop_queue_head(&data_queue);
    visited_items++;
    mtx_unlock(&queue_mtx);
    return true;
}

size_t size(void) {
    return data_queue.size;
}

size_t waiting(void) {
    return dequeue_order.size;
}

size_t visited(void) {
    return visited_items;
}
