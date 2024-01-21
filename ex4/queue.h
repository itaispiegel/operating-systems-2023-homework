#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void initQueue(void);
void destroyQueue(void);
void enqueue(void *);
void *dequeue(void);
bool tryDequeue(void **);
size_t size(void);
size_t waiting(void);
size_t visited(void);
