#ifndef QUEUE_H
#define QUEUE_H

#include "./btree.h"

typedef struct QueueNode QueueNode;
typedef struct Queue Queue;

//======================= MEMORY AND GETTERS =======================
Queue *queue_create();
int queue_get_size(Queue *q);
int queue_is_empty(Queue *q);
void queue_destroy(Queue *q);

//======================= MAIN OPERATIONS =======================
void queue_enqueue(Queue *q, Node *n);
Node *queue_dequeue(Queue *q);

#endif