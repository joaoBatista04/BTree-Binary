#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../include/queue.h"
#include "../include/btree.h"

struct Node
{
    int n_keys;     // Number of nodes
    bool is_leaf;   // Flag to leaves
    int b_position; // Node's position in the binary file
    int *keys;      // Set of keys
    int *records;   // Set of values associated to the keys
    int *children;  // Index of node's children
};

struct BTree
{
    int order;       // Min order of the three
    Node *root;      // Tree's root
    int node_amount; // Amount of nodes registred in the tree
    FILE *bfile;     // The file where the data will be write/read
};

/**
 * @brief Create a node and allocate memory for it
 *
 * @param BTree* bt
 * @param bool is_leaf
 * @param int pos
 * @return Node*
 */
Node *node_create(BTree *bt, bool is_leaf, int pos)
{
    Node *node = (Node *)malloc(sizeof(Node));

    // Set initial params
    node->n_keys = 0;
    node->is_leaf = is_leaf;
    node->b_position = pos;

    // Allocate memory to the vectors of keys, records and children
    node->keys = (int *)calloc((bt->order - 1), sizeof(int));
    node->records = (int *)calloc((bt->order - 1), sizeof(int));
    node->children = (int *)calloc(bt->order, sizeof(int));

    // Initialize them with -1
    for (int i = 0; i < bt->order - 1; i++)
    {
        node->keys[i] = -1;
        node->records[i] = -1;
        node->children[i] = -1;
    }
    node->children[bt->order - 1] = -1;

    return node;
}

/**
 * @brief Get the size of the node
 *
 * @param BTree* bt
 * @return size_t
 */
size_t node_size(BTree *bt)
{
    return sizeof(int) + sizeof(bool) + sizeof(int) + sizeof(int) * (bt->order - 1) + sizeof(int) * (bt->order - 1) + sizeof(int) * bt->order;
}

/**
 * @brief Destroy a node, freeing the memory allocated for it
 *
 * @param n
 */
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

/**
 * @brief Function to write the params of a node to the binary file
 *
 * @param BTree* btree
 * @param Node* n
 */
void disk_write(BTree *bt, Node *n)
{
    // Get the position of the node by calculating its position in the tree and its size
    fseek(bt->bfile, n->b_position * node_size(bt), SEEK_SET);

    // Write simple params to the binary file
    fwrite(&n->n_keys, sizeof(int), 1, bt->bfile);
    fwrite(&n->is_leaf, sizeof(bool), 1, bt->bfile);
    fwrite(&n->b_position, sizeof(int), 1, bt->bfile);

    // Write the vectors of keys, records and children to the file
    fwrite(n->keys, sizeof(int), bt->order - 1, bt->bfile);
    fwrite(n->records, sizeof(int), bt->order - 1, bt->bfile);
    fwrite(n->children, sizeof(int), bt->order, bt->bfile);

    fflush(bt->bfile);
}

/**
 * @brief Read a node from binary file
 *
 * @param BTree* btree
 * @param int pos
 * @return Node*
 */
Node *disk_read(BTree *bt, int pos)
{
    Node *n = (Node *)malloc(sizeof(Node));

    // Get the position of the node by calculating its position in the tree and its size
    fseek(bt->bfile, pos * node_size(bt), SEEK_SET);

    // Read simple params from binary file
    fread(&n->n_keys, sizeof(int), 1, bt->bfile);
    fread(&n->is_leaf, sizeof(bool), 1, bt->bfile);
    fread(&n->b_position, sizeof(int), 1, bt->bfile);

    // Allocate memory to key, records and children's vectors
    n->keys = (int *)malloc((bt->order - 1) * sizeof(int));
    n->records = (int *)malloc((bt->order - 1) * sizeof(int));
    n->children = (int *)malloc(bt->order * sizeof(int));

    // Read the vectors of keys, records and children from the file
    fread(n->keys, sizeof(int), bt->order - 1, bt->bfile);
    fread(n->records, sizeof(int), bt->order - 1, bt->bfile);
    fread(n->children, sizeof(int), bt->order, bt->bfile);

    return n;
}

