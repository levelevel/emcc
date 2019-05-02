#include "9cc.h"

//可変長ベクタ ---------------------------------------
Vector *new_vector(void) {
    Vector *vec = calloc(1, sizeof(Vector));
    vec->data = malloc(sizeof(void*)*16);
    vec->capacity = 16;
    vec->len = 0;
    return vec;
}

void vec_push(Vector *vec, void *elem) {
    if (vec->capacity == vec->len) {
        vec->capacity *= 2;
        vec->data = realloc(vec->data, sizeof(void*) * vec->capacity);
    }
    vec->data[vec->len++] = elem;
}

//マップ --------------------------------------------
Map *new_map(void) {
    Map *map = malloc(sizeof(Map));
    map->keys = new_vector();
    map->vals = new_vector();
    return map;
}

void map_put(Map *map, char *key, void *val) {
    vec_push(map->keys, key);
    vec_push(map->vals, val);
}

char* map_get(const Map *map, char *key) {
    for (int i=map->keys->len-1; i>=0; i--) {
        if (strcmp(map->keys->data[i], key)==0) {
            return map->vals->data[i];
        }
    }
    return NULL;
}

// エラーを報告するための関数 --------------------------
// printfと同じ引数を取る
void error(const char*fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// テスト
static void expect(int line, long expected, long actual) {
    if (expected == actual) return;
    fprintf(stderr, "%d: %ld expected, but got actual %ld\n", line, expected, actual);
    exit(1);
}

static void test_vector(void) {
    Vector *vec = new_vector();
    expect(__LINE__, 0, vec->len);

    for (long i=0; i<100; i++) {
        vec_push(vec, (void*)i);
    }
    expect(__LINE__, 100, vec->len);
    expect(__LINE__, 0, (long)vec->data[0]);
    expect(__LINE__, 50, (long)vec->data[50]);
    expect(__LINE__, 99, (long)vec->data[99]);
}

static void test_map(void) {
    Map *map = new_map();
    expect(__LINE__, 0, (long)map_get(map, "foo"));

    map_put(map, "foo", (void *)2);
    expect(__LINE__, 2, (long)map_get(map, "foo"));

    map_put(map, "bar", (void *)4);
    expect(__LINE__, 4, (long)map_get(map, "bar"));

    map_put(map, "foo", (void *)6);
    expect(__LINE__, 6, (long)map_get(map, "foo"));
}

void run_test(void) {
    test_vector();
    test_map();
    printf("run_test: OK\n");
}

