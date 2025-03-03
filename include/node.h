#ifndef NODE_H
#define NODE_H

#include <stdbool.h>

typedef struct Node Node;

Node *node_create(int bt_n_nodes, int t, bool leaf);
void node_destroy(Node *n);

void disk_write(FILE *fp, Node *n, int t);
Node *disk_read(FILE *fp, int pos, int t);

#endif