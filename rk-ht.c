#include "rk-ht.h"

#include <stdlib.h>
#include <string.h>

unsigned int BKDRHash(char *str, unsigned int len)
{
    unsigned int seed = 131; /* 31 131 1313 13131 131313 etc.. */
    unsigned int hash = 0;
    unsigned int i = 0;

    for (i = 0; i < len; str++, i++) {
        hash = (hash * seed) + (*str);
    }

    return hash;
}
/* End Of BKDR Hash Function */

unsigned int BPHash(char *str, unsigned int len)
{
    unsigned int hash = 0;
    unsigned int i = 0;
    for (i = 0; i < len; str++, i++) {
        hash = hash << 7 ^ (*str);
    }

    return hash;
}
/* End Of BP Hash Function */

unsigned int DEKHash(char *str, unsigned int len)
{
    unsigned int hash = len;
    unsigned int i = 0;

    for (i = 0; i < len; str++, i++) {
        hash = ((hash << 5) ^ (hash >> 27)) ^ (*str);
    }
    return hash;
}
/* End Of DEK Hash Function */

unsigned int DJBHash(char *str, unsigned int len)
{
    unsigned int hash = 5381;
    unsigned int i = 0;

    for (i = 0; i < len; str++, i++) {
        hash = ((hash << 5) + hash) + (*str);
    }

    return hash;
}
/* End Of DJB Hash Function */

unsigned int ELFHash(char *str, unsigned int len)
{
    unsigned int hash = 0;
    unsigned int x = 0;
    unsigned int i = 0;

    for (i = 0; i < len; str++, i++) {
        hash = (hash << 4) + (*str);
        if ((x = hash & 0xF0000000L) != 0) {
            hash ^= (x >> 24);
        }
        hash &= ~x;
    }

    return hash;
}
/* End Of ELF Hash Function */

unsigned int FNVHash(char *str, unsigned int len)
{
    const unsigned int fnv_prime = 0x811C9DC5;
    unsigned int hash = 0;
    unsigned int i = 0;

    for (i = 0; i < len; str++, i++) {
        hash *= fnv_prime;
        hash ^= (*str);
    }

    return hash;
}
/* End Of FNV Hash Function */

unsigned int JSHash(char *str, unsigned int len)
{
    unsigned int hash = 1315423911;
    unsigned int i = 0;

    for (i = 0; i < len; str++, i++) {
        hash ^= ((hash << 5) + (*str) + (hash >> 2));
    }

    return hash;
}
/* End Of JS Hash Function */

unsigned int PJWHash(char *str, unsigned int len)
{
    const unsigned int BitsInUnsignedInt =
        (unsigned int)(sizeof(unsigned int) * 8);
    const unsigned int ThreeQuarters =
        (unsigned int)((BitsInUnsignedInt * 3) / 4);
    const unsigned int OneEighth = (unsigned int)(BitsInUnsignedInt / 8);
    const unsigned int HighBits = (unsigned int)(0xFFFFFFFF)
                               << (BitsInUnsignedInt - OneEighth);
    unsigned int hash = 0;
    unsigned int test = 0;
    unsigned int i = 0;

    for (i = 0; i < len; str++, i++) {
        hash = (hash << OneEighth) + (*str);

        if ((test = hash & HighBits) != 0) {
            hash = ((hash ^ (test >> ThreeQuarters)) & (~HighBits));
        }
    }

    return hash;
}
/* End Of  P. J. Weinberger Hash Function */

unsigned int RSHash(char *str, unsigned int len)
{
    unsigned int b = 378551;
    unsigned int a = 63689;
    unsigned int hash = 0;
    unsigned int i = 0;

    for (i = 0; i < len; str++, i++) {
        hash = hash * a + (*str);
        a = a * b;
    }

    return hash;
}
/* End Of RS Hash Function */

