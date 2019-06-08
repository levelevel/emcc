#include "9cc.h"

//現在のトークン（エラー箇所）の入力文字列
#define input_str() (tokens[token_pos]->input)
//現在のトークンの型が引数と一致しているか
#define token_type() (tokens[token_pos]->type)
#define token_is(_tp) (token_type()==(_tp))
#define token_is_type() (TK_CHAR<=token_type() && token_type()<=TK_CONST)
#define next_token_type() (tokens[token_pos+1]->type)
#define next_token_is(_tp) (next_token_type()==(_tp))
#define next_token_is_type() (TK_CHAR<=next_token_type() && next_token_type()<=TK_CONST)

//トークンの種類を定義
typedef struct {
    char *name;
    int len;
    TKtype type;
} TokenDef;

//トークンの終わり判定が不要なもの
TokenDef TokenLst1[] = {
    {"++", 2, TK_INC},
    {"--", 2, TK_DEC},
    {"==", 2, TK_EQ},
    {"!=", 2, TK_NE},
    {">=", 2, TK_GE},
    {"<=", 2, TK_LE},
    {"&&", 2, TK_LAND},
    {"||", 2, TK_LOR},
    {"+=", 2, TK_PLUS_ASSIGN},
    {"-=", 2, TK_MINUS_ASSIGN},
    {"!",  1, '!'},
    {"%",  1, '%'},
    {"&",  1, '&'},
    {"(",  1, '('},
    {")",  1, ')'},
    {"*",  1, '*'},
    {"+",  1, '+'},
    {",",  1, ','},
    {"-",  1, '-'},
    {"/",  1, '/'},
    {":",  1, ':'},
    {";",  1, ';'},
    {"<",  1, '<'},
    {"=",  1, '='},
    {">",  1, '>'},
    {"?",  1, '?'},
    {"[",  1, '['},
    {"]",  1, ']'},
    {"^",  1, '^'},
    {"{",  1, '{'},
    {"|",  1, '|'},
    {"}",  1, '}'},
    {NULL, 0, 0}
};

//トークンの終わりをis_alnum()で判定するもの
TokenDef TokenLst2[] = {
    {"char",     4, TK_CHAR},
    {"short",    5, TK_SHORT},
    {"int",      3, TK_INT},
    {"long",     4, TK_LONG},
    {"signed",   6, TK_SIGNED},
    {"unsigned", 8, TK_UNSIGNED},
//  {"auto",     4, TK_AUTO},
//  {"register", 8, TK_REGISTER},
    {"static",   6, TK_STATIC},
    {"extern",   6, TK_EXTERN},
//  {"volatile", 8, TK_VOLATILE},
//  {"restrict", 8, TK_RESTRICT},
    {"const",    5, TK_CONST},
    {"return",   6, TK_RETURN},
    {"if",       2, TK_IF},
    {"else",     4, TK_ELSE},
    {"while",    5, TK_WHILE},
    {"for",      3, TK_FOR},
    {"break",    5, TK_BREAK},
    {"continue", 8, TK_CONTINUE},
    {"sizeof",   6, TK_SIZEOF},
    {"typeof",   6, TK_TYPEOF},
    {"_Alignof", 8, TK_ALIGNOF},
    {NULL, 0, 0}
};

static Token *new_token(TKtype type, char *input) {
    Token *token = calloc(1, sizeof(Token));
    token->type = type;
    token->input = input;
    vec_push(token_vec, token);
    return token;
}

//識別子に使用できる文字
static int is_alnum(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') || (c == '_');
}
//識別子の先頭に使用できる文字
static int is_alpha(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_');
}

//識別子の文字列を返す。
static char*token_ident(const char*ptop) {
    const char *p = ptop+1;
    int len = 1;
    while (is_alnum(*p)) {
        p++;
        len++;
    }
    char *name = malloc(len+1);
    strncpy(name, ptop, len);
    name[len] = 0;
    return name;
}

//文字列リテラルの文字列を返す。
static char*token_string(const char*ptop) {
    const char *p = ptop+1;
    int len = 1;
    while ((*p)!='"') {
        p++;
        len++;
    }
    char *name = malloc(len+1);
    strncpy(name, ptop, len);
    name[len] = 0;
    return name;
}

// pが指している文字列をトークンに分割してtokensに保存する
void tokenize(char *p) {
    Token *token;
    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        //行コメントをスキップ
        if (strncmp(p, "//", 2)==0) {
            p += 2;
            while (*p && *p!='\n') p++;
            continue;
        }

        //ブロックコメントをスキップ
        if (strncmp(p, "/*", 2)==0) {
            char *q = strstr(p + 2, "*/");
            if (!q) error_at(p, "コメントが閉じられていません");
            p = q + 2;
            continue;
        }

        for (TokenDef *tk = TokenLst1; tk->name; tk++) {
            if (strncmp(p, tk->name, tk->len)==0) {
                token = new_token(tk->type, p);
                p += tk->len;
                goto NEXT_LOOP;
            }
        }
        for (TokenDef *tk = TokenLst2; tk->name; tk++) {
            if (strncmp(p, tk->name, tk->len)==0 && !is_alnum(p[tk->len])) {
                token = new_token(tk->type, p);
                p += tk->len;
                goto NEXT_LOOP;
            }
        }

        if (is_alpha(*p)) {         //識別子
            token = new_token(TK_IDENT, p);
            token->str = token_ident(p);
            p += strlen(token->str);
        } else if (isdigit(*p)) {   //数値
            token = new_token(TK_NUM, p);
            if (strncmp(p, "0x", 2)==0 || strncmp(p, "0X", 2)==0) {
                token->val = strtoul(p, &p, 0); //16進
            //  fprintf(stderr, "strtoul=%lu,%lx\n", token->val, token->val);
            } else {
                token->val = strtol(p, &p, 0);  //10進、8進
            //  fprintf(stderr, "strtol=%ld,%lx\n", token->val, token->val);
            }
        } else if (*p == '"') {     //文字列
            token = new_token(TK_STRING, p);
            token->str = token_string(++p);
            p += strlen(token->str) + 1;
        } else if (*p == '\'') {    //文字
            token = new_token(TK_NUM, p++);
            token->val = *p++;
            if (*p++ != '\'') error_at(p, "トークナイズエラー");
        } else {
            error_at(p, "トークナイズエラー");
            exit(1);
        }
        NEXT_LOOP:;
    }
    token = new_token(TK_EOF, p);
    //dump_tokens();
}

