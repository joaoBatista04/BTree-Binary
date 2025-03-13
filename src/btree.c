#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../include/queue.h"
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
    free(bt);
}

Node *btree_get_root(BTree *bt)
{
    return bt->root;
}

void btree_insert(BTree *bt, int key, int record)
{
    if (btree_search(bt, key))
    {
        return;
    }

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
            Node *y = disk_read(bt, bt->root->children[0]);
            split_child(bt, bt->root, y, 0);
            insert_non_full(bt, bt->root, key, record);
        }
        else
        {
            insert_non_full(bt, bt->root, key, record);
        }
    }
}

void split_child(BTree *bt, Node *x, Node *y, int i)
{
    Node *z = node_create(bt, y->is_leaf, bt->node_amount++);

    int t = (bt->order - 1) / 2;

    z->n_keys = (bt->order - 1) - t - 1;
    for (int j = 0; j < z->n_keys; j++)
    {
        z->keys[j] = y->keys[j + t + 1];
        z->records[j] = y->records[j + t + 1];
    }

    if (!y->is_leaf)
    {
        for (int j = 0; j <= z->n_keys; j++)
        {
            z->children[j] = y->children[j + t + 1];
        }
    }

    y->n_keys = t;

    for (int j = x->n_keys; j > i; j--)
    {
        x->children[j + 1] = x->children[j];
    }
    x->children[i + 1] = z->b_position;

    for (int j = x->n_keys - 1; j >= i; j--)
    {
        x->keys[j + 1] = x->keys[j];
        x->records[j + 1] = x->records[j];
    }

    x->keys[i] = y->keys[t];
    x->records[i] = y->records[t];
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
            Node *y = disk_read(bt, node->children[i]);
            split_child(bt, node, y, i);
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
    bool res = search_node(bt, child, key);

    node_destroy(child);

    return res;
}

void btree_delete(BTree *bt, int key)
{
    if (bt->root == NULL)
    {
        return;
    }

    delete_key(bt, bt->root, key);

    if (bt->root->n_keys == 0)
    {
        Node *old = bt->root;
        if (bt->root->is_leaf)
        {
            bt->root = NULL;
        }
        else
        {
            bt->root = disk_read(bt, bt->root->children[0]);
        }
        node_destroy(old);
    }
}

void delete_key(BTree *bt, Node *n, int key)
{
    int i = 0;
    while (i < n->n_keys && key > n->keys[i])
    {
        i++;
    }

    if (i < n->n_keys && n->keys[i] == key)
    {
        if (n->is_leaf)
        {
            remove_from_leaf(bt, n, i);
        }
        else
        {
            remove_from_non_leaf(bt, n, i);
        }
    }
    else
    {
        if (n->is_leaf)
        {
            return;
        }

        bool last_child = (i == n->n_keys);
        Node *child = disk_read(bt, n->children[i]);

        if (child->n_keys < (bt->order - 1) / 2)
        {
            Node *c = disk_read(bt, n->children[i]);
            fill(bt, n, c, i);
        }

        if (last_child && i > n->n_keys)
        {
            i--;
        }

        Node *child_aux = disk_read(bt, n->children[i]);
        delete_key(bt, child_aux, key);

        node_destroy(child_aux);
        node_destroy(child);
    }
}

void remove_from_leaf(BTree *bt, Node *n, int i)
{
    for (int j = i + 1; j < n->n_keys; j++)
    {
        n->keys[j - 1] = n->keys[j];
        n->records[j - 1] = n->records[j];
    }

    n->n_keys--;
    disk_write(bt, n);
}

void remove_from_non_leaf(BTree *bt, Node *n, int i)
{
    int key = n->keys[i];

    Node *left = disk_read(bt, n->children[i]);
    if (left->n_keys >= (bt->order - 1) / 2)
    {
        int pred_key = get_predecessor(bt, left);
        n->keys[i] = pred_key;
        disk_write(bt, n);
        delete_key(bt, left, pred_key);
    }
    else
    {
        Node *right = disk_read(bt, n->children[i + 1]);
        if (right->n_keys >= (bt->order - 1) / 2)
        {
            int succ_key = get_successor(bt, right);
            n->keys[i] = succ_key;
            disk_write(bt, n);
            delete_key(bt, right, succ_key);
        }
        else
        {
            merge(bt, n, i);
            delete_key(bt, left, key);
        }
        node_destroy(right);
    }
    node_destroy(left);
}

void fill(BTree *bt, Node *n, Node *child, int i)
{
    if (i != 0)
    {
        Node *left = disk_read(bt, n->children[i - 1]);
        if (left->n_keys >= (bt->order - 1) / 2)
        {
            Node *c = disk_read(bt, n->children[i]);
            borrow_from_previous(bt, c, n, i);
            node_destroy(left);
            return;
        }
        node_destroy(left);
    }

    if (i != n->n_keys)
    {
        Node *right = disk_read(bt, n->children[i + 1]);
        if (right->n_keys >= (bt->order - 1) / 2)
        {
            Node *c = disk_read(bt, n->children[i]);
            borrow_from_next(bt, n, c, i);
            node_destroy(right);
            return;
        }
        node_destroy(right);
    }

    if (i != n->n_keys)
    {
        merge(bt, n, i);
    }
    else
    {
        merge(bt, n, i - 1);
    }

    node_destroy(child);
}

