#include "9cc.h"

//現在のトークン（エラー箇所）の入力文字列
#define input_str() (tokens[token_pos]->input)
//現在のトークンの型が引数と一致しているか
#define token_is(_tk) (tokens[token_pos]->type==(_tk))
#define token_is_type() (token_is(TK_CHAR)||token_is(TK_INT))
#define next_token_is(_tk) (tokens[token_pos+1]->type==(_tk))
#define next_token_is_type() (next_token_is(TK_CHAR)||next_token_is(TK_INT))

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
    {">",  1, '>'},
    {"<",  1, '<'},
    {"+",  1, '+'},
    {"-",  1, '-'},
    {"*",  1, '*'},
    {"/",  1, '/'},
    {"%",  1, '%'},
    {"(",  1, '('},
    {")",  1, ')'},
    {";",  1, ';'},
    {"=",  1, '='},
    {"{",  1, '{'},
    {"}",  1, '}'},
    {"[",  1, '['},
    {"]",  1, ']'},
    {"!",  1, '!'},
    {",",  1, ','},
    {"&",  1, '&'},
    {"|",  1, '|'},
    {"^",  1, '^'},
    {NULL, 0, 0}
};

//トークンの終わりをis_alnum()で判定するもの
TokenDef TokenLst2[] = {
    {"char",     4, TK_CHAR},
    {"int",      3, TK_INT},
    {"return",   6, TK_RETURN},
    {"if",       2, TK_IF},
    {"else",     4, TK_ELSE},
    {"while",    5, TK_WHILE},
    {"for",      3, TK_FOR},
    {"sizeof",   6, TK_SIZEOF},
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
            token->val = strtol(p, &p, 0);  //10進、16進、8進
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
    //print_tokens();
}