void dump_tokens(void) {
    Token **tk = (Token**)token_vec->data;
    Token *tp;
    for (int i=0; i<token_vec->len; i++) {
        tp = tk[i];
        if (tp->type < TK_NUM) {
            printf("%d: type='%c', input='%s'\n", i, tp->type, tp->input);
        } else {
            printf("%d: type='%d', val=%ld, input='%s'\n", i, tp->type, tp->val, tp->input);
        }
    }
}

//次のトークンが期待した型かどうかをチェックし、
//期待した型の場合だけ入力を1トークン読み進めて真を返す
static int consume(TKtype type) {
    if (tokens[token_pos]->type != type) return 0;
    token_pos++;
    return 1;
}

//次のトークンが識別子(TK_IDENT)かどうかをチェックし、
//その場合はnameを取得し、入力を1トークン読み進めて真を返す
static int consume_ident(char**name) {
    if (tokens[token_pos]->type != TK_IDENT) return 0;
    *name = tokens[token_pos]->str;
    token_pos++;
    return 1;
}

//型のサイズ
// - 配列の場合、要素のサイズ*要素数
// - 構造体の場合、アラインメントで単位切り上げ
long size_of(const Type *tp) {
    assert(tp);
    switch (tp->type) {
    case CHAR:     return sizeof(char);
    case SHORT:    return sizeof(short);
    case INT:      return sizeof(int);
    case LONG:     return sizeof(long);
    case LONGLONG: return sizeof(long long);
    case PTR:      return sizeof(void*);
    case ARRAY:    return tp->array_size * size_of(tp->ptr_of);
    }
    _ERROR_;
    return -1;
}

//型のアラインメント
// - 配列の場合、要素のアラインメント
// - 構造体の場合、メンバー内の最大のアラインメント
int align_of(const Type *tp) {
    assert(tp);
    if (tp->type==ARRAY) return align_of(tp->ptr_of);
    return size_of(tp);
}

//ノードのタイプが等しいかどうかを判定する
static int node_type_eq(const Type *tp1, const Type *tp2) {
    if ((tp1->type==PTR && tp2->type==ARRAY) ||
        (type_is_integer(tp1) && type_is_integer(tp2))) {
        ;   //一致とみなす（tp1=tp2の代入前提）
    } else {
        if (tp1->type != tp2->type) return 0;
    }  
    if (tp1->ptr_of) return node_type_eq(tp1->ptr_of, tp2->ptr_of);
    return 1;
}

//ローカル変数のRBPからのoffset（バイト数）を返し、var_stack_sizeを更新する。
static int get_var_offset(const Type *tp) {
    int size = size_of(tp);
    int align_size = align_of(tp);
    cur_funcdef->var_stack_size += size;
    // アラインメント（align_sizeバイト単位に切り上げ）
    if (size>0) cur_funcdef->var_stack_size = (cur_funcdef->var_stack_size + (size-1))/align_size * align_size;
    return cur_funcdef->var_stack_size;
}

//関数定義のroot生成
static Funcdef *new_funcdef(void) {
    Funcdef * funcdef;
    funcdef = calloc(1, sizeof(Funcdef));
    funcdef->ident_map = new_map();
    return funcdef;
}

//型情報の生成
static Type* new_type_ptr(Type*ptr) {
    Type *tp = calloc(1, sizeof(Type));
    tp->type = PTR;
    tp->ptr_of = ptr;
    return tp;
}

static Type* new_type_array(Type*ptr, size_t size) {
    Type *tp = calloc(1, sizeof(Type));
    tp->type = ARRAY;
    tp->ptr_of = ptr;
    tp->array_size = size;
    return tp;
}

static Type* new_type(int type, int is_unsigned) {
    Type *tp = calloc(1, sizeof(Type));
    tp->type = type;
    tp->is_unsigned = is_unsigned;
    return tp;
}

//抽象構文木の生成（演算子）
static Node *new_node(int type, Node *lhs, Node *rhs, Type *tp, char *input) {
    Node *node = calloc(1, sizeof(Node));
    node->type = type;
    node->lhs  = lhs;
    node->rhs  = rhs;
    node->tp   = tp;
    node->input = input;
    return node;
}

//抽象構文木の生成（数値）
static Node *new_node_num(long val, char *input) {
    NDtype type = (val>UINT_MAX || val<INT_MIN) ? LONG : INT;
    Node *node = new_node(ND_NUM, NULL, NULL, new_type(type, 0), input);
    node->val = val;
//    fprintf(stderr, "val=%ld\n", val);
    return node;
}

//未登録の変数であれば登録する
static void regist_var_def(Node *node) {
    char *name = node->name;
    if (cur_funcdef && !type_is_extern(node->tp)) {  //関数内かつexternでなければローカル変数
        if (map_get(cur_funcdef->ident_map, name, NULL)==0) {
            Vardef *vardef = calloc(1, sizeof(Vardef));
            vardef->name = name;
            vardef->node = node;
            if (type_is_static(node->tp)) {
                vardef->offset = global_index++;
            } else {
                vardef->offset = get_var_offset(node->tp);
            }
            node->type = ND_LOCAL_VAR_DEF;
            map_put(cur_funcdef->ident_map, name, vardef);
        } else {
            error_at(node->input, "'%s'はローカル変数の重複定義です", name);
        }
    } else {            //グローバル変数
        if (map_get(global_vardef_map, name, NULL)==0) {
            Vardef *vardef = calloc(1, sizeof(Vardef));
            vardef->name = name;
            vardef->node = node;
            node->type = ND_GLOBAL_VAR_DEF;
            map_put(global_vardef_map, name, vardef);
        } else {
            error_at(node->input, "'%s'はグローバルの重複定義です", name);
        }
    }
}

//抽象構文木の生成（変数定義）
static Node *new_node_var_def(char *name, Type*tp, char *input) {
    Node *node = new_node(0, NULL, NULL, tp, input);
    node->name = name;
    regist_var_def(node);
    return node;
}

//抽象構文木の生成（文字列リテラル）
static Node *new_node_string(char *string, char *input) {
    Type *tp = new_type_array(new_type(CHAR, 0), strlen(string)+1);
    Node *node = new_node(ND_STRING, NULL, NULL, tp, input);
    node->val = string_vec->len;    //インデックス
    vec_push(string_vec, string);

    return node;
}

