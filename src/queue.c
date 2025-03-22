#include <stdio.h>
#include <stdlib.h>
#include "../include/queue.h"

struct QueueNode
{
    Node *n;         // Queue's item is a node
    QueueNode *next; // Pointer to next node in the Queue
};

struct Queue
{
    QueueNode *head; // Reference to head of the queue
    QueueNode *tail; // Reference to tail of the queue
    int size;        // Get the size of the queue
};

/**
 * @brief Create a queue and allocate memory to it
 *
 * @return Queue*
 */
Queue *queue_create()
{
    Queue *q = (Queue *)malloc(sizeof(Queue));

    q->head = NULL;
    q->tail = NULL;
    q->size = 0;

    return q;
}

/**
 * @brief Get the size of the queue
 *
 * @param q
 * @return int
 */
int queue_get_size(Queue *q)
{
    return q->size;
}

/**
 * @brief Enqueue a node
 *
 * @param q
 * @param n
 */
void queue_enqueue(Queue *q, Node *n)
{
    QueueNode *qn = (QueueNode *)malloc(sizeof(QueueNode));

    qn->n = n;
    qn->next = NULL;

    if (!q->tail)
    {
        q->head = qn;
        q->tail = qn;
    }
    else
    {
        q->tail->next = qn;
        q->tail = qn;
    }

    q->size++;
}

/**
 * @brief Dequeue a node
 *
 * @param q
 * @return Node*
 */
Node *queue_dequeue(Queue *q)
{
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

/**
 * @brief Verifiy if queue is empty
 *
 * @param q
 * @return int
 */
int queue_is_empty(Queue *q)
{
    if (!q->head)
        return 1;
    return 0;
}

/**
 * @brief Free memory allocated to queue
 *
 * @param q
 */
void queue_destroy(Queue *q)
{
    free(q);
}