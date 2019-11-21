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

void vec_copy(Vector *dst, Vector *src) {
    for (int i=0; i<vec_len(src); i++) {
        vec_push(dst, vec_data(src, i));
    }
}

//マップ --------------------------------------------
Map *new_map(void) {
    Map *map = malloc(sizeof(Map));
    map->keys = new_vector();
    map->vals = new_vector();
    return map;
}

void map_put(Map *map, const char *key, void *val) {
    vec_push(map->keys, (void*)key);
    vec_push(map->vals, val);
}

int map_get(const Map *map, const char *key, void**val) {
    for (int i=map->keys->len-1; i>=0; i--) {
        if (strcmp(map->keys->data[i], key)==0) {
            if (val) *val = map->vals->data[i];
            return 1;
        }
    }
    if (val) *val = NULL;
    return 0;
}

//スタック -------------------------------------------
Stack *new_stack(void) {
    return (Stack*)new_vector();
}

//スタックトップにpushする。
int stack_push(Stack *stack, void*elem) {
    vec_push(stack, elem);
    return stack->len;
}

//スタックトップをpopする。
void *stack_pop(Stack *stack) {
    assert(stack->len>0);
    stack->len--;
    //fprintf(stderr, "stack_pop: %d\n", stack->len);
    return stack->data[stack->len];
}

//スタックのidx番目の要素を取り出す。スタックは変化しない。
void *stack_get(Stack *stack, int idx) {
    assert(idx < stack->len && idx >= 0);
    return stack->data[idx];
}

// ユーティリティ ----------------------------------------
//識別子に使用できる文字
int is_alnum(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') || (c == '_');
}
//識別子の先頭に使用できる文字
int is_alpha(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_');
}
//16進数に使用できる文字
int is_hex(char c) {
    return ('0' <= c && c <= '9') || ('a' <= c && c <= 'F') || ('A' <= c && c <= 'F');
}

//ファイルを1個の文字列バッファに読み込む
char *read_file(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) error("cannot open %s: %s", path, strerror(errno));

    if (fseek(fp, 0, SEEK_END) == -1) error("%s: fseek: %s", path, strerror(errno));
    size_t size = ftell(fp);
    if (fseek(fp, 0, SEEK_SET) == -1) error("%s: fseek: %s", path, strerror(errno));

    char *buf = malloc(size+2);
    fread(buf, size, 1, fp);
    fclose(fp);

    if (buf[size-1] != '\n') buf[size++] = '\n';
    buf[size] = 0;
    return buf;
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
    int indent = fprintf(stderr, "emcc:%s: %s:%d: ", level, filename, line_num);
    fprintf(stderr, "%.*s\n", (int)(end - line), line);

    // エラー箇所を"^"で指し示して、エラーメッセージを表示
    int pos = loc - line + indent;
    fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
    fprintf(stderr, "^ ");
}

static void err_ctrl(ErCtrl ctrl) {
    switch (ctrl) {
    case ERC_CONTINUE: return;
    case ERC_EXIT:     exit(1);
    case ERC_LONGJMP:  longjmp(jmpbuf, 1);
    case ERC_ABORT:    abort();
    }
}

void error_at(const char*loc, const char*fmt, ...){
    message_at(loc, "Error");

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    error_cnt++;
    err_ctrl(error_ctrl);
}

void warning_at(const char*loc, const char*fmt, ...){
    message_at(loc, "Warning");

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    warning_cnt++;
    err_ctrl(warning_ctrl);
}

void note_at(const char*loc, const char*fmt, ...){
    message_at(loc, "Note");

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    note_cnt++;
    err_ctrl(note_ctrl);
  }

// エラーと警告を報告するための関数 --------------------------
// printfと同じ引数を取る
void error(const char*fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "emcc:Error: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void warning(const char*fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "emcc:Warning: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
}

// テスト -------------------------------------------------
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
    test_error();   //test_src/test_error.c
    printf("run_test: OK\n");
}
