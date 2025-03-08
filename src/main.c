#include <stdio.h>
#include <stdlib.h>
#include "../include/btree.h"

int main(int argc, char *argv[])
{
    int order = 5;
    BTree *btree = btree_create("btree.bin", order);

    btree_insert(btree, 20, 20);
    btree_insert(btree, 75, 75);
    btree_insert(btree, 77, 77);
    btree_insert(btree, 78, 78);
    btree_insert(btree, 55, 55);
    btree_insert(btree, 62, 62);
    btree_insert(btree, 51, 51);
    btree_insert(btree, 40, 40);
    btree_insert(btree, 60, 60);
    btree_insert(btree, 45, 45);
    btree_insert(btree, 15, 15);
    btree_insert(btree, 2, 2);
    btree_insert(btree, 42, 42);
    btree_insert(btree, 10, 10);
    btree_insert(btree, 13, 13);
    btree_insert(btree, 100, 100);
    btree_insert(btree, 23, 23);
    btree_insert(btree, 44, 44);
    btree_insert(btree, 11, 11);
    btree_insert(btree, 9, 9);
    btree_insert(btree, 18, 18);
    btree_insert(btree, 5, 5);
    btree_insert(btree, 75, 75);

    for (int i = 0; i < 101; i++)
    {
        if (btree_search(btree, i))
        {
            printf("Chave %d encontrada!\n", i);
        }
        else
        {
            printf("Chave %d não encontrada!\n", i);
        }
    }

    btree_destroy(btree);

    return 0;
}