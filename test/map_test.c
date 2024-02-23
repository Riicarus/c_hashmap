#include "c_hashmap.h"
#include <stdio.h>
#include <sys/time.h>

typedef struct _student {
    char *name;
    int   age;
} student;

student *student_new(char *name, int age) {
    student *s = calloc(1, sizeof(student));
    *s = (student){name, age};
    return s;
}

void *student_produce_func(void *stu) {
    student *s = (student *)stu;
    return student_new(s->name, s->age);
}

bool student_filter_func(void *stu) {
    student *s = (student *)stu;
    return s->age < 30;
}

void *get_name(void *stu) {
    student *s = (student *)stu;
    return s->name;
}

void *get_age(void *stu) {
    student *s = (student *)stu;
    return &(s->age);
}

void stu_update(void *stu1, void *stu2) {
    printf("%s, %d, %s, %d\n",
           ((student *)stu1)->name,
           ((student *)stu1)->age,
           ((student *)stu2)->name,
           ((student *)stu2)->age);
    ((student *)stu1)->age = ((student *)stu2)->age;
}

void stu_free(void *stu) {
    printf("freed: %s=%d\n", ((student *)stu)->name, ((student *)stu)->age);
    // can not free literal char sequence
    // free(((student *)stu)->name);
    free(stu);
}

void stu_free_plus(void *stu) {
    printf("plus!!!!freed: %s=%d\n", ((student *)stu)->name, ((student *)stu)->age);
    free(stu);
}

bool foreach_f(void *stu) {
    printf("name=%s, age=%d\n", ((student *)stu)->name, ((student *)stu)->age);
    return false;
}

bool filter_diff_func(void *stu) {
    student *s = (student *)stu;
    return s->age != atoi(s->name);
}

void print_map(hashmap map) {
    hash_map_entry *b = map->bucket;
    hash_map_entry  e;
    for (int i = 0; i < map->cap; i++, b++) {
        if ((e = *b) == NULL) continue;
        printf("%d\n", i);
        while (e != NULL) {
            printf("----%d: %s=%d\n", e->hash, (char *)get_name(e->ele), *(int *)get_age(e->ele));
            e = e->next;
        }
    }
}

void test_put(hashmap map) {
    printf("\n");
    printf("--------put test--------\n");
    student **stus = calloc(8, sizeof(student *));
    stus[0] = student_new("Riicarus", 22);
    stus[1] = student_new("xiaoming", 10);
    stus[2] = student_new("xiaohu", 11);
    stus[3] = student_new("Alex", 20);
    stus[4] = student_new("Scout", 32);
    stus[5] = student_new("TheShy", 24);
    stus[6] = student_new("Bug", 99);
    stus[7] = student_new("Bugee", 999);
    hashmap_put(map, stus[0]);
    hashmap_put(map, stus[1]);
    hashmap_put(map, stus[2]);
    hashmap_put(map, stus[3]);
    hashmap_put(map, stus[4]);
    hashmap_put(map, stus[5]);
    hashmap_put(map, stus[6]);
    hashmap_put(map, stus[7]);
    printf("\n");
    printf("Remained(%d/%d):\n", map->size, map->cap);
    print_map(map);
    printf("--------------------------------\n");
}

void test_put_f(hashmap map) {
    printf("\n");
    printf("--------put_f test--------\n");
    student **stus = calloc(5, sizeof(student *));
    stus[0] = student_new("Skyline", 22);
    stus[1] = student_new("Peter", 10);
    stus[2] = student_new("KenZhu888", 11);
    stus[3] = student_new("Zzitai886", 20);
    stus[4] = student_new("JJking", 32);
    hashmap_put_f(map, stus[0], &stu_free_plus);
    hashmap_put_f(map, stus[1], &stu_free_plus);
    hashmap_put_f(map, stus[2], &stu_free_plus);
    hashmap_put_f(map, stus[3], &stu_free_plus);
    hashmap_put_f(map, stus[4], &stu_free_plus);
    printf("Remained(%d/%d):\n", map->size, map->cap);
    print_map(map);
    printf("--------------------------------\n");
}

