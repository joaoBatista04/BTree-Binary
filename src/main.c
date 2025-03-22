#include <stdio.h>
#include <stdlib.h>
#include "../include/btree.h"

int main(int argc, char *argv[])
{
    int order;
    int n_op;

    // Open entry's file
    FILE *fp = fopen(argv[1], "r");

    if (!fp)
    {
        perror("Couldn't open the entry file.\n");
        exit(1);
    }

    FILE *fp2 = fopen(argv[2], "w");

    if (!fp2)
    {
        perror("Couldn't create the exit file.\n");
        exit(1);
    }

    // Get the number of operations and the degree of a tree
    fscanf(fp, "%d\n", &order);
    fscanf(fp, "%d\n", &n_op);

    // Create the tree
    BTree *bt = btree_create("btree.bin", order);

    for (int i = 0; i < n_op; i++)
    {
        char op;
        int key;
        int record;

        // Get the operation and its params
        fscanf(fp, "%c %d", &op, &key);

        // Insertion operation
        if (op == 'I')
        {
            fscanf(fp, ", %d", &record);

            btree_insert(bt, key, record);
        }

        // Search operation
        if (op == 'B')
        {
            if (btree_search(bt, key))
            {
                fprintf(fp2, "O REGISTRO ESTA NA ARVORE!\n");
            }

            else
            {
                fprintf(fp2, "O REGISTRO NAO ESTA NA ARVORE!\n");
            }
        }

        // Delete operation
        if (op == 'R')
        {
            btree_delete(bt, key);
        }

        fscanf(fp, "\n");
    }

    // Print the tree in level-order
    fprintf(fp2, "\n-- ARVORE B\n");
    btree_level_order_print(bt, fp2);

    // Destroy memory allocated and close the file
    btree_destroy(bt);

    fclose(fp);
    fclose(fp2);

    // Destroy the binary file used in B-Tree
    remove("btree.bin");

    return 0;
}