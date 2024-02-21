#ifndef C_HASH_MAP_H
#define C_HASH_MAP_H

#include <stdlib.h>
#include <stdbool.h>

typedef unsigned int uint;

// free space of the given pointer.
typedef void (*free_func)(const void *o);

typedef struct _hash_map_entry {
    int         hash;
    const void *key;
    void       *val;

    struct _hash_map_entry *next;

    free_func k_free_f;
    free_func v_free_f;
} *hash_map_entry;

// calculate hash code for the given argument.
typedef int (*hash_func)(const void *k);
// judge if the two is the same, true means the same.
typedef bool (*eq_func)(const void *k1, const void *k2);

/*
 * Careful that _hashmap is not thread safe.
 * free_func of _hashmap can also act as a callback function when removing an entry.
 */
typedef struct _hashmap {
    uint            size;
    uint            cap;
    float           expand_factor;
    float           shrink_factor;
    hash_map_entry *bucket;

    hash_func hash_f;
    eq_func   k_eq_f;
    eq_func   v_eq_f;
    free_func k_free_f;
    free_func v_free_f;
} *hashmap;

#define DEFAULT_INIT_CAP 8
#define DEFAULT_EXPAND_FACTOR 0.75
#define DEFAULT_SHRINK_FACTOR 0.20
// capacity of hashmap must be power of 2.
hashmap hashmap_init_f(
    int init_cap, float expand_factor, float shrink_factor, hash_func hash_f, eq_func k_eq_f, eq_func v_eq_f);
hashmap hashmap_init(int init_cap, hash_func hash_f, eq_func k_eq_f, eq_func v_eq_f);
// init _hashmap with default capacity 8.
hashmap hashmap_init_default(hash_func hash_f, eq_func k_eq_f, eq_func v_eq_f);

void hashmap_set_k_free_func(const hashmap map, free_func k_free_f);
void hashmap_set_v_free_func(const hashmap map, free_func v_free_f);

// returns true if contains the given key.
bool hashmap_contains_key(const hashmap map, const void *k);
// returns true if contains the given value.
bool hashmap_contains_value(const hashmap map, void *v);

// produce a val by the key.
typedef void *(*val_produce_func)(const void *k);

// get value of the given key.
void *hashmap_get(const hashmap map, const void *k);
/*
 * Get value of the given key,
 * if the key is absent, return the given default value .
 */
void *hashmap_get_or_default(const hashmap map, const void *k, void *def_val);
/*
 * Get value of the given key,
 * if the key is absent, return value produced by val_produce_func.
 */
void *hashmap_get_or_default_f(const hashmap map, const void *k, val_produce_func val_produce_f);

// return the put value.
void *hashmap_put_f(const hashmap map, const void *k, void *v, free_func k_free_f, free_func v_free_func);
// return the put value.
void *hashmap_put(const hashmap map, const void *k, void *v);
/*
 * Put the given value if the key is absent;
 * Return the actual value of entry with the given key.
 */
void *hashmap_put_if_absent(const hashmap map, const void *k, void *def_val);
/*
 * Put the value produced by val_produce_f if the key is absent;
 * Return the actual value of entry with the given key.
 */
void *hashmap_put_if_absent_f(const hashmap map, const void *k, val_produce_func val_produce_f);

// set free function only for one entry with the given key.
bool hashmap_set_entry_free_func(const hashmap map, const void *k, free_func k_free_f, free_func v_free_f);

// return true if meets your given condition.
typedef bool (*filter_func)(const void *k, void *v);

// returned value may be invalid caused by v_free_func.
void *hashmap_remove(const hashmap map, const void *k);
// return removed entry count.
uint  hashmap_remove_if(const hashmap map, filter_func filter_f);
// clear all entries without shrink capacity.
void  hashmap_clear(const hashmap map);
// free all hashmap space(including entry's key & value) using the registered free_func.
void  hashmap_free(hashmap map);

// return true if need stop.
typedef bool (*foreach_func)(const void *k, void *v);

/*
 * Careful that _hashmap_iterator is not thread safe.
 * _hashmap_iterator should only used to iterator the hash map entries, it's not supposed to update entries and DO NOT
 * remove entries using iterator(so we did not provide any removal functions in _hashmap_iterator, till now).
 *
 * _hashmap_iterator has no direct relationship with _hashmap except you need a _hashmap to iterator through. Just use
 * it more like in a functional programming environment.
 */
typedef struct _hashmap_iterator {
    foreach_func foreach_f;
    filter_func  filter_f;
} *hashmap_itr;

hashmap_itr hashmap_itr_init(foreach_func foreach);
void        hashmap_itr_free(hashmap_itr itr);
void        hashmap_itr_set_filter_f(const hashmap_itr itr, filter_func filter_f);
void        hashmap_itr_set_foreach_f(const hashmap_itr itr, foreach_func foreach_f);

/*
 * Iterator hashmap and apply the registered function of itr.
 * Stop if the foreach_f returns true.
 * Apply foreach_f only if meets the condition of filter_f(if registered).
 */
void hashmap_foreach(const hashmap map, const hashmap_itr itr);

/*
 * util functions
 */

static bool is_power_of_2(long s) {
    return !(s & (s - 1));
}

static uint round_up_power_of_2(uint n) {
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
}

static int int_hash(int i) {
    return (i == 0) ? 0 : i ^ (i >> 16);
}

static int hash(hash_func hash_f, const void *k) {
    return int_hash(hash_f(k));
}

static int str_hash_func(const void *k) {
    int         h = 0;
    const char *c = (char *)k;
    while (*c != '\0') {
        h += h * 7 + *c;
        c++;
    }
    return h;
}

static int ptr_hash_func(const void *k) {
    return *((long *)k);
}

static bool ptr_eq_func(const void *k1, const void *k2) {
    return k1 == k2;
}

static bool int_eq_func(const void *k1, const void *k2) {
    return *(int *)k1 == *(int *)k2;
}

static bool str_eq_func(const void *k1, const void *k2) {
    const char *c1 = (char *)k1;
    const char *c2 = (char *)k2;
    while (*c1 != '\0' && *c2 == '\0' && *c1 == *c2) {
        c1++;
        c2++;
    }
    return *c1 == *c2;
}

#endif