//抽象構文木の生成（識別子：ローカル変数・グローバル変数）
static Node *new_node_var(char *name, char *input) {
    Node *node;
    NDtype type;
    Vardef *vardef;

    //定義済みの変数であるかをチェック
    if (cur_funcdef && map_get(cur_funcdef->ident_map, name, (void**)&vardef)!=0) {
        type = ND_LOCAL_VAR;
    } else if (map_get(global_vardef_map, name, (void**)&vardef)!=0) {
        type = ND_GLOBAL_VAR;
    } else {
        error_at(tokens[token_pos-1]->input, "'%s'は未定義の変数です", name);
    }

    node = new_node(type, NULL, NULL, vardef->node->tp, input);
    node->name = name;

    return node;
}

//抽象構文木の生成（関数コール）
static Node *new_node_func_call(char *name, char *input) {
    Funcdef *funcdef;
    //未登録の関数名であれば仮登録する。
    //登録済みであればタイプを取得する
    //もし本登録済みであれば正しい型名が取得できる。引数はチェックしていない。
    if (map_get(func_map, name, (void**)&funcdef)==0) {
        funcdef = new_funcdef();
        funcdef->name = name;
        funcdef->node = NULL;
        funcdef->tp = new_type(INT, 0);  //暫定値
        map_put(func_map, name, funcdef);
    }

    Node *node = new_node(ND_FUNC_CALL, NULL, NULL, funcdef->tp, input);
    node->name = name;
//  node->lhs    //引数リスト

    return node;
}

//抽象構文木の生成（関数定義）
static Node *new_node_func_def(char *name, Type *tp, char *input) {
    Node *node = new_node(ND_FUNC_DEF, NULL, NULL, tp, input);
    node->name = name;
//  node->lhs       //引数リスト
//  node->rhs       //ブロック

    cur_funcdef = new_funcdef();
    cur_funcdef->tp = tp;
    cur_funcdef->node = node;
    cur_funcdef->name = node->name;
    map_put(funcdef_map, node->name, cur_funcdef);

    //未登録または仮登録の関数名であれば登録する
    Funcdef *funcdef;
    if (map_get(func_map, name, (void**)&funcdef)==0 ||
        funcdef->node == NULL)  {
        map_put(func_map, name, cur_funcdef);
    }
    return node;
}

//抽象構文木の生成（空文）
static Node *new_node_empty(char *input) {
    Node *node = new_node(ND_EMPTY, NULL, NULL, NULL, input);
    return node;
}

//抽象構文木の生成（ブロック）
static Node *new_node_block(char *input) {
    Node *node = new_node(ND_BLOCK, NULL, NULL, NULL, input);
    node->lst  = new_vector();
    return node;
}

//抽象構文木の生成（コンマリスト）
//型は最後の要素の型であるべきだがここでは設定しない
static Node *new_node_list(Node *item, char *input) {
    Node *node = new_node(ND_LIST, NULL, NULL, NULL, input);
    node->lst  = new_vector();
    if (item) vec_push(node->lst, item);
    return node;
}

/*  文法：
- A.2.4 External Definitions
    translation_unit        = external_declaration*
    external_declaration    = function_definition | declaration
    function_definition     = declaration_specifiers declarator "(" func_arg* ")" compound_statement
- A.2.2 Declarations
    declaration             = declaration_specifiers init_declarator ( "," init_declarator )* ";"
    declaration_specifiers  = "typeof" "(" identifier ")"
                            | type_specifier         declaration_specifiers*
                            | strage_class_specifier declaration_specifiers*
                            | type_qualifier         declaration_specifiers*
    init_declarator         = declarator ( "=" initializer )?
    strage_class_specifier  = "static" | "extern"
    type_specifier          = "char" | "short" | "int" | "long" | "signed" | "unsigned"
    type_qualifier          = "const"
    declarator              = pointer* direct_declarator
    direct_declarator       = identifier | "(" declarator ")"
                            = direct_declarator "[" assign? "]"
    initializer             = assign
                            | "{" init_list "}"
                            | "{" init_list "," "}"
    init_list               = initializer
                            | init_list "," initializer
    array_def   = "[" expr? "]" ( "[" expr "]" )*
    type_ptr    = declaration_specifiers pointer*
    pointer     = ( "*" )*
    func_arg    = type_ptr identifier ( "," func_arg )
- A.2.3 Statements
    statement   = compound_statement
                | declaration
                | return" expr ";"
                | "if" "(" expr ")" ( "else" expr )?
                | "while" "(" expr ")"
                | "for" "(" expr? ";" expr? ";" expr? ")" statement
                | "break"
                | "continue"
    compound_statement      = "{" eclaration | statement* "}"
- A.2.1 Expressions
    expr        = assign ( "," assign )* 
    assign      = tri_cond ( "=" assign | "+=" assign | "-=" assign )*
    tri_cond    = logical_or "?" expr ":" tri_cond
    logical_or  = logical_and ( "||" logical_and )*
    logical_and = bitwise_or ( "&&" bitwise_or )*
    bitwise_or  = ex_or ( "&" ex_or )*
    ex_or       = bitwise_and ( "^" bitwise_and )*
    bitwise_and = equality ( "&" equality )*
    equality    = relational ( "==" relational | "!=" relational )*
    relational  = add ( "<" add | "<=" add | ">" add | ">=" add )*
    add         = mul ( "+" mul | "-" mul )*
    mul         = unary ( "*" unary | "/" unary | "%" unary )*
    unary       = post_unary
                | ( "+" | "-" |  "!" | "*" | "&" ) unary
                | ( "++" | "--" )? unary
                | "sizeof" unary
                | "sizeof" "(" type_ptr array_def? ")"
                | "_Alignof" "(" type_ptr array_def? ")"
    post_unary  = term ( "++" | "--" | "[" expr "]")?
    term        = num
                | string
                | identifier
                | identifier "(" expr? ")"   //関数コール
                |  "(" expr ")"
*/
static Node *external_declaration(void);
static Node *function_definition(Type *tp, char *name);
static Node *declaration(Type *tp, char *name);
static Node *init_declarator(Type *decl_spec, Type *tp, char *name);
static Type *declarator(Type *decl_spec, Type *tp, char **name);
static Type *direct_declarator(Type *tp, char **name);
static Node *initializer(void);
static Node *init_list(void);
static Type *type_ptr(void);
static Type *pointer(Type *tp);
static Type *declaration_specifiers(void);
static Type *array_def(Type *tp);
static Node *func_arg(void);
static Node *statement(void);
static Node *compound_statement(void);
static Node *expr(void);
static Node *assign(void);
static Node *tri_cond(void);
static Node *equality(void);
static Node *logical_or(void);
static Node *logical_and(void);
static Node *bitwise_or(void);
static Node *ex_or(void);
static Node *bitwise_and(void);
static Node *relational(void);
static Node *add(void);
static Node *mul(void); 
static Node *unary(void);
static Node *post_unary(void);
static Node *term(void); 

