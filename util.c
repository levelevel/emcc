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

int map_get(const Map *map, char *key, void**val) {
    for (int i=map->keys->len-1; i>=0; i--) {
        if (strcmp(map->keys->data[i], key)==0) {
            if (val) *val = map->vals->data[i];
            return 1;
        }
    }
    return 0;
}

// ダンプ関数 ----------------------------------------
static char *TypeStr[] = {"int", "*", "["};
static char buf1[128];
static void type_str(const Type *tp) {
    if (tp->ptr_of) type_str(tp->ptr_of);
    strcat(buf1, TypeStr[tp->type]);
    if (tp->type==ARRAY) {
        char buf[20];
        sprintf(buf, "%ld]", tp->array_size);
        strcat(buf1, buf);
    }
}

// 型を表す文字列を返す
const char* get_type_str(const Type *tp) {
    if (tp==NULL) return "null";
    buf1[0] = 0;
    type_str(tp);
    char *ret = malloc(strlen(buf1)+1);
    strcpy(ret, buf1);
    return ret;
}

static char buf2[1024];
// 関数の引数リストを表す文字列を返す
const char* get_func_args_str(const Node *node) {
    assert(node->type==ND_LIST);
    int size = node->lst->len;
    Node **ident_nodes = (Node**)node->lst->data;
    int len = 0;
    for (int i=0; i<size; i++) {
        len += sprintf(buf2+len, "%s %s", 
            get_type_str(ident_nodes[i]->tp), ident_nodes[i]->name);
        if (i<size-1) len += sprintf(buf2+len, ",");
    }
    return buf2;
}

// エラーと警告を報告するための関数 --------------------------
// printfと同じ引数を取る
void error(const char*fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "9cc:Error: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void warning(const char*fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "9cc:Warning: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
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
    void *val;
    expect(__LINE__, 0, map_get(map, "foo", NULL));

    map_put(map, "foo", (void *)2);
    expect(__LINE__, 1, map_get(map, "foo", &val));
    expect(__LINE__, 2, (long)val);

    map_put(map, "bar", (void *)4);
    expect(__LINE__, 1, map_get(map, "bar", &val));
    expect(__LINE__, 4, (long)val);

    map_put(map, "foo", (void *)6);
    expect(__LINE__, 1, map_get(map, "foo", &val));
    expect(__LINE__, 6, (long)val);
}

void run_test(void) {
    test_vector();
    test_map();
    printf("run_test: OK\n");
}

