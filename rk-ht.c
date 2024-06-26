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

#define RK_NODE_OFFSET(bp, p) ((rk_node_t *)p - (rk_node_t *)bp)
#define FREE_KEYS_OF_TABLE(table)                \
do {                                             \
    rk_node_t *node = table->next;               \
    while (node) {                               \
        if (node->key_len > sizeof(node->key)) { \
            free(node->key);                     \
        }                                        \
        node = node->next;                       \
    }                                            \
} while(0);

rk_ht_t *rk_ht_create(unsigned int table_size, rk_hs_func hs_func)
{
    /* the initial size must be greater than 1 */
    while (table_size % 2) {
        table_size++;
    }

    rk_ht_t *ht = malloc(sizeof(rk_ht_t));
    if (!ht) {
        return NULL;
    }
    /* allocate free nodes */
    ht->free = malloc(sizeof(rk_node_t) * table_size);
    if (!ht->free) {
        free(ht);
        return NULL;
    }
    /* alloc and initialize table */
    ht->table = calloc(table_size, sizeof(rk_table_t));
    if (!ht->table) {
        free(ht->free);
        free(ht);
        return NULL;
    }

    ht->mask = table_size - 1;
    ht->free_nodes = ht->free;
    /* initialize free nodes */
    for (unsigned int i = 0; i < ht->mask; i++) {
        ht->free_nodes[i].next = ht->free_nodes + (i + 1);
    }
    ht->free_nodes[ht->mask].next = NULL;

    ht->hs_func = hs_func;
    ht->size = 0;
    ht->table_size = table_size;

    return ht;
}

void rk_ht_destroy(rk_ht_t *ht)
{
    if (!ht) return;
    /* free all keys that may need to be free */
    for (unsigned int i = 0; i < ht->table_size; i++) {
        FREE_KEYS_OF_TABLE((ht->table + i));
    }
    free(ht->table);
    /* free all allocated nodes */
    free(ht->free);
    free(ht);
}

/**
 * @brief expand the hashtable
 * 
 * @param ht 
 * @param new_size 
 * @return 0 if success, -1 if fail
 */
static int rk_ht_expand(rk_ht_t *ht, unsigned int new_size)
{
    while (new_size % 2) {
        new_size++;
    }
    if (new_size <= ht->table_size) {
        /**
         * if new size is less than current size, we do not resize as it will discard element
         * if new size is less than table size, we do not resize as the costs is too high
         */
        return -1;
    }

    /* now we start to expand the hashtable */
    rk_table_t *new_table = calloc(new_size, sizeof(rk_table_t));
    if (!new_table) {
        return -1;
    }
    rk_node_t *old_free = ht->free; // perserve old free address
    /* note: the realloc will free old pointer if success */
    ht->free = realloc(ht->free, sizeof(rk_node_t) * new_size);
    if (!ht->free) {
        ht->free = old_free; // request fail and restore
        free(new_table);
        return -1;
    }

    ht->mask = new_size - 1;
    if (old_free != ht->free) {
        /* Unluckily we need update the [next] member */
        for (unsigned int i = 0; i < ht->table_size; i++) {
            if (ht->free[i].next) {
                ht->free[i].next = ht->free + RK_NODE_OFFSET(old_free, ht->free[i].next);
            }
        }
    }

    /* add new allocated nodes to free_nodes */
    for (unsigned int i = ht->table_size; i < ht->mask; i++) {
        ht->free[i].next = ht->free + i + 1;
    }
    if (ht->free_nodes) {
        ht->free[ht->mask].next = ht->free + RK_NODE_OFFSET(old_free, ht->free_nodes);
    } else {
        ht->free[ht->mask].next = NULL;
    }
    ht->free_nodes = ht->free + ht->table_size;
    /* now we start to rehash */
    for (unsigned int i = 0; i < ht->table_size; i++) {
        if (!ht->table[i].next) {
            continue;
        }
        rk_node_t *node = ht->free + RK_NODE_OFFSET(old_free, ht->table[i].next);
        rk_node_t *next;
        while (node) {
            rk_table_t *table = new_table + (node->hash & ht->mask);
            if (!table->prev) {
                table->prev = node;
            }
            next = node->next;
            node->next = table->next;
            table->next = node;

            node = next;
        }
    }
    free(ht->table);
    ht->table = new_table;
    ht->table_size = new_size;
    return 0;
}

/**
 * @brief determine if two keys are the same
 * 
 * @param key1 
 * @param key1_len 
 * @param key2 
 * @param key2_len 
 * @return 1 if same, 0 if not
 */
static inline int is_same_key(char *key1, char *key1_len, char *key2, char *key2_len)
{
    return (key1_len == key2_len &&
        ((key1_len <= sizeof(char *) && !memcmp(&key1, key2, key1_len)) ||
        (key1_len > sizeof(char *) && !memcmp(key1, key2, key1_len))));
}

int rk_ht_insert_s(rk_ht_t *ht, char *key, unsigned int key_len, void *value)
{
    /* allows inserting NULL value */
    if (!ht || !key || !key_len) return -1;

    int hash = ht->hs_func(key, key_len); /* preserve hash value */
    rk_table_t *table = ht->table + (hash & ht->mask);

    /* do not support multi map */
    rk_node_t *t_t = table->next;
    while (t_t) {
        /* already exist */
        if (is_same_key(t_t->key, t_t->key_len, key, key_len))
            return 0;
        t_t = t_t->next;
    }

    /* no free nodes */
    if (!ht->free_nodes) {
        if (rk_ht_expand(ht, ht->table_size << 1)) {
            return -1;
        } else {
            /* expand success */
            table = ht->table + (hash & ht->mask);
        }
    }

    /* now add value to table */
    rk_node_t *node = ht->free_nodes;
    ht->free_nodes = node->next;

    if (!table->prev)
        table->prev = node; // no any element in table
    node->next = table->next;
    table->next = node;
    /* fill value */
    if (key_len > sizeof(node->key)) {
        /* copy key as original key maybe modified or destroyed */
        node->key = malloc(key_len);
        memcpy(node->key, key, key_len);
    } else {
        memcpy(&node->key, key, key_len);
    }
    node->key_len = key_len;
    node->data = value;
    node->hash = hash;

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
        if (is_same_key(t_t->key, t_t->key_len, key, key_len)) {
            if (t_t->key_len > sizeof(t_t->key)) {
                free(t_t->key);
            }
            /* last node of link */
            if (!t_t->next) {
                if (prev_node == (rk_node_t *)table) {
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
    for (unsigned int i = 0; i < ht->table_size; i++) {
        rk_table_t *table = ht->table + i;
        if (table->prev) {
            /* free all keys that may need to be free */
            FREE_KEYS_OF_TABLE(table);
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
        if (is_same_key(t_t->key, t_t->key_len, key, key_len)) {
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
    for (unsigned int i = 0; i < ht->table_size; i++) {
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