void test_put_if_absent(hashmap map) {
    printf("\n");
    printf("--------put_if_absent test--------\n");
    student **stus = calloc(5, sizeof(student *));
    stus[0] = student_new("Dragon", 22);
    stus[1] = student_new("TheShy", 10);
    stus[2] = student_new("Bug", 11);
    stus[3] = student_new("Key", 20);
    stus[4] = student_new("101", 32);
    hashmap_put_if_absent(map, stus[0], stus[0]);
    hashmap_put_if_absent(map, stus[1], stus[1]);
    hashmap_put_if_absent(map, stus[2], stus[2]);
    hashmap_put_if_absent_f(map, stus[3], &student_produce_func);
    hashmap_put_if_absent_f(map, stus[4], &student_produce_func);
    printf("Remained(%d/%d):\n", map->size, map->cap);
    print_map(map);
    printf("--------------------------------\n");
}

void test_get(hashmap map) {
    printf("\n");
    printf("--------get test--------\n");
    printf("Get: xiaoming=%d\n", *(int *)hashmap_get(map, &(student){"xiaoming", 1}));
    printf("Get: Scout=%d\n", *(int *)hashmap_get(map, &(student){"Scout"}));
    printf("Get: Riicarus=%d\n", *(int *)hashmap_get(map, &(student){"Riicarus"}));
    printf("Get: Alex=%d\n", *(int *)hashmap_get(map, &(student){"Alex"}));
    printf("--------------------------------\n");
}

void test_get_or_default(hashmap map) {
    printf("\n");
    printf("--------get_or_default test--------\n");
    student *s = &(student){"xiaoming", 11111};
    printf("Get: xiaoming=%d\n", *(int *)hashmap_get_or_default(map, s, s));
    s = &(student){"JJking", 22222};
    printf("Get: JJking=%d\n", *(int *)hashmap_get_or_default(map, s, s));
    s = &(student){"Meico", 444444};
    printf("Get: Meico=%d\n", *(int *)hashmap_get_or_default_f(map, s, &student_produce_func));
    s = &(student){"TheShy", 333333};
    printf("Get: TheShy=%d\n", *(int *)hashmap_get_or_default_f(map, s, &student_produce_func));
    printf("--------------------------------\n");
}

void test_contains(hashmap map) {
    printf("\n");
    printf("--------contains test--------\n");
    printf("Remained(%d/%d):\n", map->size, map->cap);
    print_map(map);
    printf("\n");
    printf("Constains key Riicarus: %d\n", hashmap_contains_key(map, &(student){"Riicarus"}));
    printf("Constains key Scout: %d\n", hashmap_contains_key(map, &(student){"Scout"}));
    printf("Constains key xiaohu: %d\n", hashmap_contains_key(map, &(student){"xiaohu"}));
    printf("Contains value 22: %d\n", hashmap_contains_value(map, &(student){.age = 22}));
    printf("Contains value 11111: %d\n", hashmap_contains_value(map, &(student){.age = 11111}));
    printf("Contains value 10: %d\n", hashmap_contains_value(map, &(student){.age = 10}));
    printf("--------------------------------\n");
}

void test_remove(hashmap map) {
    printf("\n");
    printf("--------remove test--------\n");
    hashmap_remove(map, &(student){"xiaoming"});
    hashmap_remove(map, &(student){"Scout"});
    hashmap_remove(map, &(student){"xiaohu"});
    hashmap_remove(map, &(student){"Alex"});
    hashmap_remove(map, &(student){"Bugee"});
    printf("\n");
    printf("Remained(%d/%d):\n", map->size, map->cap);
    print_map(map);
    printf("--------------------------------\n");
}

void test_remove_if(hashmap map) {
    printf("\n");
    printf("--------remove if test--------\n");
    printf("Removed count: %d\n", hashmap_remove_if(map, &student_filter_func));
    printf("\n");
    printf("Remained(%d/%d):\n", map->size, map->cap);
    print_map(map);
    printf("--------------------------------\n");
}

void test_foreach(hashmap map) {
    printf("\n");
    printf("--------foreach test--------\n");
    hashmap_itr itr = hashmap_itr_new(&foreach_f);
    hashmap_foreach(map, itr);
    printf("--------------------------------\n");
}

void test_free(hashmap map) {
    printf("\n");
    printf("--------free test--------\n");
    hashmap_free(map);
    printf("--------------------------------\n");
}