/**
 * @brief Create a B-Tree and allocate memory to it
 *
 * @param char* path
 * @param int order
 * @return BTree*
 */
BTree *btree_create(char *path, int order)
{
    BTree *bt = (BTree *)malloc(sizeof(BTree));

    // Set initial params
    bt->order = order;
    bt->root = NULL;
    bt->node_amount = 0;

    // Create binary file to tree
    FILE *fp = fopen("btree.bin", "w+b");
    if (!fp)
    {
        perror("The system couldn't create the binary file.\n");
        exit(1);
    }

    bt->bfile = fp;

    return bt;
}

/**
 * @brief Destroy a B-Tree and free memory allocated to it
 *
 * @param BTree* bt
 */
void btree_destroy(BTree *bt)
{
    // Close binary file
    fclose(bt->bfile);
    free(bt);
}

/**
 * @brief Returns a pointer to root's node
 *
 * @param BTree* bt
 * @return Node*
 */
Node *btree_get_root(BTree *bt)
{
    return bt->root;
}

/**
 * @brief Insert a value to the tree
 *
 * @param BTree* bt
 * @param int key
 * @param int record
 */
void btree_insert(BTree *bt, int key, int record)
{
    // Check if the key already exists in the tree
    if (btree_search(bt, key))
    {
        return;
    }

    // If the tree is empty, create a new root node
    if (!bt->root)
    {
        bt->root = node_create(bt, true, bt->node_amount++);
        bt->root->keys[0] = key;
        bt->root->records[0] = record;
        bt->root->n_keys = 1;
        disk_write(bt, bt->root);
    }
    else
    {
        // If the root is full, split it and grow the tree height
        if (bt->root->n_keys == bt->order - 1)
        {
            // Create the new node and define its params
            Node *new_root = node_create(bt, false, bt->node_amount++);
            new_root->children[0] = bt->root->b_position;

            node_destroy(bt->root);
            bt->root = new_root;
            Node *y = disk_read(bt, bt->root->children[0]);

            // Split child and insert to the tree (preemptive insertion)
            split_child(bt, bt->root, y, 0);
            insert_non_full(bt, bt->root, key, record);
        }
        else
        {
            // If node isn't full, insert into it
            insert_non_full(bt, bt->root, key, record);
        }
    }
}

/**
 * @brief Function to split a node
 *
 * @param Btree* bt
 * @param Node* x
 * @param Node* y
 * @param i
 */
void split_child(BTree *bt, Node *x, Node *y, int i)
{
    Node *z = node_create(bt, y->is_leaf, bt->node_amount++);

    // Get the minimum order of the tree
    int t = (bt->order - 1) / 2;

    // Copy the second half of y's keys and records to z
    z->n_keys = (bt->order - 1) - t - 1;
    for (int j = 0; j < z->n_keys; j++)
    {
        z->keys[j] = y->keys[j + t + 1];
        z->records[j] = y->records[j + t + 1];
    }

    // If y is not a leaf, copy its children to z
    if (!y->is_leaf)
    {
        for (int j = 0; j <= z->n_keys; j++)
            z->children[j] = y->children[j + t + 1];
    }

    y->n_keys = t;

    // Shift x's children to make space for z
    for (int j = x->n_keys; j > i; j--)
    {
        x->children[j + 1] = x->children[j];
    }

    x->children[i + 1] = z->b_position;

    // Shift x's keys and records to make space in y
    for (int j = x->n_keys - 1; j >= i; j--)
    {
        x->keys[j + 1] = x->keys[j];
        x->records[j + 1] = x->records[j];
    }

    // Move the median key from y to x
    x->keys[i] = y->keys[t];
    x->records[i] = y->records[t];
    x->n_keys++;

    // Write nodes in the binary file
    disk_write(bt, x);
    disk_write(bt, y);
    disk_write(bt, z);

    node_destroy(y);
    node_destroy(z);
}

