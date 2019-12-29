#include "emcc.h"


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
    "float",
    "doube",
    "long double",
    "struct",
    "union",
    "*",
    "[",
    "...",
    "func(",
    "NEST",
    };
static const char *SClassStr[] = {
    "", 
    "typedef",
    "auto ",
    "register ",
    "static ",
    "extern ",
    };
_Static_assert(NEST+1==sizeof(TypeStr)/sizeof(char*),"TypeStr");
_Static_assert(SC_EXTERN+1==sizeof(SClassStr)/sizeof(char*), "SClassStr");

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
    strcat_word(buf, SClassStr[tp->tmp_sclass]);
    if (tp->is_const) strcat_word(buf, "const");
    if (tp->is_unsigned&& tp->type!=BOOL) strcat_word(buf, "unsigned");
    strcat_word(buf, TypeStr[tp->type]);
    if ((tp->type==STRUCT || tp->type==ENUM) && tp->node) strcat_word(buf, tp->node->name);
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
char *get_node_type_str(const Node *node) {
    char *ret = get_type_str(node->tp);
    if (node->sclass && get_storage_class(node->tp)==SC_UNDEF) {
        char *tmp = malloc(strlen(ret)+32);
        sprintf(tmp, "%s%s", SClassStr[node->sclass], ret);
        ret = tmp;
    }
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
    ENUM2STR(ND_STRUCT_DEF);
    ENUM2STR(ND_UNION_DEF);
    ENUM2STR(ND_LOCAL_VAR);
    ENUM2STR(ND_GLOBAL_VAR);
    ENUM2STR(ND_CAST);
    ENUM2STR(ND_INC);
    ENUM2STR(ND_DEC);
    ENUM2STR(ND_INC_PRE);
    ENUM2STR(ND_DEC_PRE);
    ENUM2STR(ND_NEG);
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
    ENUM2STR(ND_MUL_ASSIGN);
    ENUM2STR(ND_DIV_ASSIGN);
    ENUM2STR(ND_MOD_ASSIGN);
    ENUM2STR(ND_SHIFTR_ASSIGN);
    ENUM2STR(ND_SHIFTL_ASSIGN);
    ENUM2STR(ND_AND_ASSIGN);
    ENUM2STR(ND_XOR_ASSIGN);
    ENUM2STR(ND_OR_ASSIGN);
    ENUM2STR(ND_LOCAL_VAR_DEF);
    ENUM2STR(ND_GLOBAL_VAR_DEF);
    ENUM2STR(ND_MEMBER_DEF);
    ENUM2STR(ND_RETURN);
    ENUM2STR(ND_IF);
    ENUM2STR(ND_WHILE);
    ENUM2STR(ND_FOR);
    ENUM2STR(ND_BREAK);
    ENUM2STR(ND_CONTINUE);
    ENUM2STR(ND_BLOCK);
    ENUM2STR(ND_LIST);
    ENUM2STR(ND_INIT_LIST);
    ENUM2STR(ND_FUNC_CALL);
    ENUM2STR(ND_FUNC_DEF);
    ENUM2STR(ND_FUNC_DECL);
    ENUM2STR(ND_FUNC_END);
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
    ENUM2STR(STRUCT);
    ENUM2STR(UNION);
    ENUM2STR(PTR);
    ENUM2STR(ARRAY);
    ENUM2STR(FUNC);
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

static int f_dump_enum_lst = 0;   //ND_ENUM_DEFのlst(ND_ENUM)をダンプする

static void dump_type_indent(FILE *fp, const Type *tp, const char *str, int indent);

static void dump_node_indent(FILE *fp, const Node *node, const char *str, int indent) {
    fprintf(fp, "#%*s", indent, "");
    if (str) fprintf(fp, "%s:", str);
    if (node==NULL) {
        fprintf(fp, "Node[null]\n");
        return;        
    }
    fprintf(fp, "Node[%p]:type=%s, name=\"%s\"", 
        (void*)node,
        get_NDtype_str(node->type),
        node->name?node->name:"");
    if (node->disp_name) fprintf(fp, "(%s)", node->disp_name);
    if (node->sclass) fprintf(fp, ", sclass==%s", get_StorageClass_str(node->sclass));
    fprintf(fp, ", tp=%s, offset=%d, index=%d, val=%ld, unused=%d, line=%d\n", 
        get_node_type_str(node),
        node->offset, node->index, node->val, node->unused, node->token->info.line);
    if (g_dump_type && (node->type==ND_LOCAL_VAR_DEF||node->type==ND_GLOBAL_VAR_DEF)) {
        g_dump_type = 0;
        dump_type_indent(fp, node->tp, NULL, indent+5);
        g_dump_type = 1;
    }
    if (node->lhs) dump_node_indent(fp, node->lhs, "lhs=", indent+2);
    if (node->rhs) dump_node_indent(fp, node->rhs, "rhs=", indent+2);
    if (node->lst && indent<20 && !(node->tp && node->tp->type==PTR) && (node->type!=ND_ENUM_DEF || f_dump_enum_lst)) {
        char buf[16];
        Vector *lists = node->lst;
        Node **nodes = (Node**)lists->data;
        for (int i=0; i < lists->len; i++) {
            sprintf(buf, "lst[%d]", i);
            dump_node_indent(fp, nodes[i], buf, indent+2);
        }
    }
    if (node->type==ND_TYPE_DECL) dump_type_indent(fp, node->tp, NULL, indent+2);
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
        get_StorageClass_str(tp->tmp_sclass),
        tp->array_size);
    if (tp->node) dump_node_indent(fp, tp->node, NULL, indent+2);
    if (tp->ptr_of) dump_type_indent(fp, tp->ptr_of, NULL, indent+2);
}

void dump_type(const Type *tp, const char *str) {
    dump_type_indent(stderr, tp, str, 1);
}

void dump_symbol(int idx, const char *str) {
    if (idx<0) idx = symbol_stack->len-1;
    if (str) fprintf(stderr, "# == symbol[%d]: %s ==\n", idx, str);
    Map *symbol_map = stack_get(symbol_stack, idx); 
    int size = lst_len(symbol_map->vals);
    char **names = (char**)symbol_map->keys->data;
    Node **nodes = (Node**)symbol_map->vals->data;
    char buf[128];
    for (int i=0; i<size; i++) {
        sprintf(buf, "symbol[%d]=\"%s\"", i, names[i]);
        dump_node_indent(stderr, nodes[i], buf, 1);
    }
}

void dump_tagname(void) {
    Map *tagname_map = stack_top(tagname_stack); 
    int size = lst_len(tagname_map->vals);
    char **names = (char**)tagname_map->keys->data;
    Node **nodes = (Node**)tagname_map->vals->data;
    char buf[128];
    for (int i=0; i<size; i++) {
        sprintf(buf, "tganame[%d]=\"%s\"", i, names[i]);
        dump_node(nodes[i], buf);
    }
}