void translation_unit(void) {
    while (!token_is(TK_EOF)) {
        external_declaration();
        cur_funcdef = NULL;
    }
}

//    external_declaration    = function_definition | declaration
//    function_definition     = declaration_specifiers declarator "(" func_arg* ")" "{" statement* "}"
//    declaration             = declaration_specifiers init_declarator ( "," init_declarator )* ";"
static Node *external_declaration(void) {
    Node *node;
    Type *tp;
    char *name;

    // 関数定義：int * foo (){}
    // 変数定義：int * ptr ;
    //                    ^ここまで読まないと区別がつかない
    if (token_is_type()) {
        tp = declaration_specifiers();
        tp = pointer(tp);
        if (!consume_ident(&name)) error_at(input_str(), "型名の後に識別名がありません");
        if (consume('(')) {
            node = function_definition(tp, name);
        } else {
            node = declaration(tp, name);
            //if (!consume(';')) error_at(input_str(), "; がありません");
        }
    } else {
        error_at(input_str(), "関数・変数の定義がありません");
    }
    return node;
}

//関数の定義: lhs=引数(ND_LIST)、rhs=ブロック(ND_BLOCK)
//    function_definition     = declaration_specifiers declarator "(" func_arg* ")" "{" statement* "}"
static Node *function_definition(Type *tp, char *name) {
    Node *node;
    char *input = input_str();
    // "("まで処理済み
    node = new_node_func_def(name, tp, input);
    node->lhs = func_arg();
    if (!consume(')')) error_at(input_str(), "関数定義の閉じカッコがありません");
    node->rhs = compound_statement();

    return node;
}

//    declaration             = declaration_specifiers init_declarator ( "," init_declarator )* ";"
//    init_declarator         = declarator ( "=" initializer )?
//    declarator              = pointer* identifier array_def?
//declaration_specifiers, pointer, identifierまで先読み済み
static Node *declaration(Type *tp, char *name) {
    Node *node;
    Type *decl_spec;

    if (tp==NULL) {
        //declaration_specifiers, pointer, identifierを先読みしていなければdecl_specを読む
        decl_spec = declaration_specifiers();
    } else {
        // 変数定義：int * ptr [5], *idx, ...;
        //                   ^ここまで先読み済みであるので、ポインタを含まないdecl_spec（int）を取り出す
        decl_spec = tp;
        while (decl_spec->ptr_of) decl_spec = decl_spec->ptr_of;
    }

    node = init_declarator(decl_spec, tp, name);

    if (consume(',')) {
        Node *last_node;
        node = new_node_list(node, node->input);
        Vector *lists = node->lst;
        vec_push(lists, last_node=init_declarator(decl_spec, NULL, NULL));
        while (consume(',')) {
            vec_push(lists, last_node=init_declarator(decl_spec, NULL, NULL));
        }
        node->tp = last_node->tp;
    }
    if (!consume(';')) error_at(input_str(), "; がありません");
    return node;
}

//    init_declarator         = declarator ( "=" initializer )?
//    declarator              = pointer* identifier array_def?
//declaration_specifiers, pointer, identifierまで先読みの可能性あり
static Node *init_declarator(Type *decl_spec, Type *tp, char *name) {
    Node *node, *rhs=NULL;
    char *input = input_str();

    tp = declarator(decl_spec, tp, &name);

    //初期値: rhsに初期値を設定する
    if (consume('=')) {
        if (type_is_extern(tp)) error_at(input, "extern変数は初期化できません");
        //node->rhsに変数=初期値の形のノードを設定する。->そのまま初期値設定のコード生成に用いる
        rhs = initializer();
        long val;
        if (tp->type==ARRAY) {  //左辺が配列
            if (rhs->tp->type==ARRAY) {
                //文字列リテラルで初期化できるのはcharの配列だけ
                if (rhs->type==ND_STRING && tp->ptr_of->type!=CHAR)
                    error_at(rhs->input, "%sを文字列リテラルで初期化できません", get_type_str(tp));
                if (tp->array_size<0) {
                    tp->array_size = rhs->tp->array_size;
                }
            } else if (rhs->type==ND_LIST) {
                if (tp->array_size<0) {
                    tp->array_size = rhs->lst->len;
                }
            } else {
                error_at(rhs->input, "配列の初期値が配列形式になっていません");
            }
        }
        if (node_is_const(rhs, &val)) {
            rhs = new_node_num(val, rhs->input);
        }
    }

    node = new_node_var_def(name, tp, input); //ND_(LOCAL|GLOBAL)_VAR_DEF
    if (rhs) node->rhs = new_node('=', new_node_var(name, input), rhs, tp, input);

    //初期値のないサイズ未定義のARRAYはエラー
    //externの場合はOK
    if (node->tp->type==ARRAY && node->tp->array_size<0 && !type_is_extern(node->tp) &&
        (node->rhs==NULL || node->rhs->type!='='))
        error_at(input_str(), "配列のサイズが未定義です");

    //グローバルスカラー変数の初期値は定数または固定アドレスでなければならない
    if (node->tp->type!=ARRAY && node->type==ND_GLOBAL_VAR_DEF && node->rhs &&
        node->rhs->rhs->type!=ND_STRING) {  //文字列リテラルはここではチェックしない
        long val;
        Node *var=NULL;
        if (!node_is_const_or_address(node->rhs->rhs, &val, &var))
            error_at(node->rhs->rhs->input, "グローバル変数の初期値が定数ではありません");
        if (var) {
            if (val) {
                node->rhs->rhs = new_node('+', var, new_node_num(val, input), var->tp, input);
            } else {
                node->rhs->rhs = var;
            }
        }
    }

    if (node->type==ND_LOCAL_VAR_DEF && type_is_static(node->tp)) {
        char *name = node->name;
        //ローカルstatic変数は初期値を外してreturnする。初期化はvarderのリストから実施される。
        node = new_node(ND_LOCAL_VAR_DEF, NULL, NULL, node->tp, node->input);
        node->name = name;
    }
    return node;
}

//    declarator              = pointer* direct_declarator
//    direct_declarator       = identifier | "(" declarator ")"
//                            = direct_declarator "[" assign? "]"
//declaration_specifiers, pointer, identifierまで先読み済みの可能性あり
static Type *declarator(Type *decl_spec, Type *tp, char **name) {
    if (tp==NULL) {
        tp = pointer(decl_spec);
        *name = NULL;
    }
    tp = direct_declarator(tp, name);

    return tp;
}

