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

void *vec_get(Vector *vec, int idx) {
    assert(idx < vec->len);
    return vec->data[idx];
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
static const char *TypeStr[] = {"Nul", "void", "char", "short", "int", "long", "long long", "*", "[", "CONST"};
static const char *SClassStr[] = {"", "auto ", "register ", "static ", "extern "};

//bufに対して型を表す文字列を生成する
static void type_str(char *buf, const Type *tp) {
    if (tp->ptr_of) type_str(buf, tp->ptr_of);
    strcat(buf, SClassStr[tp->sclass]);
    if (tp->is_unsigned) strcat(buf, "unsigned ");
    if (tp->is_const)    strcat(buf, "const ");
    strcat(buf, TypeStr[tp->type]);
    if (tp->type==ARRAY) {
        char tmp[20];
        if (tp->array_size>=0) sprintf(tmp, "%ld]", tp->array_size);
        else                   sprintf(tmp ,"]");
        strcat(buf, tmp);
    }
}

// 型を表す文字列を返す。文字列はmallocされている。
const char* get_type_str(const Type *tp) {
    char buf[1024];
    const Type *p;
    if (tp==NULL) return "null";
    buf[0] = 0;
    //ARRAY[10]->array[2]->PTR->INT
    //ARRAY以外を深さ優先で先に処理する
    for (p=tp; p->type==ARRAY; p=p->ptr_of);
    type_str(buf, p);
    for (p=tp; p->type==ARRAY; p=p->ptr_of) {
        char tmp[20];
        if (p->array_size>=0) sprintf(tmp, "[%ld]", p->array_size);
        else                  sprintf(tmp ,"[]");
        strcat(buf, tmp);
    }
    char *ret = malloc(strlen(buf)+1);
    strcpy(ret, buf);
    return ret;
}

// 関数の引数リストを表す文字列を返す。文字列はmallocされている。
const char* get_func_args_str(const Node *node) {
    char buf[1024];
    assert(node->type==ND_LIST);
    int size = node->lst->len;
    Node **arg_nodes = (Node**)node->lst->data;
    int len = 0;
    buf[0] = 0;
    for (int i=0; i<size; i++) {
        if (arg_nodes[i]->type==ND_VARARGS) {
            len += sprintf(buf+len, "...");
        } else {
            len += sprintf(buf+len, "%s", get_type_str(arg_nodes[i]->tp));
            if (arg_nodes[i]->name)
                len += sprintf(buf+len, " %s", arg_nodes[i]->name);
        }
        if (i<size-1) len += sprintf(buf+len, ", ");
    }
    char *ret = malloc(strlen(buf)+1);
    strcpy(ret, buf);
    return ret;
}

// エラーの起きた場所を報告するための関数
static void message_at(const char*loc, const char *level) {
    // locが含まれている行の開始地点と終了地点を取得
    if (loc==NULL) loc = "(null)";
    const char *line = loc;
    while (user_input < line && line[-1] != '\n') line--;

    const char *end = loc;
    while (*end && *end != '\n') end++;

    // 見つかった行が全体の何行目なのかを調べる
    int line_num = 1;
    for (char *p = user_input; p < line; p++)
        if (*p == '\n') line_num++;

    // 見つかった行を、ファイル名と行番号と一緒に表示
    int indent = fprintf(stderr, "9cc:%s: %s:%d: ", level, filename, line_num);
    fprintf(stderr, "%.*s\n", (int)(end - line), line);

    // エラー箇所を"^"で指し示して、エラーメッセージを表示
    int pos = loc - line + indent;
    fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
    fprintf(stderr, "^ ");
}

void error_at(const char*loc, const char*fmt, ...){
    message_at(loc, "Error");

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");

    exit(1);
  }

void warning_at(const char*loc, const char*fmt, ...){
    message_at(loc, "Warning");

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");

    exit(1);
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
    expect(__LINE__, 0,  (long)vec->data[0]);
    expect(__LINE__, 50, (long)vec->data[50]);
    expect(__LINE__, 99, (long)vec->data[99]);
    expect(__LINE__, 0,  (long)vec_get(vec, 0));
    expect(__LINE__, 50, (long)vec_get(vec, 50));
    expect(__LINE__, 99, (long)vec_get(vec, 99));
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

