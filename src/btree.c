#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "../include/btree.h"

struct Node
{
    int n_keys;
    bool is_leaf;
    int b_position;
    int *keys;
    int *records;
    int *children;
};

struct BTree
{
    int order;
    Node *root;
    int node_amount;
    FILE *bfile;
};

Node *node_create(BTree *bt, bool is_leaf, int pos)
{
    Node *node = (Node *)malloc(sizeof(Node));

    node->n_keys = 0;
    node->is_leaf = is_leaf;
    node->b_position = pos;

    node->keys = (int *)calloc((bt->order - 1), sizeof(int));
    node->records = (int *)calloc((bt->order - 1), sizeof(int));
    node->children = (int *)calloc(bt->order, sizeof(int));

    for (int i = 0; i < bt->order - 1; i++)
    {
        node->keys[i] = -1;
        node->records[i] = -1;
        node->children[i] = -1;
    }
    node->children[bt->order - 1] = -1;

    return node;
}

size_t node_size(BTree *bt)
{
    return sizeof(int) + sizeof(bool) + sizeof(int) + sizeof(int) * (bt->order - 1) + sizeof(int) * (bt->order - 1) + sizeof(int) * bt->order;
}

void node_destroy(Node *n)
{
    if (n)
    {
        free(n->keys);
        free(n->records);
        free(n->children);
        free(n);
    }
}

void disk_write(BTree *btree, Node *n)
{
    fseek(btree->bfile, n->b_position * node_size(btree), SEEK_SET);

    fwrite(&n->n_keys, sizeof(int), 1, btree->bfile);
    fwrite(&n->is_leaf, sizeof(bool), 1, btree->bfile);
    fwrite(&n->b_position, sizeof(int), 1, btree->bfile);
    fwrite(n->keys, sizeof(int), btree->order - 1, btree->bfile);
    fwrite(n->records, sizeof(int), btree->order - 1, btree->bfile);
    fwrite(n->children, sizeof(int), btree->order, btree->bfile);

    fflush(btree->bfile);
}

Node *disk_read(BTree *btree, int pos)
{
    Node *n = (Node *)malloc(sizeof(Node));
    fseek(btree->bfile, pos * node_size(btree), SEEK_SET);

    fread(&n->n_keys, sizeof(int), 1, btree->bfile);
    fread(&n->is_leaf, sizeof(bool), 1, btree->bfile);
    fread(&n->b_position, sizeof(int), 1, btree->bfile);

    n->keys = (int *)malloc((btree->order - 1) * sizeof(int));
    n->records = (int *)malloc((btree->order - 1) * sizeof(int));
    n->children = (int *)malloc(btree->order * sizeof(int));

    fread(n->keys, sizeof(int), btree->order - 1, btree->bfile);
    fread(n->records, sizeof(int), btree->order - 1, btree->bfile);
    fread(n->children, sizeof(int), btree->order, btree->bfile);

    return n;
}

BTree *btree_create(char *path, int order)
{
    BTree *bt = (BTree *)malloc(sizeof(BTree));

    bt->order = order;
    bt->root = NULL;
    bt->node_amount = 0;

    FILE *fp = fopen("btree.bin", "w+b");

    if (!fp)
    {
        perror("The system couldn't create the binary file.\n");
        exit(1);
    }

    bt->bfile = fp;

    return bt;
}

void btree_destroy(BTree *bt)
{
    fclose(bt->bfile);
    if (bt->root)
    {
        node_destroy(bt->root);
    }
    free(bt);
}

void btree_insert(BTree *bt, int key, int record)
{
    if (bt->root == NULL)
    {
        bt->root = node_create(bt, true, bt->node_amount++);
        bt->root->keys[0] = key;
        bt->root->records[0] = record;
        bt->root->n_keys = 1;
        disk_write(bt, bt->root);
    }
    else
    {
        if (bt->root->n_keys == bt->order - 1)
        {
            Node *new_root = node_create(bt, false, bt->node_amount++);
            new_root->children[0] = bt->root->b_position;

            node_destroy(bt->root);
            bt->root = new_root;
            split_child(bt, bt->root, 0);
            insert_non_full(bt, bt->root, key, record);
        }
        else
        {
            insert_non_full(bt, bt->root, key, record);
        }
    }
}

void split_child(BTree *bt, Node *x, int index)
{
    Node *y = disk_read(bt, x->children[index]);
    Node *z = node_create(bt, y->is_leaf, bt->node_amount++);

    int t = (bt->order - 1) / 2;

    z->n_keys = (bt->order - 1) - t - 1;
    for (int i = 0; i < z->n_keys; i++)
    {
        z->keys[i] = y->keys[i + t + 1];
        z->records[i] = y->records[i + t + 1];
    }

    if (!y->is_leaf)
    {
        for (int i = 0; i <= z->n_keys; i++)
        {
            z->children[i] = y->children[i + t + 1];
        }
    }

    y->n_keys = t;

    for (int i = x->n_keys; i > index; i--)
    {
        x->children[i + 1] = x->children[i];
    }
    x->children[index + 1] = z->b_position;

    for (int i = x->n_keys - 1; i >= index; i--)
    {
        x->keys[i + 1] = x->keys[i];
        x->records[i + 1] = x->records[i];
    }

    x->keys[index] = y->keys[t];
    x->records[index] = y->records[t];
    x->n_keys++;

    disk_write(bt, x);
    disk_write(bt, y);
    disk_write(bt, z);

    node_destroy(y);
    node_destroy(z);
}

void insert_non_full(BTree *bt, Node *node, int key, int record)
{
    int i = node->n_keys - 1;

    if (node->is_leaf)
    {
        while (i >= 0 && key < node->keys[i])
        {
            node->keys[i + 1] = node->keys[i];
            node->records[i + 1] = node->records[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->records[i + 1] = record;
        node->n_keys++;
        disk_write(bt, node);
    }
    else
    {
        while (i >= 0 && key < node->keys[i])
        {
            i--;
        }
        i++;
        Node *child = disk_read(bt, node->children[i]);

        if (child->n_keys == bt->order - 1)
        {
            split_child(bt, node, i);
            if (key > node->keys[i])
            {
                i++;
            }

            node_destroy(child);
            child = disk_read(bt, node->children[i]);
        }

        insert_non_full(bt, child, key, record);

        node_destroy(child);
    }
}

bool btree_search(BTree *bt, int key)
{
    if (bt->root == NULL)
    {
        return false;
    }
    return search_node(bt, bt->root, key);
}

bool search_node(BTree *bt, Node *n, int key)
{
    int i = 0;
    while (i < n->n_keys && key > n->keys[i])
    {
        i++;
    }

    if (i < n->n_keys && key == n->keys[i])
    {
        return true;
    }

    if (n->is_leaf)
    {
        return false;
    }

    Node *child = disk_read(bt, n->children[i]);
    bool found = search_node(bt, child, key);

    node_destroy(child);

    return found;
}