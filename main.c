#include <stdio.h>

#include "rk-ht.h"

int main()
{
    rk_ht_t *ht = rk_ht_create(8, BPHash);

    int a = 4;
    int b = 8;
    int c = 28;
    int res = 0;

    res = rk_ht_insert(ht, &a, 4);
    res = rk_ht_insert(ht, &a, 100);
    res = rk_ht_insert(ht, &b, 8);
    res = rk_ht_insert(ht, &c, 28);
    res = rk_ht_insert_s(ht, "hello", 5, 5);
    res = rk_ht_insert_s(ht, "world", 5, 5);
    res = rk_ht_insert_s(ht, "aaaaa", 5, 1);
    res = rk_ht_insert_s(ht, "bbbbb", 5, 2);
    res = rk_ht_insert_s(ht, "ccccc", 5, 3);
    res = rk_ht_insert_s(ht, "ddddd", 5, 4);
    res = rk_ht_insert_s(ht, "eeeee", 5, 5);
    res = rk_ht_insert_s(ht, "fffff21341234", 5, 6);
    res = rk_ht_find(ht, &a);
    printf("%d\n", res);
    res = rk_ht_find(ht, &b);
    printf("%d\n", res);
    res = rk_ht_find(ht, &c);
    printf("%d\n", res);

    printf("start iter\n");
    rk_node_t **iter = rk_ht_create_iter(ht);
    for (int i = 0; i < ht->size; i++) {
        printf("%d\n", iter[i]->data);
    }
    rk_ht_free_iter(iter);
    printf("finish iter\n");

    rk_ht_erase(ht, &a);
    res = rk_ht_find(ht, &a);
    printf("%d\n", res);
    res = rk_ht_find(ht, &b);
    printf("%d\n", res);
    rk_ht_clear(ht);
    res = rk_ht_find(ht, &b);
    printf("%d\n", res);
    res = rk_ht_find(ht, &c);
    printf("%d\n", res);

    rk_ht_destroy(ht);

    return 0;
}