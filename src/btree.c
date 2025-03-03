#include <stdio.h>
#include <stdlib.h>
#include "../include/node.h"
#include "../include/btree.h"

struct BTree
{
    int order;
    Node *root;
    int node_amount;
    FILE *bfile;
};