static Type *direct_declarator(Type *tp, char **name) {
    if (*name == NULL) {
        if (consume('(')) {
            tp = declarator(tp, NULL, name);
            if (!consume(')')) error_at(input_str(), "閉じかっこがありません");
        } else {
            if (!consume_ident(name)) error_at(input_str(), "型名の後に識別名がありません");
        }
    }
    if (token_is('[')) tp = array_def(tp);
    return tp;
}

//    initializer = assign
//                | "{" init_list "}"
//                | "{" init_list "," "}"
static Node *initializer(void) {
    Node *node;

    if (consume('{')) {
        node = init_list();
        consume(',');
        if (!consume('}')) error_at(input_str(),"初期化式の閉じカッコ } がありません");
    } else {
        node = assign();
    }
    return node;
}

//    init_list   = initializer
//                | init_list "," initializer
static Node *init_list(void) {
    Node *node = new_node_list(NULL, input_str());
    Node *last_node;

    vec_push(node->lst, last_node=initializer());
    while (consume(',') && !token_is('}')) {
        vec_push(node->lst, last_node=initializer());
    }
    node->tp = last_node->tp;

    return node;
}

//    array_def   = "[" assign? "]"
static Type *array_def(Type *tp) {
    Type *ret_tp = tp;
    // int *a[10][2][3]
    if (consume('[')) {
        if (consume(']')) { //char *argv[];
            tp = new_type_array(tp, -1);    //最初だけ省略できる（初期化が必要）
        } else {
            char *input = input_str();
            Node *node = expr();
            long val;
            if (!node_is_const(node, &val)) error_at(input, "配列サイズが定数ではありません");
            if (val==0) error_at(input, "配列のサイズが0です");
            tp = new_type_array(tp, val);
            if (!consume(']')) error_at(input_str(), "配列サイズの閉じかっこ ] がありません"); 
        }
        ret_tp = tp;
        // ret_tp=tp=ARRAY[10] -> PTR -> INT 
    }

    while (consume('[')) {
        char *input = input_str();
        Node *node = expr();
        long val;
        if (!node_is_const(node, &val)) error_at(input, "配列サイズが定数ではありません");
        if (val==0) error_at(input, "配列のサイズが0です");
        tp->ptr_of = new_type_array(tp->ptr_of, val);
        tp = tp->ptr_of;
        if (!consume(']')) error_at(input_str(), "配列サイズの閉じかっこ ] がありません"); 
        // ARRAYのリストの最後に挿入してゆく
        // ret_tp=tp=ARRAY[10]                               -> PTR -> INT 
        // ret_tp=   ARRAY[10] -> tp=ARRAY[2]                -> PTR -> INT 
        // ret_tp=   ARRAY[10] ->    ARRAY[2] -> tp=ARRAY[3] -> PTR -> INT 
    }

    return ret_tp;
}

//    type_ptr    = declaration_specifiers pointer*
static Type *type_ptr(void) {
    Type *tp = declaration_specifiers();
    tp = pointer(tp);

    return tp;
}

static Type *pointer(Type *tp) {
    while (consume('*')) {
        tp = new_type_ptr(tp);
    }

    return tp;
}

static Type *declaration_specifiers(void) {
    Type *tp;

    if (consume(TK_TYPEOF)) {
        if (!consume('(')) error_at(input_str(), "typeofの後に開きカッコがありません");
        char *name;
        if (!consume_ident(&name)) error_at(input_str(), "識別子がありません");
        Node *node = new_node_var(name, NULL);
        tp = node->tp;
        if (!consume(')')) error_at(input_str(), "typeofの後に閉じカッコがありません");
    	return tp;
    }

    char *input;
    Typ type = 0;
    int is_unsigned = 0;

    int char_cnt = 0;
    int short_cnt = 0;
    int int_cnt = 0;
    int long_cnt = 0;
    int signed_cnt = 0;
    int unsigned_cnt = 0;
    int static_cnt = 0;
    int extern_cnt = 0;

    while (1) {
        input = input_str();
        if (consume(TK_CHAR)) {
            if (type) error_at(input, "型指定が不正です\n");
            type = CHAR;
            char_cnt++;
        } else if (consume(TK_SHORT)) {
            if (type && type!=INT) error_at(input, "型指定が不正です\n");
            type = SHORT;
            short_cnt++;
        } else if (consume(TK_INT)) {
            if (type && type!=SHORT && type!=LONG && type!=LONGLONG) error_at(input, "型指定が不正です\n");
            type = INT;
            int_cnt++;
        } else if (consume(TK_LONG)) {
            if (type==LONG) type = LONGLONG;
            else if (type && type!=INT) error_at(input, "型指定が不正です\n");
            else type = LONG;
            long_cnt++;
        } else if (consume(TK_SIGNED)) {
            if (unsigned_cnt) error_at(input, "型指定が不正です\n");
            signed_cnt++;
        } else if (consume(TK_UNSIGNED)) {
            if (signed_cnt) error_at(input, "型指定が不正です\n");
            unsigned_cnt++;
            is_unsigned = 1;
        } else if (consume(TK_STATIC)) {
            if (static_cnt) error_at(input, "型指定が不正です\n");
            static_cnt++;
        } else if (consume(TK_EXTERN)) {
            if (extern_cnt) error_at(input, "型指定が不正です\n");
            extern_cnt++;
        } else {
            if (!type && (signed_cnt || unsigned_cnt)) type = INT;
            if (!type) error_at(input_str(), "型名がありません\n");
            break;
        }
        if (char_cnt>1 || short_cnt>1 || int_cnt>1 || long_cnt>2 ||
            signed_cnt>1 || unsigned_cnt>1 || 
            static_cnt>1 || extern_cnt>1 ||
            (static_cnt && extern_cnt))
            error_at(input, "型指定が重複しています\n");
    }

    tp = new_type(type, is_unsigned);
    if (static_cnt) tp->is_static = 1;
    if (extern_cnt) tp->is_extern = 1;
    return tp;
}

