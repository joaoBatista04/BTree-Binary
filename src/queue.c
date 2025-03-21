#include <stdio.h>
#include <stdlib.h>
#include "../include/queue.h"

struct QueueNode {
    Node *n;
    QueueNode *next;
};

struct Queue {
    QueueNode *head;
    QueueNode *tail;
    int size;
};

Queue *queue_create(){
    Queue *q = (Queue *)malloc(sizeof(Queue));

    q->head = NULL;
    q->tail = NULL;
    q->size = 0;

    return q;
}

int queue_get_size(Queue *q){
    return q->size;
}

void queue_enqueue(Queue *q, Node *n){
    QueueNode *qn = (QueueNode *)malloc(sizeof(QueueNode));

    qn->n = n;
    qn->next = NULL;

    if (!q->tail){
        q->head = qn;
        q->tail = qn;
    }
    else{
        q->tail->next = qn;
        q->tail = qn;
    }

    q->size++;
}

Node *queue_dequeue(Queue *q){
    if (!q->head)
        return NULL;

    QueueNode *temp = q->head;
    Node *n = temp->n;

    q->head = q->head->next;

    if (q->head == NULL)
        q->tail = NULL;

    free(temp);

    q->size--;

    return n;
}

int queue_is_empty(Queue *q){
    if (!q->head)
        return 1;
    return 0;
}

void queue_destroy(Queue *q){
    free(q);
}