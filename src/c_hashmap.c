#include "c_hashmap.h"
#include "limits.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

hashmap hashmap_init_f(
    int init_cap, float expand_factor, float shrink_factor, hash_func hash_f, eq_func k_eq_f, eq_func v_eq_f) {
    hashmap map = (hashmap)calloc(1, sizeof(struct _hashmap));
    if (map == NULL) goto error;

    map->size = 0;
    if (init_cap <= 0) init_cap = DEFAULT_INIT_CAP;

    // avoid overflow
    if (init_cap >= INT_MAX >> 1) map->cap = INT_MAX;
    else map->cap = round_up_power_of_2(init_cap);

    map->expand_factor = expand_factor < 0 ? DEFAULT_EXPAND_FACTOR : expand_factor;
    map->shrink_factor = shrink_factor < 0 ? DEFAULT_SHRINK_FACTOR : shrink_factor;

    map->bucket = (hash_map_entry *)calloc(map->cap, sizeof(hash_map_entry));
    if (map->bucket == NULL) goto error;

    map->hash_f = hash_f == NULL ? &ptr_hash_func : hash_f;
    map->k_eq_f = k_eq_f == NULL ? &ptr_eq_func : k_eq_f;
    map->v_eq_f = v_eq_f == NULL ? &ptr_eq_func : v_eq_f;
    return map;

error:
    perror("no enough memory");
    return NULL;
}

hashmap hashmap_init(int init_cap, hash_func hash_f, eq_func k_eq_f, eq_func v_eq_f) {
    return hashmap_init_f(init_cap, DEFAULT_EXPAND_FACTOR, DEFAULT_SHRINK_FACTOR, hash_f, k_eq_f, v_eq_f);
}

hashmap hashmap_init_default(hash_func hash_f, eq_func k_eq_f, eq_func v_eq_f) {
    return hashmap_init(DEFAULT_INIT_CAP, hash_f, k_eq_f, v_eq_f);
}

void hashmap_set_k_free_func(const hashmap map, free_func k_free_f) {
    map->k_free_f = k_free_f;
}

void hashmap_set_v_free_func(const hashmap map, free_func v_free_f) {
    map->v_free_f = v_free_f;
}

int _hashmap_cul_index(uint cap, int h) {
    return h & (cap - 1);
}

/*
 * Expand:
 * hash = 110110, idx = 6(0110), cap = 16(10000), new_cap = 32(100000)
 * hash & cap(highest bit of (new_cap - 1)) = 110110 & 10000 = 010000
 * new_idx = 22 (110110 & 11111) = 6 + 16
 * otherwise:
 * hash = 100110, idx = 6(0110), cap = 16(10000), new_cap = 32(100000)
 * hash & cap(highest bit of (new_cap - 1)) = 100110 & 10000 = 000000
 * new_idx = 6 (100110 & 11111) = 6 (the same index)
 *
 * Shrink:
 * hash = 110110, idx = 22(10110), cap = 32(100000), new_cap = 16(10000)
 * hash & new_cap(highest bit of (cap - 1)) = 110110 & 10000 = 010000
 * new_idx = 6 (110110 & 1111) = 22 - 16
 * otherwise:
 * hash = 100110, idx = 22(0110), cap = 32(100000), new_cap = 16(10000)
 * hash & new_cap(highest bit of (cap - 1)) = 100110 & 10000 = 000000
 * new_idx = 6 (100110 & 1111) = 6 (the same index)
 */
void _hashmap_rehash(const hashmap map, hash_map_entry *new_bucket, bool is_expand) {
    hash_map_entry *b = map->bucket;
    hash_map_entry  e, ne;
    int             new_idx;

    uint new_cap = is_expand ? map->cap << 1 : map->cap >> 1;
    for (int i = 0; i < map->cap; i++, b++) {
        if ((e = *b) == NULL) continue;

        while (e != NULL) {
            /*
             * hash = 110110, idx = 6(0110), cap = 16(10000), new_cap = 32(100000)
             * hash & cap(highest bit of (new_cap - 1)) = 110110 & 10000 = 010000
             * new_idx = 22 (110110 & 11111) = 6 + 16
             * otherwise:
             * hash = 100110, idx = 6(0110), cap = 16(10000), new_cap = 32(100000)
             * hash & cap(highest bit of (new_cap - 1)) = 100110 & 10000 = 000000
             * new_idx = 6 (100110 & 11111) = 6 (the same index)
             */
            if (is_expand) new_idx = e->hash & map->cap ? i + map->cap : i;
            else new_idx = e->hash & new_cap ? i - new_cap : i;

            ne = e->next;
            e->next = NULL;

            // head-insert to new bucket
            hash_map_entry h = new_bucket[new_idx];
            new_bucket[new_idx] = e;
            if (h != NULL) e->next = h;
            e = ne;
        }
    }

    free(map->bucket);
    map->cap = new_cap;
    map->bucket = new_bucket;
}

