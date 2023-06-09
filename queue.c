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

static bool initialized;
static struct queue_node *head;
static struct queue_node *tail;
static size_t queue_size;
static size_t visited_items;
static size_t waiting_cnt;

static mtx_t queue_mtx;
static cnd_t queue_not_empty_cnd;

static void *pop_queue_head() {
    void *item;
    struct queue_node *prev_head;
    prev_head = head;
    item = head->item;
    if ((head = head->next) == NULL) {
        tail = NULL;
    }
    queue_size--;
    visited_items++;
    free(prev_head);
    return item;
}

void initQueue(void) {
    initialized = true;
    head = NULL;
    tail = NULL;
    queue_size = 0;
    visited_items = 0;
    waiting_cnt = 0;
    cnd_init(&queue_not_empty_cnd);
    mtx_init(&queue_mtx, mtx_plain);
}

void destroyQueue(void) {
    void *item;
    while (tryDequeue(&item)) {
    }
    initialized = false;
    cnd_destroy(&queue_not_empty_cnd);
    mtx_destroy(&queue_mtx);
}

void enqueue(void *item) {
    struct queue_node *new_tail = malloc(sizeof(struct queue_node));
    new_tail->item = item;
    new_tail->next = NULL;
    mtx_lock(&queue_mtx);
    if (head == NULL) {
        head = new_tail;
    } else {
        tail->next = new_tail;
    }
    tail = new_tail;
    queue_size++;
    mtx_unlock(&queue_mtx);
    cnd_signal(&queue_not_empty_cnd);
}

void *dequeue(void) {
    void *item;
    mtx_lock(&queue_mtx);
    while (head == NULL) {
        waiting_cnt++;
        cnd_wait(&queue_not_empty_cnd, &queue_mtx);
        waiting_cnt--;
    }

    item = pop_queue_head();
    mtx_unlock(&queue_mtx);
    return item;
}

bool tryDequeue(void **item_ptr) {
    if (head == NULL) {
        return false;
    }

    mtx_lock(&queue_mtx);
    *item_ptr = pop_queue_head();
    mtx_unlock(&queue_mtx);
    return true;
}

size_t size(void) {
    return queue_size;
}

size_t waiting(void) {
    return waiting_cnt;
}

size_t visited(void) {
    return visited_items;
}