//    func_arg    = type_ptr identifier ( "," func_arg )
static Node *func_arg(void) {
    Node *node = new_node_list(NULL, input_str());
    Type *tp;
    char *name;
    if (!token_is_type()) return node; //空のリスト
    for (;;) {
        char *input = input_str();
        //C言語仕様上型名は省略可能（デフォルトはint）
        tp = type_ptr();
        if (!consume_ident(&name)) error_at(input_str(), "変数名がありません");
        if (token_is('[')) {    //char *argv[]
            tp = array_def(tp);
            if (tp->type==ARRAY && tp->array_size<0) tp->type = PTR;
        }
        vec_push(node->lst, new_node_var_def(name, tp, input));
        if (!consume(',')) break;
    }
    return node;
}

static Node *statement(void) {
    Node *node;
    char *input = input_str();

    if (token_is('{')) {
        return compound_statement();    //{ ブロック }
    } else if (consume(';')) {
        return new_node_empty(input_str());
    } else if (consume(TK_RETURN)) {
        node = expr();
        node = new_node(ND_RETURN, node, NULL, node->tp, input);
    } else if (consume(TK_IF)) {    //if(A)B else C
        Node *node_A, *node_B;
        if (!consume('(')) error_at(input_str(), "ifの後に開きカッコがありません");
        node_A = expr();
        if (!consume(')')) error_at(input_str(), "ifの開きカッコに対応する閉じカッコがありません");
        input = input_str();
        node_B = statement();
        node = new_node(0, node_A, node_B, NULL, input); //lhs
        input = input_str();
        if (consume(TK_ELSE)) {
            node = new_node(ND_IF, node, statement(), NULL, input);
        } else {
            node = new_node(ND_IF, node, NULL, NULL, input);
        }
        return node;
    } else if (consume(TK_WHILE)) {
        if (!consume('(')) error_at(input_str(), "whileの後に開きカッコがありません");
        node = expr();
        if (!consume(')')) error_at(input_str(), "whileの開きカッコに対応する閉じカッコがありません");
        node = new_node(ND_WHILE, node, statement(), NULL, input);
        return node;
    } else if (consume(TK_FOR)) {   //for(A;B;C)D
        Node *node1, *node2;
        if (!consume('(')) error_at(input_str(), "forの後に開きカッコがありません");
        if (consume(';')) {
            node1 = new_node_empty(input_str());
        } else {
            node1 = expr();         //A
            if (!consume(';')) error_at(input_str(), "forの1個目の;がありません");
        }
        if (consume(';')) {
            node2 = new_node_empty(input_str());
        } else {
            node2 = expr();         //B
            if (!consume(';')) error_at(input_str(), "forの2個目の;がありません");
        }
        node = new_node(0, node1, node2, NULL, input);       //A,B
        if (consume(')')) {
            node1 = new_node_empty(input_str());
        } else {
            node1 = expr();         //C
            if (!consume(')')) error_at(input_str(), "forの開きカッコに対応する閉じカッコがありません");
        }
        node2 = new_node(0, node1, statement(), NULL, input);     //C,D
        node = new_node(ND_FOR, node, node2, NULL, input);   //(A,B),(C,D)
        return node;
    } else if (consume(TK_BREAK)) {     //break
        node = new_node(ND_BREAK, NULL, NULL, NULL, input);
    } else if (consume(TK_CONTINUE)) {  //continue
        node = new_node(ND_CONTINUE, NULL, NULL, NULL, input);
    } else {
        node = expr();
    }

    if (!consume(';')) {
        error_at(input_str(), ";でないトークンです");
    }
    return node;
}

static Node *compound_statement(void) {
    Node *node;

    if (!consume('{')) error_at(input_str(), "{がありません");

    node = new_node_block(input_str());
    while (!consume('}')) {
        Node *block;
        if (token_is_type()) {
            block = declaration(NULL, NULL);
        } else {
            block = statement();
        }
        vec_push(node->lst, block);
    }
    return node;
}

//expr：単なる式またはそのコンマリスト（左結合）
//リストであればリスト(ND_LIST)を作成する
//    expr        = assign ( "," assign )* 
static Node *expr(void) {
    char *input = input_str();
    Node *node = assign();
    Node *last_node = node;
    if (consume(',')) {
        node = new_node_list(node, input);
        Vector *lists = node->lst;
        vec_push(lists, last_node=assign());
        while (consume(',')) {
            vec_push(lists, last_node=assign());
        }
    } else {
        return node;
    }
    node->tp = last_node->tp;
    return node;
}

//代入（右結合）
static Node *assign(void) {
    Node *node = tri_cond(), *rhs;
    char *input = input_str();
    if (consume('=')) {
        if (node->tp->type==ARRAY) error_at(node->input, "左辺値ではありません");
        rhs = assign(); 
        if (!(rhs->type==ND_NUM && rhs->val==0) &&  //右辺が0の場合は無条件にOK
            !node_type_eq(node->tp, rhs->tp))
            warning_at(input, "=の左右の型(%s:%s)が異なります", 
                get_type_str(node->tp), get_type_str(rhs->tp));
        node = new_node('=', node, rhs, node->tp, input); //ND_ASIGN
    } else if (consume(TK_PLUS_ASSIGN)) { //+=
        if (node->tp->type==ARRAY) error_at(node->input, "左辺値ではありません");
        rhs = assign(); 
        if (node_is_ptr(node) && node_is_ptr(rhs))
            error_at(node->input, "ポインタ同士の加算です");
        node = new_node(ND_PLUS_ASSIGN, node, rhs, node->tp, input);
    } else if (consume(TK_MINUS_ASSIGN)) { //-=
        if (node->tp->type==ARRAY) error_at(node->input, "左辺値ではありません");
        rhs = assign(); 
        if node_is_ptr(rhs)
            error_at(node->input, "ポインタによる減算です");
        node = new_node(ND_MINUS_ASSIGN, node, rhs, node->tp, input);
    }
    return node;
}

//三項演算子（右結合）
//    tri_cond    = logical_or "?" expr ":" tri_cond
static Node *tri_cond(void) {
    Node *node = logical_or(), *sub_node, *lhs, *rhs;
    char *input = input_str();
    if (consume('?')) {
        lhs = expr();
        if (!consume(':'))
            error_at(node->input, "三項演算に : がありません");
        rhs = tri_cond();
        sub_node = new_node(0, lhs, rhs, lhs->tp, input);
        node = new_node(ND_TRI_COND, node, sub_node, lhs->tp, node->input);
    }
    return node;
}

//論理和（左結合）
//    logical_or  = logical_and ( "||" logical_and )*
static Node *logical_or(void) {
    Node *node = logical_and();
    for (;;) {
        char *input = input_str();
        if (consume(TK_LOR)) {
            node = new_node(ND_LOR, node, logical_and(), new_type(INT, 0), input);
        } else {
            break;
        }
    }
    return node;
}

