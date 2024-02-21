#include "c_hashmap.h"
#include <stdio.h>

int my_hash(const void *k) {
    return *((int *)k);
}

bool my_foreach_func(const void *k, void *v) {
    printf("%s = %s\n", (char *)k, (char *)v);
    // return str_eq_func(k, "Band");
    return false;
}

bool my_filter_func(const void *k, void *v) {
    return str_eq_func((char *)k, "def") || str_eq_func((char *)k, "java") || str_eq_func((char *)v, "fun");
}

void my_free_func(const void *o) {
    printf("freed: %s\n", (char *)o);
    // free(o);
}

void print_hashmap(const hashmap map) {
    hash_map_entry *bucket = map->bucket;
    for (int i = 0; i < map->cap; i++, bucket++) {
        hash_map_entry e = *bucket;
        if (e == NULL) continue;

        printf("%d\n", i);

        while (e != NULL) {
            printf("    %d-->%s: %s\n", e->hash, (char *)e->key, (char *)e->val);
            e = e->next;
        }
    }
}

int main() {
    printf("Size of void *: %lu\n", sizeof(void *));
    printf("%d\n", int_hash(0x12345));
    int key = 0x12345;
    printf("%d\n", hash(&my_hash, &key));
    printf("Size of char: %lu\n", sizeof(char));
    printf("Size of int: %lu\n", sizeof(int));
    printf("Size of hash entry: %lu\n", sizeof(hash_map_entry));
    printf("Size of hash node entry: %lu\n", sizeof(struct _hash_map_entry));

    hashmap map = hashmap_init(8, &str_hash_func, &str_eq_func, &str_eq_func);
    hashmap_set_k_free_func(map, &my_free_func);
    hashmap_set_v_free_func(map, &my_free_func);
    printf("Size: %d, Cap: %d, Factor: %f\n", map->size, map->cap, map->expand_factor);
    char *a = "hello";
    char *b = "world";
    char *c = "v";
    char *d = "java";
    hashmap_put(map, "1234", a);
    printf("%s\n", (char *)hashmap_get(map, "1234"));
    hashmap_put(map, "123", b);
    hashmap_put(map, "k", c);

    printf("%s\n", (char *)(hashmap_get(map, "1234")));
    printf("%s\n", (char *)(hashmap_get(map, "123")));
    printf("%s\n", (char *)(hashmap_get(map, "k")));

    hashmap_put(map, "123", d);
    hashmap_put(map, "lang", "java");

    printf("\n");
    print_hashmap(map);

    printf("%s\n", (char *)hashmap_get(map, "123"));
    printf("%s\n", (char *)hashmap_get(map, "lang"));
    printf("Cap: %d\n", map->cap);

    hashmap_put(map, "my best friend", "Mao mao");
    hashmap_put(map, "EOF", "eof");
    hashmap_put(map, "King", "queen");
    hashmap_put(map, "Band", "Linkin Park");

    print_hashmap(map);

    hashmap_itr itr = hashmap_itr_init(&my_foreach_func);
    // itr->filter_f = &my_filter_func;

    hashmap_foreach(map, itr);
    printf("Size: %d, Cap: %d\n\n", map->size, map->cap);

    printf("removed: %s, %s\n", "King", (char *)hashmap_remove(map, "King"));
    printf("removed: %s, %s\n", "EOF", (char *)hashmap_remove(map, "EOF"));
    printf("removed: %s, %s\n", "123", (char *)hashmap_remove(map, "123"));
    printf("removed: %s, %s\n", "1234", (char *)hashmap_remove(map, "1234"));
    printf("removed: %s, %s\n", "k", (char *)hashmap_remove(map, "k"));

    print_hashmap(map);

    printf("removed: %s, %s\n", "nil", (char *)hashmap_remove(map, "nil"));
    printf("\nafter remove:\n");
    hashmap_foreach(map, itr);
    printf("Size: %d, Cap: %d\n\n", map->size, map->cap);

    printf("removed: %s, %s\n", "Band", (char *)hashmap_remove(map, "Band"));
    printf("removed: %s, %s\n", "lang", (char *)hashmap_remove(map, "lang"));
    printf("\nafter remove:\n");
    hashmap_foreach(map, itr);
    printf("Size: %d, Cap: %d\n\n", map->size, map->cap);

    hashmap_put(map, "study", "fun");
    hashmap_put_f(map, "string", "this", &my_free_func, &my_free_func);
    print_hashmap(map);
    printf("Size: %d, Cap: %d\n\n", map->size, map->cap);

    printf("%d\n", hashmap_contains_key(map, "str"));
    printf("%d\n", hashmap_contains_key(map, "string"));
    printf("%d\n", hashmap_contains_key(map, "study"));
    printf("%d\n", hashmap_contains_value(map, "Mao mao"));
    printf("%d\n", hashmap_contains_value(map, "this"));
    printf("%d\n", hashmap_contains_value(map, "Band"));

    printf("removed: %s, %s\n\n", "string", (char *)hashmap_remove(map, "string"));

    printf("%s\n", (char *)hashmap_get_or_default(map, "def", "default"));
    printf("%s\n", (char *)hashmap_get_or_default(map, "def", "another default"));
    printf("%s\n", (char *)hashmap_get_or_default(map, "study", "study def"));
    printf("%s\n", (char *)hashmap_put_if_absent(map, "java", "JDK 1.8"));
    printf("%s\n", (char *)hashmap_put_if_absent(map, "java", "JDK 17"));
    printf("%s\n", (char *)hashmap_put_if_absent(map, "def", "default"));
    printf("%s\n", (char *)hashmap_put_if_absent(map, "def", "another default"));
    printf("%s\n", (char *)hashmap_get_or_default(map, "def", "another default"));
    printf("%s\n\n", (char *)hashmap_put_if_absent(map, "study", "good"));
    
    printf("Size: %d, Cap: %d\n\n", map->size, map->cap);
    print_hashmap(map);

    printf("\n");
    printf("remove count: %d\n", hashmap_remove_if(map, &my_filter_func));
    printf("Size: %d, Cap: %d\n\n", map->size, map->cap);
    print_hashmap(map);

    printf("\n");
    hashmap_clear(map);
    print_hashmap(map);
    printf("Size: %d, Cap: %d\n\n", map->size, map->cap);

    printf("%s, %s, %s, %s\n", a, b, c, d);
    printf("%d\n", hashmap_contains_key(map, "study"));
    printf("%d\n", hashmap_contains_value(map, "Mao mao"));

    hashmap_free(map);
    hashmap_itr_free(itr);

    return 0;
}