/**
 * @brief Insert a key in the tree if node isn't full
 *
 * @param BTree* bt
 * @param Node* node
 * @param int key
 * @param int record
 */
void insert_non_full(BTree *bt, Node *node, int key, int record)
{
    int i = node->n_keys - 1;

    // If the node is a leaf, insert the key directly
    if (node->is_leaf)
    {
        // Shift keys and records to make space for the new key
        while (i >= 0 && key < node->keys[i])
        {
            node->keys[i + 1] = node->keys[i];
            node->records[i + 1] = node->records[i];
            i--;
        }

        // Insert the key and record
        node->keys[i + 1] = key;
        node->records[i + 1] = record;
        node->n_keys++;
        disk_write(bt, node);
    }
    else
    {
        // Find the appropriate child for insertion
        while (i >= 0 && key < node->keys[i])
        {
            i--;
        }
        i++;
        Node *child = disk_read(bt, node->children[i]);

        // If the child is full, split it
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

        // Recursively insert the key into the child
        insert_non_full(bt, child, key, record);

        // Free the memory of the child
        node_destroy(child);
    }
}

/**
 * @brief Search a node in B-Tree
 *
 * @param BTree* bt
 * @param int key
 * @return true
 * @return false
 */
bool btree_search(BTree *bt, int key)
{
    if (bt->root == NULL)
    {
        return false;
    }
    return search_node(bt, bt->root, key);
}

/**
 * @brief Search a node by searching a key
 *
 * @param BTree* bt
 * @param Node* n
 * @param int key
 * @return true
 * @return false
 */
bool search_node(BTree *bt, Node *n, int key)
{
    int i = 0;
    // Find the position of the key in the node
    while (i < n->n_keys && key > n->keys[i])
    {
        i++;
    }

    // If the key is found in the node, return true
    if (i < n->n_keys && key == n->keys[i])
    {
        return true;
    }

    // If the node is a leaf and the key is not found, return false
    if (n->is_leaf)
    {
        return false;
    }

    // Recursively search in the appropriate child node
    Node *child = disk_read(bt, n->children[i]);
    bool res = search_node(bt, child, key);

    // Free the memory of the child node
    node_destroy(child);

    return res;
}

/**
 * @brief Delete a key and the value associated to the key from B-Tree
 *
 * @param Btree* bt
 * @param int key
 */
void btree_delete(BTree *bt, int key)
{
    if (bt->root == NULL)
    {
        return;
    }

    // Remove a key, recursively, from node
    remove_from_node(bt, bt->root, key);

    // Update root if root is empty
    if (bt->root->n_keys == 0)
    {
        if (bt->root->is_leaf)
        {
            // If root is a leaf, let them null
            node_destroy(bt->root);
            bt->root = NULL;
        }
        else
        {
            // If root isn't a leaf, turn the first child in root
            int child_pos = bt->root->children[0];
            node_destroy(bt->root);
            bt->root = disk_read(bt, child_pos);
        }
    }
}

/**
 * @brief Auxiliary function to find a key's index in the node
 *
 * @param Node* node
 * @param int key
 * @return int
 */
int find_key_index(Node *node, int key)
{
    int i = 0;

    while (i < node->n_keys && node->keys[i] < key)
    {
        ++i;
    }

    return i;
}

/**
 * @brief Recursive function to delete a key from the tree
 *
 * @param BTree* bt
 * @param Node* n
 * @param key
 */
