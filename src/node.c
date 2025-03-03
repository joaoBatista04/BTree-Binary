#include <stdio.h>
#include <stdlib.h>
#include "../include/node.h"

struct Node
{
    int n_keys;
    bool is_leaf;
    int b_position;
    int *keys;
    int *records;
    int *children;
};

Node *node_create(int bt_n_nodes, int t, bool leaf)
{
    Node *node = (Node *)malloc(sizeof(Node));

    node->n_keys = 0;
    node->b_position = bt_n_nodes;
    node->is_leaf = leaf;

    node->keys = (int *)calloc(t, sizeof(int));
    node->records = (int *)calloc(t, sizeof(int));
    node->children = (int *)calloc(t, sizeof(int));

    for (int i = 0; i < t; i++)
    {
        node->children[i] = -1;
    }

    return node;
}

void node_destroy(Node *n)
{
    free(n->keys);
    free(n->records);
    free(n->children);
    free(n);
}

void disk_write(FILE *fp, Node *n, int t)
{
    size_t size = (n->b_position + 1) * (sizeof(int) * (3 * t) + sizeof(int) * 2 + sizeof(bool));

    fseek(fp, size, SEEK_SET);

    fwrite(&n->n_keys, sizeof(int), 1, fp);
    fwrite(&n->is_leaf, sizeof(bool), 1, fp);
    fwrite(&n->b_position, sizeof(int), 1, fp);
    fwrite(n->keys, sizeof(int), t, fp);
    fwrite(n->records, sizeof(int), t, fp);
    fwrite(n->children, sizeof(int), t, fp);

    fflush(fp);
}

Node *disk_read(FILE *fp, int pos, int t)
{
    size_t size = (pos + 1) * (sizeof(int) * (3 * t) + sizeof(int) * 2 + sizeof(bool));
    Node *n = (Node *)malloc(sizeof(Node));

    fseek(fp, size, SEEK_SET);

    fread(&n->n_keys, sizeof(int), 1, fp);
    fread(&n->is_leaf, sizeof(bool), 1, fp);
    fread(&n->b_position, sizeof(int), 1, fp);

    n->keys = (int *)malloc(sizeof(int) * (t));
    n->records = (int *)malloc(sizeof(int) * (t));
    n->children = (int *)malloc(sizeof(int) * t);

    fread(n->keys, sizeof(int), t, fp);
    fread(n->records, sizeof(int), t, fp);
    fread(n->children, sizeof(int), t, fp);

    for (int i = 0; i < t; i++)
    {
        printf("%d\n", n->children[i]);
    }

    return n;
}