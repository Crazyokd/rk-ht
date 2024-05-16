#ifndef _RK_HT_
#define _RK_HT_

typedef unsigned int (*rk_hs_func)(char *, unsigned int);
/* hash algorithm */
unsigned int BKDRHash(char *str, unsigned int len);
unsigned int BPHash(char *str, unsigned int len);
unsigned int DEKHash(char *str, unsigned int len);
unsigned int DJBHash(char *str, unsigned int len);
unsigned int ELFHash(char *str, unsigned int len);
unsigned int FNVHash(char *str, unsigned int len);
unsigned int JSHash(char *str, unsigned int len);
unsigned int PJWHash(char *str, unsigned int len);
unsigned int RSHash(char *str, unsigned int len);
unsigned int SDBMHash(char *str, unsigned int len);
/* end hash algorithm */

typedef struct rk_node_s {
    struct rk_node_s *next;
    void *data; /* point to user data */
    char *key;
    unsigned int key_len;
} rk_node_t;

typedef struct rk_table_s {
    rk_node_t *next;
    rk_node_t *prev;
} rk_table_t;

typedef struct rk_ht_s {
    rk_table_t *table;
    unsigned int mask; /* the mask of current table size */

    unsigned int size; /* number of nodes currently in use */
    unsigned int table_size;

    rk_node_t *free_nodes; /* point to free nodes */
    rk_node_t *free; /* for free all nodes */
    rk_hs_func hs_func; /* hash function */
} rk_ht_t;

rk_ht_t *rk_ht_create(unsigned int table_size, rk_hs_func hs_func);
void rk_ht_destroy(rk_ht_t *table);
int rk_ht_insert_s(rk_ht_t *table, char *key, unsigned int key_len,
                   void *value);
#define rk_ht_insert(table, key, value) \
    rk_ht_insert_s(table, (char *)key, sizeof(*key), (void *)value)
int rk_ht_erase_s(rk_ht_t *table, char *key, unsigned int key_len);
#define rk_ht_erase(table, key) rk_ht_erase_s(table, (char *)key, sizeof(*key))
void *rk_ht_find_s(rk_ht_t *table, char *key, unsigned int key_len);
#define rk_ht_find(table, key) rk_ht_find_s(table, (char *)key, sizeof(*key))
int rk_ht_clear(rk_ht_t *table);
/**
 * create an iterator
 * please free the return value with rk_ht_free_iter
 * 
 * @param table
 * @return a pointer to an iterator
*/
rk_node_t **rk_ht_create_iter(rk_ht_t *table);
void rk_ht_free_iter(rk_node_t **iter);

#endif