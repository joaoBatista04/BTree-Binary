#include <stdio.h>
#include <stdlib.h>
#include "../include/btree.h"
#include "../include/node.h"

int main(int argc, char *argv[])
{
    Node *c1 = node_create(0, 4, true);
    Node *c2 = node_create(1, 4, true);
    Node *c3 = node_create(5, 4, true);
    Node *c4 = node_create(8, 4, true);

    FILE *fp = fopen("btree.bin", "w+b");

    if (!fp)
    {
        exit(1);
    }

    disk_write(fp, c1, 4);
    disk_write(fp, c2, 4);
    disk_write(fp, c3, 4);
    disk_write(fp, c4, 4);

    Node *c5 = disk_read(fp, 8, 4);

    node_destroy(c1);
    node_destroy(c2);
    node_destroy(c3);
    node_destroy(c4);
    node_destroy(c5);

    fclose(fp);
    return 0;
}