#include <stdio.h>

#include "rk-ht.h"

int main()
{
    rk_ht_t *ht = rk_ht_create(8, 8, 1024, BPHash);

    int a = 4;
    int b = 8;
    int c = 28;
    int res = 0;

    res = rk_ht_insert(ht, &a, 4);
    res = rk_ht_insert(ht, &b, 8);
    res = rk_ht_insert(ht, &c, 28);
    res = rk_ht_find(ht, &a);
    printf("%d\n", res);
    res = rk_ht_find(ht, &b);
    printf("%d\n", res);
    res = rk_ht_find(ht, &c);
    printf("%d\n", res);
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