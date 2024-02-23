#ifndef C_HASH_MAP_H
#define C_HASH_MAP_H

#include <stdlib.h>
#include <stdbool.h>

typedef unsigned int uint;

// get key or value of element
typedef void *(*attr_get_func)(void *ele);
// set ele2's value to ele1
typedef void (*val_update_func)(void *ele1, void *ele2);
// free space of the given pointer.
typedef void (*free_func)(void *ele);
// calculate hash code for the given argument, the input is ele's key, not ele.
typedef int (*hash_func)(void *k);
// judge if the two is the same, true means the same, the input is ele's key/val, not ele.
typedef bool (*eq_func)(void *k1, void *k2);
// produce a val by the key.
typedef void *(*produce_func)(void *ele);
// return true if meets your given condition.
typedef bool (*filter_func)(void *ele);
// return true if need stop.
typedef bool (*foreach_func)(void *ele);

// ele is the k/v pair
typedef struct _hash_map_entry {
    int   hash;
    void *ele;

    struct _hash_map_entry *next;

    free_func free_f;
} *hash_map_entry;

/*
 * Careful that _hashmap is not thread safe.
 * free_func of _hashmap can also act as a callback function when removing an entry.
 *
 * Each entry contains a void *ele pointer, which is the k-v pair.
 */
typedef struct _hashmap {
    uint            size;
    uint            cap;
    float           expand_factor;
    float           shrink_factor;
    hash_map_entry *bucket;

    attr_get_func   k_get_f;
    attr_get_func   v_get_f;
    val_update_func v_update_f;
    hash_func       hash_f;
    eq_func         k_eq_f;
    eq_func         v_eq_f;
    free_func       free_f;
} *hashmap;

#define DEFAULT_INIT_CAP 8
#define DEFAULT_EXPAND_FACTOR 0.75
#define DEFAULT_SHRINK_FACTOR 0.20
// k/v_get_f could not be null
hashmap hashmap_new_f(int             init_cap,
                      float           expand_factor,
                      float           shrink_factor,
                      attr_get_func   k_get_f,
                      attr_get_func   v_get_f,
                      val_update_func v_update_f,
                      hash_func       hash_f,
                      eq_func         k_eq_f,
                      eq_func         v_eq_f,
                      free_func       free_f);
// k/v_get_f could not be null
hashmap hashmap_new(int             init_cap,
                    attr_get_func   k_get_f,
                    attr_get_func   v_get_f,
                    val_update_func v_update_f,
                    hash_func       hash_f,
                    eq_func         k_eq_f,
                    eq_func         v_eq_f);
// init _hashmap with default capacity 8. k/v_get_f could not be null
hashmap hashmap_new_default(attr_get_func   k_get_f,
                            attr_get_func   v_get_f,
                            val_update_func v_update_f,
                            hash_func       hash_f,
                            eq_func         k_eq_f,
                            eq_func         v_eq_f);

void hashmap_set_free_func(const hashmap map, free_func free_f);

// returns true if contains the given key.
bool hashmap_contains_key(const hashmap map, void *ele);
// returns true if contains the given value.
bool hashmap_contains_value(const hashmap map, void *ele);

// get value of the given key.
void *hashmap_get(const hashmap map, void *ele);
/*
 * Get value of the given ele's key,
 * if the key is absent, return the given def_ele's value .
 */
void *hashmap_get_or_default(const hashmap map, void *ele, void *def_ele);
/*
 * Get value of the given ele's key,
 * if the key is absent, return ele's value produced by produce_func.
 */
void *hashmap_get_or_default_f(const hashmap map, void *ele, produce_func produce_f);

// return the put ele's value and set free func to the given ele.
void *hashmap_put_f(const hashmap map, void *ele, free_func free_f);
// return the put ele's value.
void *hashmap_put(const hashmap map, void *ele);
/*
 * Put the given def_ele's value if the ele's key is absent;
 * Return the actual ele's value of entry with the given ele's key.
 */
void *hashmap_put_if_absent(const hashmap map, void *ele, void *def_ele);
/*
 * Put the value produced by produce_f if the ele's key is absent;
 * Return the actual ele's value of entry with the given ele's key.
 */
void *hashmap_put_if_absent_f(const hashmap map, void *ele, produce_func produce_f);

// set free function only for one entry with the given ele's key.
bool hashmap_ele_set_free_func(const hashmap map, void *ele, free_func free_f);

// returned value may be invalid caused by free_func.
void *hashmap_remove(const hashmap map, void *ele);
// return removed entry count.
uint  hashmap_remove_if(const hashmap map, filter_func filter_f);
// clear all entries without shrink capacity.
void  hashmap_clear(const hashmap map);
// free all hashmap space(including entry's key & value) using the registered free_func.
void  hashmap_free(hashmap map);

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

hashmap_itr hashmap_itr_new(foreach_func foreach);
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

static int hash(hash_func hash_f, void *k) {
    return int_hash(hash_f(k));
}

static int str_hash_func(void *k) {
    int         h = 0;
    const char *c = (char *)k;
    while (*c != '\0') {
        h += h * 7 + *c;
        c++;
    }
    return h;
}

static int ptr_hash_func(void *k) {
    return *((long *)k);
}

static bool ptr_eq_func(void *k1, void *k2) {
    return k1 == k2;
}

static bool int_eq_func(void *k1, void *k2) {
    return *(int *)k1 == *(int *)k2;
}

static bool str_eq_func(void *k1, void *k2) {
    const char *c1 = (char *)k1;
    const char *c2 = (char *)k2;
    while (*c1 != '\0' && *c2 != '\0' && *c1 == *c2) {
        c1++;
        c2++;
    }
    return *c1 == *c2;
}

#endif