void print_tokens(void) {
    Token **tk = (Token**)token_vec->data;
    Token *tp;
    for (int i=0; i<token_vec->len; i++) {
        tp = tk[i];
        if (tp->type < TK_NUM) {
            printf("%d: type='%c', input='%s'\n", i, tp->type, tp->input);
        } else {
            printf("%d: type='%d', val=%d, input='%s'\n", i, tp->type, tp->val, tp->input);
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
int size_of(const Type *tp) {
    assert(tp);
    switch (tp->type) {
    case CHAR: return sizeof(char);
    case INT:  return sizeof(int);
    case PTR:  return sizeof(void*);
    case ARRAY: return tp->array_size * size_of(tp->ptr_of);
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
        (tp1->type==CHAR && tp2->type==INT) ||
        (tp1->type==INT && tp2->type==CHAR)) {
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

static Type* new_type(int type) {
    Type *tp = calloc(1, sizeof(Type));
    tp->type = type;
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
static Node *new_node_num(int val, char *input) {
    Node *node = new_node(ND_NUM, NULL, NULL, new_type(INT), input);
    node->val = val;
    return node;
}

//未登録の変数であれば登録する
static void regist_var_def(Node *node) {
    char *name = node->name;
    if (cur_funcdef) {  //関数内であればローカル変数
        if (map_get(cur_funcdef->ident_map, name, NULL)==0) {
            Vardef *vardef = calloc(1, sizeof(Vardef));
            vardef->name = name;
            vardef->node = node;
            vardef->offset = get_var_offset(node->tp);
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
    Type *tp = new_type_array(new_type(CHAR), strlen(string)+1);
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
        funcdef->tp = new_type(INT);  //暫定値
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
    program     = top_item*
    top_item    = function | var_def ";"
    function    = type_ptr ident "(" func_arg* ")" "{" stmt* "}"
    stmt        = expr ";"
                | var_def ";"
                | return" expr ";"
                | "if" "(" expr ")" ( "else" expr )?
                | "while" "(" expr ")"
                | "for" "(" expr? ";" expr? ";" expr? ")" stmt
                | "{" expr* "}"
    var_def     = type_spec var_def1 ( "," var_def1 )*
    var_def1    = pointer* ident array_def? ( "=" initializer )?
    initializer = assign
                | "{" init_list "}"
                | "{" init_list "," "}"
    init_list   = initializer
                | init_list "," initializer
    array_def   = "[" assign? "]"
    type_ptr    = type_spec pointer*
    pointer     = ( "*" )*
    type_spec =  "char" | "int"
    func_arg    = type_ptr ident ( "," func_arg )
    expr        = assign ( "," assign )* 
    assign      = logical_or ( "=" assign )*
    logical_or  = logical_and ( "||" logical_and )*
    logical_and = bitwise_or ( "&&" bitwise_or )*
    bitwise_or  = ex_or ( "&" ex_or )*
    ex_or       = bitwise_and ( "^" bitwise_and )*
    bitwise_and = equality ( "&" equality )*
    equality    = relational ( "==" relational | "!=" relational )*
    relational  = add ( "<" add | "<=" add | ">" add | ">=" add )*
    add         = mul ( "+" mul | "-" mul )*
    mul         = unary ( "*" unary | "/" unary | "%" unary )*
    unary       = ( "+" | "-" |  "!" | "*" | "&" ) unary
                | ( "++" | "--" )? post_unary
                | "sizeof" unary
                | "sizeof" "(" type_ptr array_def? ")"
                | "_Alignof" "(" type_ptr array_def? ")"
    post_unary  = term ( "++" | "--" )
    term        = num
                | string
                | ident
                | ident "(" expr? ")"   //関数コール
                |  "(" expr ")"
                | term array_def
*/
static Node *top_item(void);
static Node *function(Type *tp, char *name);
static Node *stmt(void);
static Node *var_def(Type *tp, char *name);
static Node *var_def1(Type *simple_tp, Type *tp, char *name);
static Node *initializer(void);
static Node *init_list(void);
static Type *type_ptr(void);
static Type *pointer(Type *tp);
static Type *type_spec(void);
static Type *array_def(Type *tp);
static Node *func_arg(void);
static Node *empty_or_expr(void);
static Node *expr(void);
static Node *assign(void);
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

void program(void) {
    while (!token_is(TK_EOF)) {
        top_item();
        cur_funcdef = NULL;
    }
}

static Node *top_item(void) {
    Node *node;
    Type *tp;
    char *name;
    if (token_is_type()) {
        tp = type_ptr();
        if (!consume_ident(&name)) error_at(input_str(), "型名の後に識別名がありません");
        if (consume('(')) {
            node = function(tp, name);
        } else {
            node = var_def(tp, name);
            if (!consume(';')) error_at(input_str(), "; がありません");
        }
    } else {
        error_at(input_str(), "関数・変数の定義がありません");
    }
    return node;
}

//関数の定義: lhs=引数(ND_LIST)、rhs=ブロック(ND_BLOCK)
//    function    = type_ptr indent "(" func_arg* ")" "{" stmt* "}"
static Node *function(Type *tp, char *name) {
    Node *node;
    char *input = input_str();
    // typeと関数前と"("まで処理済み
    node = new_node_func_def(name, tp, input);
    node->lhs = func_arg();
    if (!consume(')')) error_at(input_str(), "関数定義の閉じカッコがありません");
    if (!consume('{')) error_at(input_str(), "関数定義の { がありません");

    node->rhs = new_node_block(input_str());
    while (!consume('}')) {
        vec_push(node->rhs->lst, stmt());
    }

    return node;
}

static Node *stmt(void) {
    Node *node;
    char *input = input_str();
    if (token_is_type()) {         //型名 ident（変数定義）
        node = var_def(NULL, NULL);
    } else if (consume(TK_RETURN)) {
        node = expr();
        node = new_node(ND_RETURN, node, NULL, node->tp, input);
    } else if (consume(TK_IF)) {    //if(A)B else C
        Node *node_A, *node_B;
        if (!consume('(')) error_at(input_str(), "ifの後に開きカッコがありません");
        node_A = expr();
        if (!consume(')')) error_at(input_str(), "ifの開きカッコに対応する閉じカッコがありません");
        input = input_str();
        node_B = stmt();
        node = new_node(0, node_A, node_B, NULL, input); //lhs
        input = input_str();
        if (consume(TK_ELSE)) {
            node = new_node(ND_IF, node, stmt(), NULL, input);
        } else {
            node = new_node(ND_IF, node, NULL, NULL, input);
        }
        return node;
    } else if (consume(TK_WHILE)) {
        if (!consume('(')) error_at(input_str(), "whileの後に開きカッコがありません");
        node = expr();
        if (!consume(')')) error_at(input_str(), "whileの開きカッコに対応する閉じカッコがありません");
        node = new_node(ND_WHILE, node, stmt(), NULL, input);
        return node;
    } else if (consume(TK_FOR)) {   //for(A;B;C)D
        Node *node1, *node2;
        if (!consume('(')) error_at(input_str(), "forの後に開きカッコがありません");
        node1 = empty_or_expr();   //A
        if (!consume(';')) error_at(input_str(), "forの1個目の;がありません");
        node2 = empty_or_expr();   //B
        if (!consume(';')) error_at(input_str(), "forの2個目の;がありません");
        node = new_node(0, node1, node2, NULL, input);       //A,B
        node1 = empty_or_expr();   //C
        if (!consume(')')) error_at(input_str(), "forの開きカッコに対応する閉じカッコがありません");
        node2 = new_node(0, node1, stmt(), NULL, input);     //C,D
        node = new_node(ND_FOR, node, node2, NULL, input);   //(A,B),(C,D)
        return node;
    } else if (consume('{')) {      //{ ブロック }
        node = new_node_block(input_str());
        while (!consume('}')) {
            vec_push(node->lst, stmt());
        }
        return node;
    } else {
        node = empty_or_expr();
    }

    if (!consume(';')) {
        error_at(input_str(), ";'でないトークンです");
    }
    return node;
}

//    var_def     = type_spec var_def1 ( "," var_def1 )*
static Node *var_def(Type *tp, char *name) {
    Node *node, *last_node;
    Type *simple_tp;

    if (tp==NULL) {
        //type_ptrと名前を先読みしていなければsimple_typeを読む
        simple_tp = type_spec();
    } else {
        //ポインタを含まないsimple_typeを取り出す
        simple_tp = tp;
        while (simple_tp->ptr_of) simple_tp = simple_tp->ptr_of;
    }

    node = var_def1(simple_tp, tp, name);

    if (consume(',')) {
        node = new_node_list(node, node->input);
        Vector *lists = node->lst;
        vec_push(lists, last_node=var_def1(simple_tp, NULL, NULL));
        while (consume(',')) {
            vec_push(lists, last_node=var_def1(simple_tp, NULL, NULL));
        }
    } else {
        return node;
    }
    node->tp = last_node->tp;
    return node;
}

//    var_def1    = pointer* ident array_def? ( "=" initializer )?
static Node *var_def1(Type *simple_tp, Type *tp, char *name) {
    Node *node, *rhs=NULL;
    char *input = input_str();

    if (tp==NULL) {
        //type_ptrと名前を先読みしていなければポインタ部と名前を読む
        tp = simple_tp;
        while (consume('*')) {
            tp = new_type_ptr(tp);
        }
        if (!consume_ident(&name)) error_at(input_str(), "型名の後に変数名がありません");
    }

    //配列
    if (token_is('[')) tp = array_def(tp);

    //初期値: rhsに初期値を設定する
    if (consume('=')) {
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
                    //fprintf(stderr, "array_size=%ld\n", tp->array_size);
                }
            } else if (rhs->type==ND_LIST) {
                if (tp->array_size<0) {
                    tp->array_size = rhs->lst->len;
                    fprintf(stderr, "array_size=%ld\n", tp->array_size);
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
    if (node->tp->type==ARRAY && node->tp->array_size<0 &&
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

    //fprintf(stderr, "vardef: %s %s\n", get_type_str(node->tp), name);
    return node;
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
    if (consume('[')) {
        if (consume(']')) { //char *argv[];
            tp = new_type_array(tp, -1);
        } else {
            char *input = input_str();
            Node *node = assign();
            long val;
            if (!node_is_const(node, &val)) error_at(input, "配列サイズが定数ではありません");
            if (val==0) error_at(input, "配列のサイズが0です");
            tp = new_type_array(tp, val);
            if (!consume(']')) error_at(input_str(), "配列サイズの閉じかっこ ] がありません"); 
        }
    }
    return tp;
}

//    type_ptr    = type_spec pointer*
static Type *type_ptr(void) {
    Type *tp = type_spec();
    tp = pointer(tp);

    return tp;
}

static Type *pointer(Type *tp) {
    while (consume('*')) {
        tp = new_type_ptr(tp);
    }

    return tp;
}

static Type *type_spec(void) {
    Type *tp;
    if (consume(TK_CHAR)) {
        tp = new_type(CHAR);
    } else if (consume(TK_INT)) {
        tp = new_type(INT);
    } else {
        error_at(input_str(), "型名がありません: %s\n");
    }
    return tp;
}

//    func_arg    = type_ptr ident ( "," func_arg )
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

//コンマリスト（左結合）
// ';'、')'でないトークンまで読んで、空文(ND_EMPTY)または式(expr)を作成する
static Node *empty_or_expr(void) {
    if (token_is(';') || token_is(')')) return new_node_empty(input_str());
    return expr();
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
    Node *node = logical_or(), *rhs;
    char *input = input_str();
    if (consume('=')) {
        if (node->tp->type==ARRAY) error_at(node->input, "左辺値ではありません");
        rhs = assign(); 
        if (!(rhs->type==ND_NUM && rhs->val==0) &&  //右辺が0の場合は無条件にOK
            !node_type_eq(node->tp, rhs->tp))
            warning_at(input, "=の左右の型(%s:%s)が異なります", 
                get_type_str(node->tp), get_type_str(rhs->tp));
        node = new_node('=', node, rhs, node->tp, input);
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
            node = new_node(ND_LOR, node, logical_and(), new_type(INT), input);
        } else {
            return node;
        }
    }
}

//論理積（左結合）
//    logical_and = bitwise_or ( "&&" bitwise_or )*
static Node *logical_and(void) {
    Node *node = bitwise_or();
    for (;;) {
        char *input = input_str();
        if (consume(TK_LAND)) {
            node = new_node(ND_LAND, node, bitwise_or(), new_type(INT), input);
        } else {
            return node;
        }
    }
}

//OR（左結合）
//    bitwise_or = ex_or ( "|" ex_or )*
static Node *bitwise_or(void) {
    Node *node = ex_or();
    for (;;) {
        char *input = input_str();
        if (consume('|')) {
            node = new_node('|', node, ex_or(), new_type(INT), input);
        } else {
            return node;
        }
    }
}

//EX-OR（左結合）
//    bitwise_or = bitwise_and ( "|" bitwise_and )*
static Node *ex_or(void) {
    Node *node = bitwise_and();
    for (;;) {
        char *input = input_str();
        if (consume('^')) {
            node = new_node('^', node, bitwise_and(), new_type(INT), input);
        } else {
            return node;
        }
    }
}

//AND（左結合）
//    bitwise_and = equality ( "&" equality )*
static Node *bitwise_and(void) {
    Node *node = equality();
    for (;;) {
        char *input = input_str();
        if (consume('&')) {
            node = new_node('&', node, equality(), new_type(INT), input);
        } else {
            return node;
        }
    }
}

//等価演算（左結合）
//    equality    = relational ( "==" relational | "!=" relational )*
static Node *equality(void) {
    Node *node = relational();
    for (;;) {
        char *input = input_str();
        if (consume(TK_EQ)) {
            node = new_node(ND_EQ, node, relational(), new_type(INT), input);
        } else if (consume(TK_NE)) {
            node = new_node(ND_NE, node, relational(), new_type(INT), input);
        } else {
            return node;
        }
    }
}

//関係演算（左結合）
//    relational  = add ( "<" add | "<=" add | ">" add | ">=" add )*
static Node *relational(void) {
    Node *node = add();
    for (;;) {
        char *input = input_str();
        if (consume('<')) {
            node = new_node('<',   node, add(), new_type(INT), input);
        } else if (consume(TK_LE)) {
            node = new_node(ND_LE, node, add(), new_type(INT), input);
        } else if (consume('>')) {
            node = new_node('<',   add(), node, new_type(INT), input);
        } else if (consume(TK_GE)) {
            node = new_node(ND_LE, add(), node, new_type(INT), input);
        } else {
            return node;
        }
    }
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
            return node;
        }
    }
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
            return node;
        }
    }
}

//前置単項演算子（右結合）
//    unary       = ( "+" | "-" |  "!" ) unary
//                | ( "++" | "--" | "*" | "&" )? post_unary
static Node *unary(void) {
    Node *node;
    char *input = input_str();
    if (consume('+')) {
        return unary();
    } else if (consume('-')) {
        node = unary();
        return new_node('-', new_node_num(0, input), node, node->tp, input);
    } else if (consume('!')) {
        return new_node('!', NULL, unary(), new_type(INT), input);
    } else if (consume(TK_INC)) {
        node = post_unary();
        return new_node(ND_INC_PRE, NULL, node, node->tp, input);
    } else if (consume(TK_DEC)) {
        node = post_unary();
        return new_node(ND_DEC_PRE, NULL, node, node->tp, input);
    } else if (consume('*')) {
        node = unary();
        assert(node->tp != NULL);
        if (!node_is_ptr(node)) 
            error_at(node->input, "'*'は非ポインタ型(%s)を参照しています", 
                get_type_str(node->tp));
        return new_node(ND_INDIRECT, NULL, node, node->tp->ptr_of, input);
    } else if (consume('&')) {
        node = unary();
        return new_node(ND_ADDRESS, NULL, node, new_type_ptr(node->tp), input);
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
        return new_node_num(size_of(tp), input);
    } else if (consume(TK_ALIGNOF)) {
        Type *tp;
        if (!consume('(')) error_at(input_str(), "開きカッコがありません");
        tp = type_ptr();
        if (token_is('[')) tp = array_def(tp);
        if (!consume(')')) error_at(input_str(), "開きカッコに対応する閉じカッコがありません");
        return new_node_num(align_of(tp), input);
    } else {
        return post_unary();
    }
}

//後置単項演算子（左結合）
//    post_unary  = term ( "++" | "--" )
static Node *post_unary(void) {
    Node *node = term();
    char *input = input_str();
    if (consume(TK_INC)) {
        return new_node(ND_INC, node, NULL, node->tp, input);
    } else if (consume(TK_DEC)) {
        return new_node(ND_DEC, node, NULL, node->tp, input);
    } else {
        return node;
    }
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
        return NULL;
    }

    if (consume('[')) {
        // a[3] => *(a+3)
        Node *rhs = assign();
        node = new_node('+', node, rhs, node->tp ,input);
        Type *tp = node->tp->ptr_of ? node->tp->ptr_of : rhs->tp->ptr_of;
        assert(tp!=NULL);
        node = new_node(ND_INDIRECT, NULL, node, tp, input);
        if (!consume(']')) {
            error_at(input_str(), "配列の開きカッコに対応する閉じカッコがありません");
        }
    }

    return node;
}

int node_is_const(Node *node, long *valp) {
    /*代入は副作用を伴うので定数とはみなさない。
    if (node->type == '=') {
        return node_is_const(node->rhs, val);   //厳密にはlhsへの型変換を考慮すべき？
    }*/

    long val, val1, val2;
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