// avg_time(unit: ns/op)--o0(o3): 1e3: 53(40), 1e4: 76(54), 1e5: 107(72), 1e6: 113(74), 1e7: 128(79)
void benchmark_put_expand() {
    printf("\n");
    printf("--------benchmark expand put--------\n");
    int     cnt = 10000000;
    hashmap map = hashmap_new_default(&get_name, &get_age, &stu_update, &str_hash_func, &str_eq_func, &str_eq_func);
    hashmap_set_free_func(map, &stu_free);
    student **stus = calloc(cnt, sizeof(student *));

    student **stu_itr = stus;
    for (int i = 0; i < cnt; i++, stu_itr++) {
        char *c = calloc(6, sizeof(char));
        sprintf(c, "%d", i);
        *stu_itr = student_new(c, i);
    }
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long long st = tv.tv_sec * 1e6 + tv.tv_usec;
    stu_itr = stus;
    for (int i = 0; i < cnt; i++, stu_itr++) {
        hashmap_put(map, *stu_itr);
    }
    gettimeofday(&tv, NULL);
    long long et = tv.tv_sec * 1000000 + tv.tv_usec;
    printf("total_op: %d, total_time: %lld us, avg: %f ns\n", cnt, et - st, ((et - st) * 1000.0 / cnt));

    free(stus);
    printf("--------------------------------\n");
}

// avg_time(unit: ns/op)--o0(o3): 1e3: 55(40), 1e4: 65(45), 1e5: 78(50), 1e6: 88(55), 1e7: 98(58)
void benchmark_put_no_expand() {
    printf("\n");
    printf("--------benchmark no expand put--------\n");
    int     cnt = 100000;
    hashmap map = hashmap_new(cnt << 1, &get_name, &get_age, &stu_update, &str_hash_func, &str_eq_func, &str_eq_func);
    hashmap_set_free_func(map, &stu_free);
    student **stus = calloc(cnt, sizeof(student *));
    student **stu_itr = stus;
    for (int i = 0; i < cnt; i++, stu_itr++) {
        char *c = calloc(8, sizeof(char));
        sprintf(c, "%d", i);
        *stu_itr = student_new(c, i);
    }
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long long st = tv.tv_sec * 1e6 + tv.tv_usec;
    stu_itr = stus;
    for (int i = 0; i < cnt; i++, stu_itr++) {
        hashmap_put(map, *stu_itr);
    }
    gettimeofday(&tv, NULL);
    long long et = tv.tv_sec * 1000000 + tv.tv_usec;
    printf("total_op: %d, total_time: %lld us, avg: %f ns\n", cnt, et - st, ((et - st) * 1000.0 / cnt));

    free(stus);
    printf("--------------------------------\n");
}

// avg_time(unit: ns/op)--o0(o3): 1e3: 17(7), 1e4: 18(7), 1e5: 25(10), 1e6: 28(11), 1e7: 32(12)
void benchmark_get() {
    printf("\n");
    printf("--------benchmark get--------\n");
    int     cnt = 1000;
    hashmap map = hashmap_new(cnt, &get_name, &get_age, &stu_update, &str_hash_func, &str_eq_func, &str_eq_func);
    hashmap_set_free_func(map, &stu_free);
    student **stus = calloc(cnt, sizeof(student *));
    student **stu_itr = stus;
    for (int i = 0; i < cnt; i++, stu_itr++) {
        char *c = calloc(8, sizeof(char));
        sprintf(c, "%d", rand() % cnt);
        *stu_itr = student_new(c, i);
    }

    stu_itr = stus;
    for (int i = 0; i < cnt; i++, stu_itr++) {
        hashmap_get(map, *stu_itr);
    }
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long long st = tv.tv_sec * 1e6 + tv.tv_usec;
    stu_itr = stus;
    for (int i = 0; i < cnt; i++, stu_itr++) {
        hashmap_get(map, *stu_itr);
    }
    gettimeofday(&tv, NULL);
    long long et = tv.tv_sec * 1000000 + tv.tv_usec;
    printf("total_op: %d, total_time: %lld us, avg: %f ns\n", cnt, et - st, ((et - st) * 1000.0 / cnt));

    free(stus);
    printf("--------------------------------\n");
}

int main() {
    hashmap map = hashmap_new(3, &get_name, &get_age, &stu_update, &str_hash_func, &str_eq_func, &str_eq_func);
    hashmap_set_free_func(map, &stu_free);

    test_put(map);

    test_get(map);

    test_foreach(map);

    test_remove(map);

    test_put_f(map);

    test_contains(map);

    test_get_or_default(map);

    test_remove_if(map);

    test_put_if_absent(map);

    test_free(map);

    benchmark_put_expand();

    // benchmark_put_no_expand();

    // benchmark_get();
}