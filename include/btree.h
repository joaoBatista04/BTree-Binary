#ifndef B_TREE_H
#define B_TREE_H

#include <stdbool.h>

typedef struct Node Node;
typedef struct BTree BTree;

Node *node_create(BTree *bt, bool is_leaf, int pos);
size_t node_size(BTree *bt);
void node_destroy(Node *n);

void disk_write(BTree *btree, Node *n);
Node *disk_read(BTree *btree, int pos);

BTree *btree_create(char *path, int order);
void btree_destroy(BTree *bt);

void btree_insert(BTree *bt, int key, int record);
void split_child(BTree *bt, Node *x, int index);
void insert_non_full(BTree *bt, Node *node, int key, int record);

bool btree_search(BTree *bt, int key);
bool search_node(BTree *bt, Node *n, int key);

#endif