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

// ダンプ関数 ----------------------------------------
static const char *TypeStr[] = {
    "Nul",
    "void",
    "_Bool",
    "char",
    "short",
    "int",
    "long",
    "long long",
    "enum",
    "*",
    "[",
    "...",
    "func(",
    "CONST",
    };
static const char *SClassStr[] = {
    "", 
    "auto ",
    "register ",
    "static ",
    "extern ",
    "typedef",
    };
_Static_assert(NEST==sizeof(TypeStr)/sizeof(char*),"TypeStr");
_Static_assert(SC_TYPEDEF+1==sizeof(SClassStr)/sizeof(char*), "SClassStr");

static void strcat_word(char *buf, const char *str) {
    char last_char = 0;
    int len;
    last_char = ((len=strlen(buf))>0) ? buf[len-1] : 0; 
    if (is_alnum(last_char) && is_alnum(*str)) strcat(buf, " ");
    strcat(buf, str);
}
//bufに対してTypeをダンプする
static void sprint_type(char *buf, const Type *tp) {
    const char *str = TypeStr[tp->type];
    if (tp->is_unsigned && tp->type!=BOOL) strcat_word(buf, "unsigned");
    if (tp->is_const) strcat_word(buf, "const");
    strcat_word(buf, str);
    if (tp->type==ARRAY) {
        char tmp[20];
        if (tp->array_size>=0) sprintf(tmp, "%ld]", tp->array_size);
        else                   sprintf(tmp ,"]");
        strcat(buf, tmp);
    } else if (tp->type==FUNC) {
        if (tp->node) strcat(buf, get_func_args_str(tp->node->lhs));
        strcat(buf, ")");
    } else if (tp->type==ENUM) {
        strcat_word(buf, tp->node->name?tp->node->name:"(anonymous)");
    }
    if (tp->ptr_of) sprint_type(buf, tp->ptr_of);
}

//bufに対して型を表す文字列をCの文法で生成する
static void sprintC_type(char *buf, const Type *tp) {
    if (tp->ptr_of) sprintC_type(buf, tp->ptr_of);
    strcat_word(buf, SClassStr[tp->sclass]);
    if (tp->is_unsigned&& tp->type!=BOOL) strcat_word(buf, "unsigned");
    if (tp->is_const) strcat_word(buf, "const");
    strcat_word(buf, TypeStr[tp->type]);
    if (tp->type==ARRAY) {
        char tmp[20];
        if (tp->array_size>=0) sprintf(tmp, "%ld]", tp->array_size);
        else                   sprintf(tmp ,"]");
        strcat(buf, tmp);
    }
}

// 型を表す文字列を返す。文字列はmallocされている。
char *get_type_str(const Type *tp) {
    char buf[1024];
    const Type *p;
    if (tp==NULL) return "null";
    buf[0] = 0;
    //ARRAY[10]->ARRAY[2]->PTR->INT
    //ARRAY以外を深さ優先で先に処理する
    for (p=tp; p->type==ARRAY; p=p->ptr_of);
    if (0) {
        sprint_type(buf, p);
    } else {
        sprintC_type(buf, p);
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
        len += sprintf(buf+len, "%s", get_type_str(arg_nodes[i]->tp));
        if (arg_nodes[i]->name)
            len += sprintf(buf+len, " %s", arg_nodes[i]->name);
        if (i<size-1) len += sprintf(buf+len, ", ");
    }
    char *ret = malloc(strlen(buf)+1);
    strcpy(ret, buf);
    return ret;
}