void remove_from_node(BTree *bt, Node *n, int key)
{
    // Find the index of a key in the node
    int i = find_key_index(n, key);

    // Verify if the key is present in the node
    if (i < n->n_keys && n->keys[i] == key)
    {
        if (n->is_leaf)
        {
            // Case 1: the node is a leaf and has the minimum amount of keys
            remove_from_leaf(bt, n, i);
        }
        else
        {
            // Case 2: the node isn't a leaf
            remove_from_non_leaf(bt, n, i);
        }
    }
    else
    {
        if (n->is_leaf)
        {
            node_destroy(n);
            return;
        }

        // Flag to verify if last node was found
        bool is_last_child = (i == n->n_keys);

        // Read ci children from binary file
        Node *child = disk_read(bt, n->children[i]);

        // Verify if child node has the minimum amount of keys
        int min_keys = (bt->order - 1) / 2;
        if (child->n_keys <= min_keys)
        {
            // Fill the node
            fill_node(bt, n, i);

            node_destroy(child);

            // If was the last child and a merge occured, an update is made
            if (is_last_child && i > n->n_keys)
            {
                child = disk_read(bt, n->children[i - 1]);
            }
            else
            {
                child = disk_read(bt, n->children[i]);
            }
        }

        // Remove recursively the key from child
        remove_from_node(bt, child, key);
    }
}

/**
 * @brief Remove a key if the node is a leaf
 *
 * @param BTree* bt
 * @param Node* n
 * @param int index
 */
void remove_from_leaf(BTree *bt, Node *n, int index)
{
    // Shift all keys and records to the right of the key to be removed
    for (int i = index + 1; i < n->n_keys; ++i)
    {
        n->keys[i - 1] = n->keys[i];
        n->records[i - 1] = n->records[i];
    }

    n->n_keys--;

    // Write the node, updating it in the file
    disk_write(bt, n);
}

/**
 * @brief Remove a key if the node is a leaf
 *
 * @param bt
 * @param n
 * @param index
 */
void remove_from_non_leaf(BTree *bt, Node *n, int index)
{
    int key = n->keys[index];

    // Case 2a: if node doesn't have the minimum of keys, find the predecessor of the key in the node y
    Node *child_left = disk_read(bt, n->children[index]);

    int min_keys = (bt->order - 1) / 2;
    if (child_left->n_keys > min_keys)
    {
        int pred_record = get_predecessor(bt, n, index);
        n->records[index] = pred_record;

        disk_write(bt, n);
        node_destroy(child_left);
        return;
    }

    // Case 2b: if node y doesn't have the minimum amount of node, get the successor of a key in node z
    Node *child_right = disk_read(bt, n->children[index + 1]);
    if (child_right->n_keys > min_keys)
    {
        int succ_record = get_successor(bt, n, index);
        n->records[index] = succ_record;

        disk_write(bt, n);
        node_destroy(child_left);
        node_destroy(child_right);
        return;
    }

    // Case 2c: if none of children nodes have the minimum amount of keys, merge them and remove the key
    merge_nodes(bt, n, index);
    node_destroy(child_right);

    // Remove the key from new node
    remove_from_node(bt, child_left, key);
}

/**
 * @brief Get the predecessor of a key in left's node
 *
 * @param BTree* bt
 * @param Node* n
 * @param index
 * @return int
 */
int get_predecessor(BTree *bt, Node *n, int i)
{
    // Get the left's child
    Node *current = disk_read(bt, n->children[i]);

    // Go to the rightmost node of this subtree
    while (!current->is_leaf)
    {
        Node *temp = disk_read(bt, current->children[current->n_keys]);
        node_destroy(current);
        current = temp;
    }

    // Predecessor is the last key of this node
    int pred_key = current->keys[current->n_keys - 1];
    int pred_record = current->records[current->n_keys - 1];

    // Update key in the original node
    n->keys[i] = pred_key;

    node_destroy(current);
    return pred_record;
}

/**
 * @brief Get the successor of a key in right's node
 *
 * @param BTree* bt
 * @param Node* n
 * @param index
 * @return int
 */