//論理積（左結合）
//    logical_and = bitwise_or ( "&&" bitwise_or )*
static Node *logical_and(void) {
    Node *node = bitwise_or();
    for (;;) {
        char *input = input_str();
        if (consume(TK_LAND)) {
            node = new_node(ND_LAND, node, bitwise_or(), new_type(INT, 0), input);
        } else {
            break;
        }
    }
    return node;
}

//OR（左結合）
//    bitwise_or = ex_or ( "|" ex_or )*
static Node *bitwise_or(void) {
    Node *node = ex_or();
    for (;;) {
        char *input = input_str();
        if (consume('|')) {
            node = new_node('|', node, ex_or(), new_type(INT, 0), input);
        } else {
            break;
        }
    }
    return node;
}

//EX-OR（左結合）
//    bitwise_or = bitwise_and ( "|" bitwise_and )*
static Node *ex_or(void) {
    Node *node = bitwise_and();
    for (;;) {
        char *input = input_str();
        if (consume('^')) {
            node = new_node('^', node, bitwise_and(), new_type(INT, 0), input);
        } else {
            break;
        }
    }
    return node;
}

//AND（左結合）
//    bitwise_and = equality ( "&" equality )*
static Node *bitwise_and(void) {
    Node *node = equality();
    for (;;) {
        char *input = input_str();
        if (consume('&')) {
            node = new_node('&', node, equality(), new_type(INT, 0), input);
        } else {
            break;
        }
    }
    return node;
}

//等価演算（左結合）
//    equality    = relational ( "==" relational | "!=" relational )*
static Node *equality(void) {
    Node *node = relational();
    for (;;) {
        char *input = input_str();
        if (consume(TK_EQ)) {
            node = new_node(ND_EQ, node, relational(), new_type(INT, 0), input);
        } else if (consume(TK_NE)) {
            node = new_node(ND_NE, node, relational(), new_type(INT, 0), input);
        } else {
            break;
        }
    }
    return node;
}

//関係演算（左結合）
//    relational  = add ( "<" add | "<=" add | ">" add | ">=" add )*
static Node *relational(void) {
    Node *node = add();
    for (;;) {
        char *input = input_str();
        if (consume('<')) {
            node = new_node('<',   node, add(), new_type(INT, 0), input);
        } else if (consume(TK_LE)) {
            node = new_node(ND_LE, node, add(), new_type(INT, 0), input);
        } else if (consume('>')) {
            node = new_node('<',   add(), node, new_type(INT, 0), input);
        } else if (consume(TK_GE)) {
            node = new_node(ND_LE, add(), node, new_type(INT, 0), input);
        } else {
            break;
        }
    }
    return node;
}

//加減算（左結合）
//    add         = mul ( "+" mul | "-" mul )*
static Node *add(void) {
    Node *node = mul(), *rhs;
    for (;;) {
        char *input = input_str();
        if (consume('+')) {
            rhs = mul();
            if (node_is_ptr(node) && node_is_ptr(rhs))
                error_at(node->input, "ポインタ同士の加算です");
            Type *tp = node_is_ptr(node) ? node->tp : rhs->tp;
            node = new_node('+', node, rhs, tp, input);
        } else if (consume('-')) {
            rhs = mul();
            if (node_is_ptr(rhs))
                error_at(input, "ポインタによる減算です");
            node = new_node('-', node, rhs, rhs->tp, input);
        } else {
            break;
        }
    }
    return node;
}

//乗除算、剰余（左結合）
//    mul         = unary ( "*" unary | "/" unary | "%" unary )*
static Node *mul(void) {
    Node *node = unary();
    for (;;) {
        char *input = input_str();
        if (consume('*')) {
            node = new_node('*', node, unary(), node->tp, input);
        } else if (consume('/')) {
            node = new_node('/', node, unary(), node->tp, input);
        } else if (consume('%')) {
            node = new_node('%', node, unary(), node->tp, input);
        } else {
            break;
        }
    }
    return node;
}

//前置単項演算子（右結合）
//    unary       = post_unary
//                | ( "+" | "-" |  "!" | "*" | "&" ) unary
//                | ( "++" | "--" )? unary
//                | "sizeof" unary
//                | "sizeof"   "(" type_ptr array_def? ")"
//                | "_Alignof" "(" type_ptr array_def? ")"
static Node *unary(void) {
    Node *node;
    char *input = input_str();
    if (consume('+')) {
        node = unary();
    } else if (consume('-')) {
        node = unary();
        node = new_node('-', new_node_num(0, input), node, node->tp, input);
    } else if (consume('!')) {
        node = new_node('!', NULL, unary(), new_type(INT, 0), input);
    } else if (consume(TK_INC)) {
        node = unary();
        node = new_node(ND_INC_PRE, NULL, node, node->tp, input);
    } else if (consume(TK_DEC)) {
        node = unary();
        node = new_node(ND_DEC_PRE, NULL, node, node->tp, input);
    } else if (consume('*')) {
        node = unary();
        if (!node_is_ptr(node)) 
            error_at(node->input, "'*'は非ポインタ型(%s)を参照しています", get_type_str(node->tp));
        node = new_node(ND_INDIRECT, NULL, node, node->tp->ptr_of, input);
    } else if (consume('&')) {
        node = unary();
        node = new_node(ND_ADDRESS, NULL, node, new_type_ptr(node->tp), input);
    } else if (consume(TK_SIZEOF)) {
        Type *tp;
        if (token_is('(')) {
            if (next_token_is_type()) {
                consume('(');
                tp = type_ptr();
                if (token_is('[')) tp = array_def(tp);
                if (!consume(')')) error_at(input_str(), "開きカッコに対応する閉じカッコがありません");
            } else {
                node = unary();
                tp = node->tp;
            }
        } else {
            node = unary();
            tp = node->tp;
        }
        node = new_node_num(size_of(tp), input);
    } else if (consume(TK_ALIGNOF)) {
        Type *tp;
        if (!consume('(')) error_at(input_str(), "開きカッコがありません");
        tp = type_ptr();
        if (token_is('[')) tp = array_def(tp);
        if (!consume(')')) error_at(input_str(), "開きカッコに対応する閉じカッコがありません");
        node = new_node_num(align_of(tp), input);
    } else {
        node = post_unary();
    }
    return node;
}