#define ENUM2STR(val) case val: return #val
const char *get_NDtype_str(NDtype type) {
    switch (type) {
    ENUM2STR(ND_UNDEF);
    ENUM2STR(ND_NOT);
    ENUM2STR(ND_MOD);
    ENUM2STR(ND_AND);
    ENUM2STR(ND_MUL);
    ENUM2STR(ND_PLUS);
    ENUM2STR(ND_MINUS);
    ENUM2STR(ND_DIV);
    ENUM2STR(ND_LT);
    ENUM2STR(ND_ASSIGN);
    ENUM2STR(ND_GT);
    ENUM2STR(ND_XOR);
    ENUM2STR(ND_OR);
    ENUM2STR(ND_NUM);
    ENUM2STR(ND_STRING);
    ENUM2STR(ND_IDENT);
    ENUM2STR(ND_TYPE_DECL);
    ENUM2STR(ND_ENUM_DEF);
    ENUM2STR(ND_ENUM);
    ENUM2STR(ND_TYPEDEF);
    ENUM2STR(ND_LOCAL_VAR);
    ENUM2STR(ND_GLOBAL_VAR);
    ENUM2STR(ND_CAST);
    ENUM2STR(ND_INC);
    ENUM2STR(ND_DEC);
    ENUM2STR(ND_INC_PRE);
    ENUM2STR(ND_DEC_PRE);
    ENUM2STR(ND_INDIRECT);
    ENUM2STR(ND_ADDRESS);
    ENUM2STR(ND_EQ);
    ENUM2STR(ND_NE);
    ENUM2STR(ND_LE);
    ENUM2STR(ND_LAND);
    ENUM2STR(ND_LOR);
    ENUM2STR(ND_TRI_COND);
    ENUM2STR(ND_PLUS_ASSIGN);
    ENUM2STR(ND_MINUS_ASSIGN);
    ENUM2STR(ND_LOCAL_VAR_DEF);
    ENUM2STR(ND_GLOBAL_VAR_DEF);
    ENUM2STR(ND_RETURN);
    ENUM2STR(ND_IF);
    ENUM2STR(ND_WHILE);
    ENUM2STR(ND_FOR);
    ENUM2STR(ND_BREAK);
    ENUM2STR(ND_CONTINUE);
    ENUM2STR(ND_BLOCK);
    ENUM2STR(ND_LIST);
    ENUM2STR(ND_FUNC_CALL);
    ENUM2STR(ND_FUNC_DEF);
    ENUM2STR(ND_FUNC_DECL);
    ENUM2STR(ND_VARARGS);
    ENUM2STR(ND_EMPTY);
    default: return "ND_???";
    }
}
static const char *get_TPType_str(TPType type) {
    switch (type) {
    ENUM2STR(VOID);
    ENUM2STR(CHAR);
    ENUM2STR(SHORT);
    ENUM2STR(INT);
    ENUM2STR(LONG);
    ENUM2STR(LONGLONG);
    ENUM2STR(ENUM);
    ENUM2STR(PTR);
    ENUM2STR(ARRAY);
    ENUM2STR(FUNC);
    ENUM2STR(CONST);
    ENUM2STR(NEST);
    default: return "???";
    }
}
static const char *get_StorageClass_str(StorageClass type) {
    switch (type) {
    ENUM2STR(SC_UNDEF);
    ENUM2STR(SC_AUTO);
    ENUM2STR(SC_REGISTER);
    ENUM2STR(SC_STATIC);
    ENUM2STR(SC_EXTERN);
    ENUM2STR(SC_TYPEDEF);
    default: return "SC_???";
    }
}

static void dump_node_indent(FILE *fp, const Node *node, const char *str, int indent) {
    fprintf(fp, "#%*s", indent, "");
    if (str) fprintf(fp, "%s:", str);
    if (node==NULL) {
        fprintf(fp, "Node[null]\n");
        return;        
    }
    fprintf(fp, "Node[%p]:type=%s, name=\"%s\", tp=%s, offset=%d, val=%ld, unused=%d\n", 
        (void*)node,
        get_NDtype_str(node->type),
        node->name?node->name:"",
        get_type_str(node->tp),
        node->offset, node->val, node->unused);
    if (node->lhs) dump_node_indent(fp, node->lhs, "lhs=", indent+2);
    if (node->rhs) dump_node_indent(fp, node->rhs, "rhs=", indent+2);
    if (node->lst && indent<10) {
        char buf[16];
        Vector *lists = node->lst;
        Node **nodes = (Node**)lists->data;
        for (int i=0; i < lists->len; i++) {
            sprintf(buf, "lst[%d]", i);
            dump_node_indent(fp, nodes[i], buf, indent+2);
        }
    }
}

void dump_node(const Node *node, const char *str) {
    if (str) fprintf(stderr, "# == %s ==\n", str);
    dump_node_indent(stderr, node, "top=", 1);
}

static void dump_type_indent(FILE *fp, const Type *tp, const char *str, int indent) {
    fprintf(fp, "#%*s", indent, "");
    if (str) fprintf(fp, "%s:", str);
    fprintf(fp, "Type[%p] (%s) type=%s, is_unsigned=%d, is_const=%d, sclass=%s, array_size=%ld\n",
        (void*)tp,
        get_type_str(tp),
        get_TPType_str(tp->type), 
        tp->is_unsigned,
        tp->is_const,
        get_StorageClass_str(tp->sclass),
        tp->array_size);
    if (tp->node) dump_node_indent(fp, tp->node, NULL, indent+2);
    if (tp->ptr_of) dump_type_indent(fp, tp->ptr_of, NULL, indent+2);
}

void dump_type(const Type *tp, const char *str) {
    dump_type_indent(stderr, tp, str, 1);
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