int get_predecessor(BTree *bt, Node *n)
{
    while (!n->is_leaf)
    {
        n = disk_read(bt, n->children[n->n_keys]);
    }
    return n->keys[n->n_keys - 1];
}

int get_successor(BTree *bt, Node *n)
{
    while (!n->is_leaf)
    {
        n = disk_read(bt, n->children[0]);
    }
    return n->keys[0];
}

void borrow_from_previous(BTree *bt, Node *n, Node *child, int i)
{
    Node *left = disk_read(bt, n->children[i - 1]);

    for (int j = child->n_keys - 1; j >= 0; j--)
    {
        child->keys[j + 1] = child->keys[j];
        child->records[j + 1] = child->records[j];
    }

    if (!child->is_leaf)
    {
        for (int j = child->n_keys; j >= 0; j--)
        {
            child->children[j + 1] = child->children[j];
        }
    }

    child->keys[0] = n->keys[i - 1];
    child->records[0] = n->records[i - 1];

    n->keys[i - 1] = left->keys[left->n_keys - 1];
    n->records[i - 1] = left->records[left->n_keys - 1];

    if (!child->is_leaf)
    {
        child->children[0] = left->children[left->n_keys];
    }

    child->n_keys++;
    left->n_keys--;

    disk_write(bt, n);
    disk_write(bt, child);
    disk_write(bt, left);

    node_destroy(left);
}

void borrow_from_next(BTree *bt, Node *n, Node *child, int i)
{
    Node *right = disk_read(bt, n->children[i + 1]);

    child->keys[child->n_keys] = n->keys[i];
    child->records[child->n_keys] = n->records[i];

    if (!child->is_leaf)
    {
        child->children[child->n_keys + 1] = right->children[0];
    }

    n->keys[i] = right->keys[0];
    n->records[i] = right->records[0];

    for (int j = 1; j < right->n_keys; j++)
    {
        right->keys[j - 1] = right->keys[j];
        right->records[j - 1] = right->records[j];
    }

    if (!right->is_leaf)
    {
        for (int j = 1; j <= right->n_keys; j++)
        {
            right->children[j - 1] = right->children[j];
        }
    }

    child->n_keys++;
    right->n_keys--;

    disk_write(bt, n);
    disk_write(bt, child);
    disk_write(bt, right);

    node_destroy(right);
}

void merge(BTree *bt, Node *n, int i)
{
    Node *child = disk_read(bt, n->children[i]);
    Node *next_child = disk_read(bt, n->children[i + 1]);

    child->keys[child->n_keys] = n->keys[i];
    child->records[child->n_keys] = n->records[i];

    for (int j = 0; j < next_child->n_keys; j++)
    {
        child->keys[child->n_keys + 1 + j] = next_child->keys[j];
        child->records[child->n_keys + 1 + j] = next_child->records[j];
    }

    if (!child->is_leaf)
    {
        for (int j = 0; j <= next_child->n_keys; j++)
        {
            child->children[child->n_keys + 1 + j] = next_child->children[j];
        }
    }

    child->n_keys += next_child->n_keys + 1;

    for (int j = i + 1; j < n->n_keys; j++)
    {
        n->keys[j - 1] = n->keys[j];
        n->records[j - 1] = n->records[j];
    }
    for (int j = i + 2; j <= n->n_keys; j++)
    {
        n->children[j - 1] = n->children[j];
    }

    n->n_keys--;

    disk_write(bt, n);
    disk_write(bt, child);
    disk_write(bt, next_child);

    node_destroy(child);
    node_destroy(next_child);
}

void btree_level_order_print(BTree *bt)
{
    Queue *q = queue_create();
    queue_enqueue(q, bt->root);

    while (!queue_is_empty(q))
    {
        int level_size = queue_get_size(q);

        for (int i = 0; i < level_size; i++)
        {
            Node *curr = queue_dequeue(q);

            if (curr->n_keys > 0)
            {
                printf("[");
                for (int j = 0; j < curr->n_keys; j++)
                {
                    printf("key: %d, ", curr->keys[j]);
                }
                printf("]");
            }

            if (!curr->is_leaf)
            {
                for (int j = 0; j <= curr->n_keys; j++)
                {
                    Node *child = disk_read(bt, curr->children[j]);
                    queue_enqueue(q, child);
                }
            }

            if (curr != bt->root)
            {
                node_destroy(curr);
            }
        }
        printf("\n");
    }

    if (bt->root)
    {
        node_destroy(bt->root);
    }

    queue_destroy(q);
}