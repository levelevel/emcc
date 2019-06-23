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
    if (stack->len>0) {
        return stack->data[--stack->len];
    }
    assert(0);
    return NULL;
}

//スタックトップの要素を取り出す。スタックは変化しない。
void *stack_get(Stack *stack, int idx) {
    assert (idx < stack->len);
    return stack->data[idx];
}

// ダンプ関数 ----------------------------------------
static const char *TypeStr[] = {"Nul", "void", "char", "short", "int", "long", "long long", "*", "[", "func(", "CONST"};
static const char *SClassStr[] = {"", "auto ", "register ", "static ", "extern "};

//bufに対してTypeをダンプする
static void dump_type(char *buf, const Type *tp) {
    const char *str = TypeStr[tp->type];
    if (tp->is_unsigned) strcat(buf, "unsigned ");
    if (tp->is_const)    strcat(buf, "const ");
    if (*buf && *str!='*' && *str!='[') strcat(buf, " ");
    strcat(buf, str);
    if (tp->type==ARRAY) {
        char tmp[20];
        if (tp->array_size>=0) sprintf(tmp, "%ld]", tp->array_size);
        else                   sprintf(tmp ,"]");
        strcat(buf, tmp);
    } else if (tp->type==FUNC) {
        if (tp->node) strcat(buf, get_func_args_str(tp->node->lhs));
        strcat(buf, ")");
    }
    if (tp->ptr_of) dump_type(buf, tp->ptr_of);
}

//bufに対して型を表す文字列をCの文法で生成する
static void print_type(char *buf, const Type *tp) {
    if (tp->ptr_of) print_type(buf, tp->ptr_of);
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

// 型を表す文字列をCの文法表記で返す。文字列はmallocされている。
char *get_type_str(const Type *tp) {
    char buf[1024];
    const Type *p;
    if (tp==NULL) return "null";
    buf[0] = 0;
    //ARRAY[10]->ARRAY[2]->PTR->INT
    //ARRAY以外を深さ優先で先に処理する
    for (p=tp; p->type==ARRAY; p=p->ptr_of);
    if (1) {
        dump_type(buf, p);
    } else {
        print_type(buf, p);
    }
    //strcat(buf, ")");
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

// 関数の引数リストを表す文字列をCの文法表記で返す。文字列はmallocされている。
char *get_func_args_str(const Node *node) {
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

static const char *get_NDtype_str(NDtype type) {
    #define NDTYPE_STR(t,val) if (t==val) return #val
    NDTYPE_STR(type,ND_UNDEF);
    NDTYPE_STR(type,ND_NOT);
    NDTYPE_STR(type,ND_MOD);
    NDTYPE_STR(type,ND_AND);
    NDTYPE_STR(type,ND_MUL);
    NDTYPE_STR(type,ND_PLUS);
    NDTYPE_STR(type,ND_MINUS);
    NDTYPE_STR(type,ND_DIV);
    NDTYPE_STR(type,ND_LT);
    NDTYPE_STR(type,ND_ASIGN);
    NDTYPE_STR(type,ND_GT);
    NDTYPE_STR(type,ND_XOR);
    NDTYPE_STR(type,ND_OR);
    NDTYPE_STR(type,ND_NUM);
    NDTYPE_STR(type,ND_STRING);
    NDTYPE_STR(type,ND_LOCAL_VAR);
    NDTYPE_STR(type,ND_GLOBAL_VAR);
    NDTYPE_STR(type,ND_CAST);
    NDTYPE_STR(type,ND_INC);
    NDTYPE_STR(type,ND_DEC);
    NDTYPE_STR(type,ND_INC_PRE);
    NDTYPE_STR(type,ND_DEC_PRE);
    NDTYPE_STR(type,ND_INDIRECT);
    NDTYPE_STR(type,ND_ADDRESS);
    NDTYPE_STR(type,ND_EQ);
    NDTYPE_STR(type,ND_NE);
    NDTYPE_STR(type,ND_LE);
    NDTYPE_STR(type,ND_LAND);
    NDTYPE_STR(type,ND_LOR);
    NDTYPE_STR(type,ND_TRI_COND);
    NDTYPE_STR(type,ND_PLUS_ASSIGN);
    NDTYPE_STR(type,ND_MINUS_ASSIGN);
    NDTYPE_STR(type,ND_LOCAL_VAR_DEF);
    NDTYPE_STR(type,ND_GLOBAL_VAR_DEF);
    NDTYPE_STR(type,ND_RETURN);
    NDTYPE_STR(type,ND_IF);
    NDTYPE_STR(type,ND_WHILE);
    NDTYPE_STR(type,ND_FOR);
    NDTYPE_STR(type,ND_BREAK);
    NDTYPE_STR(type,ND_CONTINUE);
    NDTYPE_STR(type,ND_BLOCK);
    NDTYPE_STR(type,ND_LIST);
    NDTYPE_STR(type,ND_FUNC_CALL);
    NDTYPE_STR(type,ND_FUNC_DEF);
    NDTYPE_STR(type,ND_FUNC_DECL);
    NDTYPE_STR(type,ND_VARARGS);
    NDTYPE_STR(type,ND_EMPTY);
    return "ND_???";
}

static void dump_node_indent(FILE *fp, const Node *node, const char *str, int indent) {
    fprintf(fp, "%*s", indent, "");
    if (str) fprintf(fp, "%s:", str);
    fprintf(fp, "Node[%p]:type=%s, name=%s, tp=%s, offset=%d, val=%ld", 
        node,
        get_NDtype_str(node->type),
        node->name?node->name:"",
        get_type_str(node->tp),
        node->offset, node->val);
    fprintf(fp, "\n");
    if (node->lhs) dump_node_indent(fp, node->lhs, "lhs=", indent+2);
    if (node->rhs) dump_node_indent(fp, node->rhs, "rhs=", indent+2);
}

void dump_node(const Node *node, const char *str) {
    dump_node_indent(stderr, node, str, 2);
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