int get_successor(BTree *bt, Node *n, int i)
{
    // Get the right's child
    Node *current = disk_read(bt, n->children[i + 1]);

    // Go to the leftmost node of this subtree
    while (!current->is_leaf)
    {
        Node *temp = disk_read(bt, current->children[0]);
        node_destroy(current);
        current = temp;
    }

    // Successor is the first key of this node
    int succ_key = current->keys[0];
    int succ_record = current->records[0];

    // Update key in the original node
    n->keys[i] = succ_key;

    node_destroy(current);
    return succ_record;
}

/**
 * @brief Merge children from right and left
 *
 * @param bt
 * @param n
 * @param index
 */
void merge_nodes(BTree *bt, Node *n, int index)
{
    Node *child = disk_read(bt, n->children[index]);
    Node *sibling = disk_read(bt, n->children[index + 1]);

    int min_keys = (bt->order - 1) / 2;

    // Copy the keys from parents to the child
    child->keys[min_keys] = n->keys[index];
    child->records[min_keys] = n->records[index];

    // Copy all keys and registers of a sibling to the node
    for (int i = 0; i < sibling->n_keys; ++i)
    {
        child->keys[min_keys + 1 + i] = sibling->keys[i];
        child->records[min_keys + 1 + i] = sibling->records[i];
    }

    // If node isn't leaf, copy the references to children
    if (!child->is_leaf)
    {
        for (int i = 0; i <= sibling->n_keys; ++i)
        {
            child->children[min_keys + 1 + i] = sibling->children[i];
        }
    }

    // Move keys in parent node to fill space of removed key
    for (int i = index + 1; i < n->n_keys; ++i)
    {
        n->keys[i - 1] = n->keys[i];
        n->records[i - 1] = n->records[i];
    }

    // Moves references to children in the parent node
    for (int i = index + 2; i <= n->n_keys; ++i)
    {
        n->children[i - 1] = n->children[i];
    }

    // Update amount of keys
    child->n_keys += sibling->n_keys + 1;
    n->n_keys--;

    // Write nodes in the binary file
    disk_write(bt, n);
    disk_write(bt, child);

    node_destroy(child);
    node_destroy(sibling);
}

/**
 * @brief Fill the node to ensure it has the minimum number of keys
 *
 * @param BTree* bt
 * @param Node* n
 * @param int index
 */
void fill_node(BTree *bt, Node *n, int index)
{
    int min_keys = (bt->order - 1) / 2;

    // Try to borrow from the brother on the left
    if (index != 0)
    {
        Node *sibling = disk_read(bt, n->children[index - 1]);
        if (sibling->n_keys > min_keys)
        {
            borrow_from_prev(bt, n, index);
            node_destroy(sibling);
            return;
        }
        node_destroy(sibling);
    }

    // Try to borrow from the brother on the right
    if (index != n->n_keys)
    {
        Node *sibling = disk_read(bt, n->children[index + 1]);
        if (sibling->n_keys > min_keys)
        {
            borrow_from_next(bt, n, index);
            node_destroy(sibling);
            return;
        }
        node_destroy(sibling);
    }

    // If it was not possible to borrow, do the merge
    if (index != n->n_keys)
    {
        // Merge with right node
        merge_nodes(bt, n, index);
    }
    else
    {
        // Merge with left node
        merge_nodes(bt, n, index - 1);
    }
}

/**
 * @brief Get a key from previous node
 *
 * @param BTree* bt
 * @param Node* n
 * @param index
 */