// inc_size == 0 means shrink.
bool _hashmap_ensure_cap(const hashmap map, int inc_size) {
    if (map->size + inc_size > INT_MAX) {
        perror("reach the max capacity of hash map");
        return false;
    }

    bool is_expand;
    if (!inc_size && map->size <= map->shrink_factor * map->cap) is_expand = false;
    else if (map->cap != INT_MAX && inc_size > 0 && map->size + inc_size >= map->expand_factor * map->cap)
        is_expand = true;
    else return true;

    hash_map_entry *new_bucket = is_expand ? (hash_map_entry *)calloc(map->cap << 1, sizeof(hash_map_entry))
                                           : (hash_map_entry *)calloc(map->cap >> 1, sizeof(hash_map_entry));

    if (new_bucket == NULL) goto error;

    _hashmap_rehash(map, new_bucket, is_expand);
    return true;

error:
    perror("no enough memory");
    return false;
}

hash_map_entry _hashmap_get_entry(const hashmap map, const void *k) {
    int h = hash(map->hash_f, k);
    int idx = _hashmap_cul_index(map->cap, h);

    hash_map_entry e = map->bucket[idx];
    while (e != NULL) {
        if (map->k_eq_f(e->key, k)) return e;
        e = e->next;
    }
    return NULL;
}

bool hashmap_contains_key(const hashmap map, const void *k) {
    return _hashmap_get_entry(map, k) != NULL;
}

bool hashmap_contains_value(const hashmap map, void *v) {
    hash_map_entry *b = map->bucket;
    hash_map_entry  e;
    for (int i = 0; i < map->cap; i++, b++) {
        if ((e = *b) == NULL) continue;
        while (e != NULL) {
            if (map->v_eq_f(e->val, v)) return true;
            e = e->next;
        }
    }
    return false;
}

void *hashmap_get(const hashmap map, const void *k) {
    int h = hash(map->hash_f, k);
    int idx = _hashmap_cul_index(map->cap, h);

    hash_map_entry e = map->bucket[idx];
    while (e != NULL) {
        if (map->k_eq_f(e->key, k)) return e->val;
        e = e->next;
    }
    return NULL;
}

void *hashmap_get_or_default(const hashmap map, const void *k, void *def_val) {
    void *v = hashmap_get(map, k);
    if (v == NULL) return def_val;
    return v;
}

void *hashmap_get_or_default_f(const hashmap map, const void *k, val_produce_func val_produce_f) {
    void *v = hashmap_get(map, k);
    if (v == NULL) return val_produce_f(k);
    return v;
}

void *hashmap_put_f(const hashmap map, const void *k, void *v, free_func k_free_f, free_func v_free_func) {
    hash_map_entry e = (hash_map_entry)malloc(sizeof(struct _hash_map_entry));
    if (e == NULL) {
        perror("no enough memory");
        return v;
    }
    e->hash = hash(map->hash_f, k);
    e->key = k;
    e->val = v;
    e->next = NULL;
    e->k_free_f = k_free_f;
    e->v_free_f = v_free_func;

    if (!_hashmap_ensure_cap(map, 1)) return v;

    int idx = _hashmap_cul_index(map->cap, e->hash);
    if (map->bucket[idx] == NULL) {
        map->bucket[idx] = e;
        map->size += 1;
        return v;
    }

    // find if key exists
    hash_map_entry h = map->bucket[idx];
    while (h != NULL) {
        if (map->k_eq_f(h->key, k)) {
            h->val = v;
            return v;
        }
        h = h->next;
    }
    // key not exists, use head-insert
    e->next = map->bucket[idx];
    map->bucket[idx] = e;
    map->size += 1;

    return v;
}

void *hashmap_put(const hashmap map, const void *k, void *v) {
    return hashmap_put_f(map, k, v, NULL, NULL);
}

void *hashmap_put_if_absent(const hashmap map, const void *k, void *def_val) {
    hash_map_entry e = _hashmap_get_entry(map, k);
    if (e != NULL) return e->val;
    return hashmap_put(map, k, def_val);
}

