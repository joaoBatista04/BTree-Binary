#ifndef B_TREE_H
#define B_TREE_H

#include <stdbool.h>

typedef struct Node Node;
typedef struct BTree BTree;

//============================== NODE FUNCTIONS ==============================
Node *node_create(BTree *bt, bool is_leaf, int pos);
size_t node_size(BTree *bt);
void node_destroy(Node *n);
//============================== NODE FUNCTIONS ==============================

//============================== ACCESS FUNCTIONS ==============================
void disk_write(BTree *btree, Node *n);
Node *disk_read(BTree *btree, int pos);
//============================== ACCESS FUNCTIONS ==============================

//============================== B-TREE FUNCTIONS ==============================
BTree *btree_create(char *path, int order);
void btree_destroy(BTree *bt);
Node *btree_get_root(BTree *bt);

//============================== INSERT FUNCTIONS ==============================
void btree_insert(BTree *bt, int key, int record);
void split_child(BTree *bt, Node *x, Node *y, int index);
void insert_non_full(BTree *bt, Node *node, int key, int record);

//============================== SEARCH FUNCTIONS ==============================
bool btree_search(BTree *bt, int key);
bool search_node(BTree *bt, Node *n, int key);

//============================== DELETE FUNCTIONS ==============================
void btree_delete(BTree *bt, int key);
void remove_from_node(BTree *bt, Node *n, int key);
void remove_from_leaf(BTree *bt, Node *n, int i);
void remove_from_non_leaf(BTree *bt, Node *n, int i);
int get_predecessor(BTree *bt, Node *n, int i);
int get_successor(BTree *bt, Node *n, int i);
void merge_nodes(BTree *bt, Node *n, int i);
void borrow_from_prev(BTree *bt, Node *n, int i);
void borrow_from_next(BTree *bt, Node *n, int i);
void fill_node(BTree *bt, Node *n, int i);
int find_key_index(Node *node, int key);

//============================== PRINT FUNCTION ==============================
void btree_level_order_print(BTree *bt, FILE *fp);

#endif