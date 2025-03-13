#include <stdio.h>
#include <stdlib.h>
#include "../include/btree.h"

int main(int argc, char *argv[])
{
    int order;
    int n_op;

    FILE *fp = fopen(argv[1], "r");

    if (!fp)
    {
        perror("Couldn't open the entry file.\n");
        exit(1);
    }

    fscanf(fp, "%d\n", &order);
    fscanf(fp, "%d\n", &n_op);

    BTree *bt = btree_create("btree.bin", order);

    for (int i = 0; i < n_op; i++)
    {
        char op;
        int key;
        int record;
        fscanf(fp, "%c %d", &op, &key);

        if (op == 'I')
        {
            fscanf(fp, ", %d", &record);

            btree_insert(bt, key, record);
        }

        if (op == 'B')
        {
            if (btree_search(bt, key))
            {
                printf("O REGISTRO ESTA NA ARVORE!\n");
            }

            else
            {
                printf("O REGISTRO NAO ESTA NA ARVORE!\n");
            }
        }

        if (op == 'R')
        {
            btree_delete(bt, key);
        }

        fscanf(fp, "\n");
    }

    printf("\n-- ARVORE B\n");
    btree_level_order_print(bt);

    btree_destroy(bt);

    fclose(fp);

    remove("btree.bin");

    return 0;
}