//後置単項演算子（左結合）
//    post_unary  = term ( "++" | "--" | "[" expr "]")?
static Node *post_unary(void) {
    Node *node = term();
    for (;;) {
        char *input = input_str();
        Type *tp = node->tp;
        if (consume(TK_INC)) {
            node = new_node(ND_INC, node, NULL, node->tp, input);
        } else if (consume(TK_DEC)) {
            node = new_node(ND_DEC, node, NULL, node->tp, input);
        } else if (consume('[')) {
            // a[3] => *(a+3)
            // a[3][2] => *(*(a+3)+2)
            input = input_str();
            Node *rhs = expr();
            node = new_node('+', node, rhs, tp ,input);
            tp = node->tp->ptr_of ? node->tp->ptr_of : rhs->tp->ptr_of;
            if (tp==NULL) error_at(input_str(), "ここでは配列を指定できません");
            node = new_node(ND_INDIRECT, NULL, node, tp, input);
            if (!consume(']')) {
                error_at(input_str(), "配列の開きカッコに対応する閉じカッコがありません");
            }
        } else {
            break;
        }
    }
    return node;
}

//終端記号：数値、識別子（変数、関数）、カッコ
static Node *term(void) {
    Node *node;
    char *name;
    char *input = input_str();
    if (consume('(')) {
        node = expr();
        if (!consume(')')) {
            error_at(input_str(), "開きカッコに対応する閉じカッコがありません");
        }
    } else if (consume(TK_NUM)) {
        node = new_node_num(tokens[token_pos-1]->val, input);
    } else if (consume(TK_STRING)) {
        node = new_node_string(tokens[token_pos-1]->str, input);
    } else if (consume_ident(&name)) {
        if (consume('(')) { //関数コール
            node = new_node_func_call(name, input);
            if (consume(')')) return node;
            node->lhs = expr();
            if (node->lhs->type != ND_LIST) {
                node->lhs = new_node_list(node->lhs, input);
            }
            if (!consume(')')) {
                error_at(input_str(), "関数コールの開きカッコに対応する閉じカッコがありません");
            }
        } else {
            node = new_node_var(name, input);
        }
    } else {
        error_at(input, "終端記号でないトークンです");
    }

    return node;
}

int node_is_const(Node *node, long *valp) {
    /*代入は副作用を伴うので定数とはみなさない。
    if (node->type == '=') {
        return node_is_const(node->rhs, val);   //厳密にはlhsへの型変換を考慮すべき？
    }*/

    long val, val1, val2;

    if (node->type==ND_TRI_COND) {
        if (!node_is_const(node->lhs, &val)) return 0;
        if (val) {
            if (!node_is_const(node->rhs->lhs, &val)) return 0;
        } else {
            if (!node_is_const(node->rhs->rhs, &val)) return 0;
        }
        if (valp) *valp = val;
        return 1;
    }

    if (node->lhs && !node_is_const(node->lhs, &val1)) return 0;
    if (node->rhs && !node_is_const(node->rhs, &val2)) return 0;
    switch ((int)node->type) {
    case ND_NUM:  val = node->val;    break;
    case ND_LAND: val = val1 && val2; break;
    case ND_LOR:  val = val1 || val2; break;
    case '&':     val = val1 &  val2; break;
    case '^':     val = val1 ^  val2; break;
    case '|':     val = val1 |  val2; break;
    case ND_EQ:   val = val1 == val2; break;
    case ND_NE:   val = val1 != val2; break;
    case '<':     val = val1 <  val2; break;
    case ND_LE:   val = val1 <= val2; break;
    case '+':     val = val1 +  val2; break;
    case '-':     val = val1 -  val2; break;
    case '*':     val = val1 *  val2; break;
    case '/':     val = val1 /  val2; break;
    case '%':     val = val1 %  val2; break;
    case '!':     val = !val1;        break;
    default:
        return 0;
    }
    if (valp) *valp = val;
    return 1;
}

//nodeがアドレス+定数の形式になっているかどうかを調べる。varpにND_ADDRESS(&var)のノード、valpに定数を返す
int node_is_const_or_address(Node *node, long *valp, Node **varp) {
    long val, val1, val2;
    Node *var1=NULL, *var2=NULL;

    if (node->type==ND_ADDRESS && node->rhs->type==ND_GLOBAL_VAR) {
        if (varp) *varp = node;
        if (valp) *valp = 0;
        return 1;
    }
    if (node->type==ND_GLOBAL_VAR && node->tp->type==ARRAY) {
        if (varp) *varp = new_node(ND_ADDRESS, NULL, node, node->tp->ptr_of, node->input);
        if (valp) *valp = 0;
        return 1;
    }
    if (node->type==ND_LIST) {
        Node **nodes = (Node**)node->lst->data;
        if (node->lst->len==1)
            return node_is_const_or_address(nodes[0], valp, varp);
        return 0;
    }

    if (node->lhs && !node_is_const_or_address(node->lhs, &val1, &var1)) return 0;
    if (node->rhs && !node_is_const_or_address(node->rhs, &val2, &var2)) return 0;
    if (var1 && var2) {
        return 0;
    } else if (var1) {
        if (node->type=='+' || node->type=='-') {
            if (varp) *varp = var1;
        } else {
            return 0;
        }
    } else if (var2) {
        if (node->type=='+') {
            if (varp) *varp = var2;
        } else {
            return 0;
        }
    }

    switch ((int)node->type) {
    case ND_NUM:  val = node->val;    break;
    case ND_LAND: val = val1 && val2; break;
    case ND_LOR:  val = val1 || val2; break;
    case '&':     val = val1 &  val2; break;    //bitwise and
    case '^':     val = val1 ^  val2; break;
    case '|':     val = val1 |  val2; break;
    case ND_EQ:   val = val1 == val2; break;
    case ND_NE:   val = val1 != val2; break;
    case '<':     val = val1 <  val2; break;
    case ND_LE:   val = val1 <= val2; break;
    case '+':     val = val1 +  val2; break;
    case '-':     val = val1 -  val2; break;
    case '*':     val = val1 *  val2; break;
    case '/':     val = val1 /  val2; break;
    case '%':     val = val1 %  val2; break;
    case '!':     val = !val1;        break;
    default:
        return 0;
    }
    if (valp) *valp = val;
    return 1;
}

// static char*p; のような宣言ではPTRにはis_staticが設定されず、
// charだけに設定されているのでcharのところまで見に行く
int type_is_static(Type *tp) {
    if (tp->ptr_of) return type_is_static(tp->ptr_of);
    return tp->is_static;
}

int type_is_extern(Type *tp) {
    if (tp->ptr_of) return type_is_extern(tp->ptr_of);
    return tp->is_extern;
}