void *hashmap_put_if_absent_f(const hashmap map, const void *k, val_produce_func val_produce_f) {
    hash_map_entry e = _hashmap_get_entry(map, k);
    if (e != NULL) return e->val;
    return hashmap_put(map, k, val_produce_f(k));
}

bool hashmap_set_entry_free_func(const hashmap map, const void *k, free_func k_free_f, free_func v_free_f) {
    int h = hash(map->hash_f, k);
    int idx = _hashmap_cul_index(map->cap, h);

    hash_map_entry e = map->bucket[idx];
    while (e != NULL) {
        if (map->k_eq_f(e->key, k)) {
            e->k_free_f = k_free_f;
            e->v_free_f = v_free_f;
            return true;
        }
        e = e->next;
    }
    return false;
}

void _free_entry(const hashmap map, hash_map_entry e) {
    free_func k_free_f = e->k_free_f == NULL ? map->k_free_f : e->k_free_f;
    free_func v_free_f = e->v_free_f == NULL ? map->v_free_f : e->v_free_f;
    if (k_free_f != NULL) k_free_f(e->key);
    if (v_free_f != NULL) v_free_f(e->val);
    free(e);
    e = NULL;
}

void *hashmap_remove(const hashmap map, const void *k) {
    int h = hash(map->hash_f, k);
    int idx = _hashmap_cul_index(map->cap, h);

    hash_map_entry e = map->bucket[idx];
    hash_map_entry pe = NULL;
    while (e != NULL) {
        if (map->k_eq_f(e->key, k)) {
            if (pe == NULL) map->bucket[idx] = e->next;
            else pe->next = e->next;

            void *val = e->val;
            _free_entry(map, e);
            map->size -= 1;

            _hashmap_ensure_cap(map, 0);
            return val;
        }
        pe = e;
        e = e->next;
    }

    return NULL;
}

uint hashmap_remove_if(const hashmap map, filter_func filter_f) {
    uint            cnt = 0;
    hash_map_entry *b = map->bucket;
    hash_map_entry  pe = NULL, e;

    for (int i = 0; i < map->cap; i++, b++) {
        if ((e = *b) == NULL) continue;

        while (e != NULL) {
            if (filter_f(e->key, e->val)) {
                if (pe == NULL) map->bucket[i] = e->next;
                else pe->next = e->next;

                _free_entry(map, e);
                e = e->next;
                map->size -= 1;
                cnt++;
                continue;
            }
            pe = e;
            e = e->next;
        }
    }

    _hashmap_ensure_cap(map, 0);
    return cnt;
}

void hashmap_clear(const hashmap map) {
    hash_map_entry *b = map->bucket;
    hash_map_entry  e, ne;
    for (int i = 0; i < map->cap; i++, b++) {
        if ((e = *b) == NULL) continue;

        while (e != NULL) {
            ne = e->next;
            _free_entry(map, e);
            e = ne;
        }
        *b = NULL;
    }

    map->size = 0;
}

void hashmap_free(hashmap map) {
    if (map == NULL) return;

    if (map->bucket != NULL) {
        hashmap_clear(map);
        free(map->bucket);
        map->bucket = NULL;
    }
    free(map);
    map = NULL;
}

hashmap_itr hashmap_itr_init(foreach_func foreach_f) {
    hashmap_itr itr = (hashmap_itr)calloc(1, sizeof(struct _hashmap_iterator));
    if (itr == NULL) goto error;

    itr->foreach_f = foreach_f;
    return itr;

error:
    perror("no enough memory");
    return NULL;
}

void hashmap_itr_free(hashmap_itr itr) {
    free(itr);
    itr = NULL;
}

void hashmap_itr_set_filter_f(const hashmap_itr itr, filter_func filter_f) {
    itr->filter_f = filter_f;
}

void hashmap_itr_set_foreach_f(const hashmap_itr itr, foreach_func foreach_f) {
    itr->foreach_f = foreach_f;
}

void hashmap_foreach(const hashmap map, const hashmap_itr itr) {
    hash_map_entry *b = map->bucket;
    hash_map_entry  e;
    for (int i = 0; i < map->cap; i++, b++) {
        if ((e = *b) == NULL) continue;

        while (e != NULL) {
            if (itr->filter_f == NULL || itr->filter_f(e->key, e->val)) {
                if (itr->foreach_f(e->key, e->val)) return;
                e = e->next;
            } else e = e->next;
        }
    }
}