void borrow_from_prev(BTree *bt, Node *n, int index)
{
    Node *child = disk_read(bt, n->children[index]);
    Node *sibling = disk_read(bt, n->children[index - 1]);

    // Move the keys in the child
    for (int i = child->n_keys - 1; i >= 0; --i)
    {
        child->keys[i + 1] = child->keys[i];
        child->records[i + 1] = child->records[i];
    }

    // If it is not a leaf, it also moves the references to children
    if (!child->is_leaf)
    {
        for (int i = child->n_keys; i >= 0; --i)
        {
            child->children[i + 1] = child->children[i];
        }
    }

    // Set the parent's key as the child's first key
    child->keys[0] = n->keys[index - 1];
    child->records[0] = n->records[index - 1];

    // If it is not a leaf, the last child of the sibling becomes the first child.
    if (!child->is_leaf)
    {
        child->children[0] = sibling->children[sibling->n_keys];
    }

    // The sibling's last key goes to the parent
    n->keys[index - 1] = sibling->keys[sibling->n_keys - 1];
    n->records[index - 1] = sibling->records[sibling->n_keys - 1];

    // Update amount of keys
    child->n_keys++;
    sibling->n_keys--;

    // Write nodes in the binary file
    disk_write(bt, n);
    disk_write(bt, child);
    disk_write(bt, sibling);

    node_destroy(child);
    node_destroy(sibling);
}

/**
 * @brief Get a key from next node
 *
 * @param BTree* bt
 * @param Node* n
 * @param index
 */
void borrow_from_next(BTree *bt, Node *n, int index)
{
    Node *child = disk_read(bt, n->children[index]);
    Node *sibling = disk_read(bt, n->children[index + 1]);

    // The parent's key goes to the child's end
    child->keys[child->n_keys] = n->keys[index];
    child->records[child->n_keys] = n->records[index];

    // If it is not a leaf, the first child of the sibling becomes the last child.
    if (!child->is_leaf)
    {
        child->children[child->n_keys + 1] = sibling->children[0];
    }

    // The sibling's first key goes to the parent
    n->keys[index] = sibling->keys[0];
    n->records[index] = sibling->records[0];

    // Move the keys to the sibling
    for (int i = 1; i < sibling->n_keys; ++i)
    {
        sibling->keys[i - 1] = sibling->keys[i];
        sibling->records[i - 1] = sibling->records[i];
    }

    // If it is not a leaf, it also moves the references to children
    if (!sibling->is_leaf)
    {
        for (int i = 1; i <= sibling->n_keys; ++i)
        {
            sibling->children[i - 1] = sibling->children[i];
        }
    }

    // Update amount of keys
    child->n_keys++;
    sibling->n_keys--;

    // Write nodes in the binary file
    disk_write(bt, n);
    disk_write(bt, child);
    disk_write(bt, sibling);

    node_destroy(child);
    node_destroy(sibling);
}

/**
 * @brief Print B-Tree in level-order
 *
 * @param bt
 */
void btree_level_order_print(BTree *bt, FILE *fp)
{
    // Create a queue to make level-order traversal
    Queue *q = queue_create();

    // Enqueue root
    queue_enqueue(q, bt->root);

    while (!queue_is_empty(q))
    {
        // Get the number of nodes at the current level
        int level_size = queue_get_size(q);

        // Process each node at the current level
        for (int i = 0; i < level_size; i++)
        {
            // Dequeue the current node
            Node *curr = queue_dequeue(q);

            if (curr->n_keys > 0)
            {
                fprintf(fp, "[");
                for (int j = 0; j < curr->n_keys; j++)
                    fprintf(fp, "key: %d, ", curr->keys[j]);
                fprintf(fp, "]");
            }

            // If the node is not a leaf, enqueue its children for the next level
            if (!curr->is_leaf)
            {
                for (int j = 0; j <= curr->n_keys; j++)
                {
                    Node *child = disk_read(bt, curr->children[j]);
                    queue_enqueue(q, child);
                }
            }

            if (curr != bt->root)
                node_destroy(curr);
        }
        fprintf(fp, "\n");
    }

    // Destroy the root node after traversal is complete
    if (bt->root)
    {
        node_destroy(bt->root);
    }

    // Destroy the queue
    queue_destroy(q);
}