unsigned int SDBMHash(char *str, unsigned int len)
{
    unsigned int hash = 0;
    unsigned int i = 0;

    for (i = 0; i < len; str++, i++) {
        hash = (*str) + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}
/* End Of SDBM Hash Function */

rk_ht_t *rk_ht_create(unsigned int its, unsigned int mts, unsigned int max_size,
                      rk_hs_func hs_func)
{
    /* the initial size must be greater than 1 */
    if (mts < its || its < 2) return NULL;

    rk_ht_t *ht = malloc(sizeof(rk_ht_t));
    if (!ht) {
        return NULL;
    }
    /* allocate free nodes */
    ht->free = malloc(sizeof(rk_node_t) * max_size);
    if (!ht->free) {
        free(ht);
        return NULL;
    }
    /* alloc and initialize table */
    ht->table = calloc(its, sizeof(rk_table_t));
    if (!ht->table) {
        free(ht->free);
        free(ht);
        return NULL;
    }

    ht->free_nodes = ht->free;
    /* initialize free nodes */
    for (unsigned int i = 0; i < max_size - 1; i++) {
        ht->free_nodes[i].next = ht->free_nodes + (i + 1);
    }
    ht->free_nodes[max_size - 1].next = NULL;

    ht->cts = its;
    ht->mask = ht->cts - 1;
    ht->mts = mts;
    ht->hs_func = hs_func;
    ht->size = 0;
    ht->max_size = max_size;

    return ht;
}

void rk_ht_destroy(rk_ht_t *ht)
{
    if (!ht) return;
    free(ht->table);
    /* free all allocated nodes */
    free(ht->free);
    free(ht);
}

int rk_ht_insert_s(rk_ht_t *ht, char *key, unsigned int key_len, void *value)
{
    /* allows inserting NULL value */
    if (!ht || !key || !key_len) return -1;

    int idx = ht->hs_func(key, key_len) & ht->mask;
    rk_table_t *table = ht->table + idx;

    /* do not support multi map */
    rk_node_t *t_t = table->next;
    while (t_t) {
        /* already exist */
        if (t_t->data == value) return 0;
        t_t = t_t->next;
    }

    /* no free nodes */
    if (!ht->free_nodes) return -1;

    /* now add value to table[idx] */
    rk_node_t *node = ht->free_nodes;
    ht->free_nodes = node->next;

    if (!table->next)
        table->prev = node; // no any element in table
    node->next = table->next;
    table->next = node;
    /* fill value */
    node->key = key;
    node->key_len = key_len;
    node->data = value;

    ht->size++;
    return 1;
}

int rk_ht_erase_s(rk_ht_t *ht, char *key, unsigned int key_len)
{
    if (!ht || !key || !key_len) return -1;

    int idx = ht->hs_func(key, key_len) & ht->mask;
    rk_table_t *table = &ht->table[idx];

    /* The [next] member of table and node all are first position */
    rk_node_t *prev_node = (rk_node_t *)table;
    rk_node_t *t_t = table->next;
    while (t_t) {
        if (t_t->key_len == key_len && !memcmp(t_t->key, key, key_len)) {
            /* last node of link */
            if (!t_t->next) {
                if (prev_node == table) {
                    /* no any element now */
                    table->prev = NULL;
                } else {
                    table->prev = prev_node;
                }
            }
            prev_node->next = t_t->next;

            /* add to free list */
            t_t->next = ht->free_nodes;
            ht->free_nodes = t_t;
            ht->size--;
            return 1;
        }
        prev_node = t_t;
        t_t = t_t->next;
    }
    return 0;
}

int rk_ht_clear(rk_ht_t *ht)
{
    if (!ht) return -1;

    /* we can choose two strategy to clear */
    /* 1. find all allocated nodes and add to free list */
    /* 2. erase table and restore free list */
    for (unsigned int i = 0; i < ht->cts; i++) {
        rk_table_t *table = ht->table + i;
        if (table->prev) {
            table->prev->next = ht->free_nodes;
            ht->free_nodes = table->next;
            table->prev = table->next = NULL;
        }
    }
    ht->size = 0;
    return 0;
}

void *rk_ht_find_s(rk_ht_t *ht, char *key, unsigned int key_len)
{
    if (!ht || !key || !key_len) return NULL;

    int idx = ht->hs_func(key, key_len) & ht->mask;
    rk_table_t *table = ht->table + idx;

    rk_node_t *t_t = table->next;
    while (t_t) {
        if (t_t->key_len == key_len && !memcmp(t_t->key, key, key_len)) {
            return t_t->data;
        }
        t_t = t_t->next;
    }
    return NULL;
}

rk_node_t **rk_ht_create_iter(rk_ht_t *ht)
{
    if (!ht) return NULL;

    rk_node_t **nodes = malloc(sizeof(rk_node_t *) * ht->size);
    if (!nodes) return NULL;
    int idx = 0;
    for (unsigned int i = 0; i < ht->cts; i++) {
        rk_node_t *node = (ht->table + i)->next;
        while (node) {
            nodes[idx++] = node;
            node = node->next;
        }
    }
    return nodes;
}

inline void rk_ht_free_iter(rk_node_t **nodes)
{
    if (!nodes) return;
    free(nodes);
}
