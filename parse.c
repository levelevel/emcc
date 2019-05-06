#include "9cc.h"

//エラー位置の入力文字列
#define input_str() (tokens[token_pos]->input)

#define token_is(_tk) (tokens[token_pos]->type==(_tk))

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
    {"!",  1, '!'},
    {",",  1, ','},
    {"&",  1, '&'},
    {NULL, 0, 0}
};

//トークンの終わりをis_alnum()で判定するもの
TokenDef TokenLst2[] = {
    {"int",    3, TK_INT},
    {"return", 6, TK_RETURN},
    {"if",     2, TK_IF},
    {"else",   4, TK_ELSE},
    {"while",  5, TK_WHILE},
    {"for",    3, TK_FOR},
    {NULL, 0, 0}
};

static Token *new_token(void) {
    Token *token = calloc(1, sizeof(Token));
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
static char*ident_name(char*ptop) {
    char *p = ptop+1;
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

// pが指している文字列をトークンに分割してtokensに保存する
void tokenize(char *p) {
    Token *token;
    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        for (TokenDef *tk = TokenLst1; tk->name; tk++) {
            if (strncmp(p, tk->name, tk->len)==0) {
                token = new_token();
                token->type = tk->type;
                token->input = p;
                p += tk->len;
                goto NEXT_LOOP;
            }
        }
        for (TokenDef *tk = TokenLst2; tk->name; tk++) {
            if (strncmp(p, tk->name, tk->len)==0 && !is_alnum(p[tk->len])) {
                token = new_token();
                token->type = tk->type;
                token->input = p;
                p += tk->len;
                goto NEXT_LOOP;
            }
        }

        if (is_alpha(*p)) {         //識別子
            token = new_token();
            token->type = TK_IDENT;
            token->name = ident_name(p);
            token->input = p;
            p += strlen(token->name);
        } else if (isdigit(*p)) {   //数値
            token = new_token();
            token->type = TK_NUM;
            token->input = p;
            token->val = strtol(p, &p, 10);
        } else {
            error("トークナイズエラー: '%s'\n", p);
            exit(1);
        }
        NEXT_LOOP:;
    }
    token = new_token();
    token->type = TK_EOF;
    token->input = p;
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
    *name = tokens[token_pos]->name;
    token_pos++;
    return 1;
}

//関数定義のroot生成
static Funcdef *new_funcdef(void) {
    Funcdef * funcdef;
    funcdef = calloc(1, sizeof(Funcdef));
    funcdef->ident_map = new_map();
    return funcdef;
}

//型情報の生成
Type* new_type(Type*ptr) {
    Type *tp = calloc(1, sizeof(Type));
    if (ptr) {
        tp->type = PTR;
        tp->ptr_of = ptr;
    }
    return tp;
}

Type* new_type_int(void) {
    Type *tp = calloc(1, sizeof(Type));
    tp->type = INT;
    return tp;
}

//抽象構文木の生成（演算子）
static Node *new_node(int type, Node *lhs, Node *rhs, char *input) {
    Node *node = calloc(1, sizeof(Node));
    node->type = type;
    node->lhs = lhs;
    node->rhs = rhs;
    node->input = input;
    return node;
}

//抽象構文木の生成（数値）
static Node *new_node_num(int val, char *input) {
    Node *node = new_node(ND_NUM, NULL, NULL, input);
    node->val = val;
    node->tp = new_type_int();
    return node;
}

//抽象構文木の生成（ローカル変数定義）
static Node *new_node_var_def(char *name, Type*tp, char *input) {
    Node *node = new_node(ND_VAR_DEF, NULL, NULL, input);
    node->name = name;
    node->tp = tp;

    //未登録の識別子であれば登録する
    if (map_get(cur_funcdef->ident_map, name, NULL)==0) {
        Vardef *vardef = calloc(1, sizeof(Vardef));
        vardef->name = name;
        vardef->offset = (cur_funcdef->ident_map->keys->len + 1) * 8;
        vardef->tp = tp;
        map_put(cur_funcdef->ident_map, name, vardef);
    } else {
        error("'%s'は変数の重複定義です: '%s'\n", name, tokens[token_pos-1]->input);
    }
    return node;
}

//抽象構文木の生成（識別子：ローカル変数）
static Node *new_node_ident(char *name, char *input) {
    Node *node = new_node(ND_IDENT, NULL, NULL, input);
    node->name = name;

    //定義済みの識別子であるかをチェック
    Vardef *vardef;
    if (map_get(cur_funcdef->ident_map, name, (void**)&vardef)==0) {
        error("'%s'は未定義の変数です: %s\n", name, tokens[token_pos-1]->input);
    }
    node->tp = vardef->tp;

    return node;
}

//抽象構文木の生成（関数コール）
static Node *new_node_func_call(char *name, char *input) {
    Node *node = new_node(ND_FUNC_CALL, NULL, NULL, input);
    node->name = name;
//  node->lhs  = list();    //引数リスト

    //未登録の関数名であれば仮登録する。
    Funcdef *funcdef;
    if (map_get(func_map, name, (void**)&funcdef)==0) {
        funcdef = new_funcdef();
        funcdef->name = name;
        funcdef->node = NULL;
        funcdef->tp = new_type_int();  //暫定値
        map_put(func_map, name, funcdef);
        node->tp = funcdef->tp;
    } else {
    //登録済みであればタイプを取得する
    //もし本登録済みであれば正しい型名が取得できる。。引数はチェックしていない。
        node->tp = funcdef->tp;
    }
    return node;
}

//抽象構文木の生成（関数定義）
static Node *new_node_func_def(char *name, Type *tp, char *input) {
    Node *node = new_node(ND_FUNC_DEF, NULL, NULL, input);
    node->name = name;
    node->tp = tp;
    cur_funcdef->tp = tp;
//  node->lhs  = list();        //引数リスト
//  node->rhs  = block_items(); //ブロック

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
    Node *node = new_node(ND_EMPTY, NULL, NULL, input);
    return node;
}

//抽象構文木の生成（ブロック）
static Node *new_node_block(char *input) {
    Node *node = new_node(ND_BLOCK, NULL, NULL, input);
    node->lst  = new_vector();
    return node;
}

//抽象構文木の生成（コンマリスト）
static Node *new_node_list(Node *item, char *input) {
    Node *node = new_node(ND_LIST, NULL, NULL, input);
    node->lst  = new_vector();
    if (item) vec_push(node->lst, item);
    return node;
}

/*  文法：
    program: function program
    function: type indent "(" func_arg_list ")" "{" block_items "}"
    block_items: stmt
    block_items: stmt block_items
    stmt: var_def ";"
    stmt: "return" list ";"
    stmt: "if" "(" list ")" stmt
    stmt: "if" "(" list ")" stmt "else" stmt
    stmt: "while" "(" list ")" stmt
    stmt: "for" "(" empty_or_list "";" empty_or_list "";" empty_or_list ")" stmt
    stmt: "{" block_items "}"
    stmt: empty_or_list ";"
    var_def: type ident
    type: "int"
    type: type "*"
    func_arg_list: type ident
    func_arg_list: func_arg_list "," type ident
    func_arg_list: <empty>
    empty_or_list: list
    empty_or_list: <empty>
    list: assign
    list: list "," assign
    assign: equality
    assign: equality "=" logical_or
    logical_or: logical_and
    logical_or: logical_or "&&" logical_and
    logical_and: equality
    logical_and: logical_and "&&" equality
    equality: relational
    equality: equality "==" relational
    equality: equality "!=" relational
    relational: add
    relational: relational "<"  add
    relational: relational "<=" add
    relational: relational ">"  add
    relational: relational ">=" add
    add: mul
    add: add "+" mul
    add: add "-" mul
    mul: unary
    mul: mul "*" unary
    mul: mul "/" unary
    mul: mod "%" unary
    unary: post_unary
    unary: "+" unary
    unary: "-" unary
    unary: "!" unary
    unary: "++" post_unary;
    unary: "--" post_unary;
    unary: "*" post_unary
    unary: "&" post_unary
    post_unary: term
    post_unary: term "++"
    post_unary: term "--"
    term: num
    term: ident
    term: ident "(" ")"
    term: "(" assign ")"
*/
static Node *function(void);
static Node *stmt(void);
static Node* var_def(void);
static Type* type(void);
static Node *block_items(void);
static Node *func_arg_list(void);
static Node *empty_or_list(void);
static Node *list(void);
static Node *assign(void);
static Node *equality(void);
static Node *logical_or(void);
static Node *logical_and(void);
static Node *relational(void);
static Node *add(void);
static Node *mul(void); 
static Node *unary(void);
static Node *post_unary(void);
static Node *term(void); 

void program(void) {
    Node *node;
    while (!token_is(TK_EOF)) {
        cur_funcdef = new_funcdef();
        node = function();
        assert(node->type==ND_FUNC_DEF);
        cur_funcdef->node = node;
        cur_funcdef->name = node->name;
        map_put(funcdef_map, node->name, cur_funcdef);
    }
}

//関数の定義: lhs=引数(ND_LIST)、rhs=ブロック(ND_BLOCK)
//    function: type indent "(" func_arg_list ")" "{" block_items "}"
static Node *function(void) {
    Node *node;
    Type *tp;
    char *name;
    char *input = input_str();
    if (token_is(TK_INT)) {
        tp = type();
        if (!consume_ident(&name)) error("型名の後に関数名がありません: %s\n", input_str());
        if (!consume('(')) error("関数定義の開きカッコがありません: %s\n", input_str());
        node = new_node_func_def(name, tp, input);
        node->lhs = func_arg_list();
        if (!consume(')')) error("関数定義の閉じカッコがありません: %s\n", input_str());
        if (!consume('{')) error("関数定義の { がありません: %s\n", input_str());
        node->rhs = block_items();
    } else {
        error("関数定義がありません: %s", input_str());
    }
    return node;
}

static Node *stmt(void) {
    Node *node;
    char *input = input_str();
    if (token_is(TK_INT)) {         //int ident（変数定義）
        node = var_def();
    } else if (consume(TK_RETURN)) {
        node = new_node(ND_RETURN, list(), NULL, input);
    } else if (consume(TK_IF)) {    //if(A)B else C
        Node *node_A, *node_B;
        if (!consume('(')) error("ifの後に開きカッコがありません: %s\n", input_str());
        node_A = list();
        if (!consume(')')) error("ifの開きカッコに対応する閉じカッコがありません: %s\n", input_str());
        input = input_str();
        node_B = stmt();
        node = new_node(0, node_A, node_B, input); //lhs
        input = input_str();
        if (consume(TK_ELSE)) {
            node = new_node(ND_IF, node, stmt(), input);
        } else {
            node = new_node(ND_IF, node, NULL, input);
        }
        return node;
    } else if (consume(TK_WHILE)) {
        if (!consume('(')) error("whileの後に開きカッコがありません: %s\n", input_str());
        node = list();
        if (!consume(')')) error("whileの開きカッコに対応する閉じカッコがありません: %s\n", input_str());
        node = new_node(ND_WHILE, node, stmt(), input);
        return node;
    } else if (consume(TK_FOR)) {   //for(A;B;C)D
        Node *node1, *node2;
        if (!consume('(')) error("forの後に開きカッコがありません: %s\n", input_str());
        node1 = empty_or_list();   //A
        if (!consume(';')) error("forの1個目の;がありません: %s\n", input_str());
        node2 = empty_or_list();   //B
        if (!consume(';')) error("forの2個目の;がありません: %s\n", input_str());
        node = new_node(0, node1, node2, input);       //A,B
        node1 = empty_or_list();   //C
        if (!consume(')')) error("forの開きカッコに対応する閉じカッコがありません: %s\n", input_str());
        node2 = new_node(0, node1, stmt(), input);     //C,D
        node = new_node(ND_FOR, node, node2, input);   //(A,B),(C,D)
        return node;
    } else if (consume('{')) {      //{ ブロック }
        node = block_items();
        return node;
    } else {
        node = empty_or_list();
    }

    if (!consume(';')) {
        error(";'でないトークンです: %s", input_str());
    }
    return node;
}

/* int *ident（変数定義）
    var_def: type ident
    type: "int"
    type: type "*"
*/
static Node* var_def(void) {
    Node *node = NULL;
    Type *tp = type();
    char *name;
    char *input = input_str();
    if (!consume_ident(&name)) error("型名の後に変数名がありません: %s\n", input_str());
    node = new_node_var_def(name, tp, input); //ND_VAR_DEF
    return node;
}

static Type* type(void) {
    Type *tp = new_type(NULL);
    if (consume(TK_INT)) {
        tp->type = INT;
    } else {
        error("型名がありません: %s\n", input_str());
    }
    while (consume('*')) {
        tp = new_type(tp);
    }
    return tp;
}

static Node *block_items(void) {
    Node *node = new_node_block(input_str());
    Vector *blocks = node->lst;
    while (!consume('}')) {
        vec_push(blocks, stmt());
    }
    return node;
}

/* 0個以上のident nodeをnode->lstに設定する。
    func_arg_list: type ident
    func_arg_list: func_arg_list "," type ident
    func_arg_list: 
*/
static Node *func_arg_list(void) {
    Node *node = new_node_list(NULL, input_str());
    Type *tp;
    char *name;
    if (!token_is(TK_INT)) return node; //空のリスト
    for (;;) {
        char *input = input_str();
        //C言語仕様上型名は省略可能（デフォルトはint）
        tp = type();
        if (!consume_ident(&name)) error("変数名がありません: %s", input_str());
        vec_push(node->lst, new_node_var_def(name, tp, input));
        if (!consume(',')) break;
    }
    return node;
}

//コンマリスト（左結合）
// ';'、')'でないトークンまで読んで、空文(ND_EMPTY)またはリスト(ND_LIST)を作成する
static Node *empty_or_list(void) {
    if (token_is(';') || token_is(')')) return new_node_empty(input_str());
    return list();
}

//コンマリスト（左結合）
// ','でないトークンまで読んで、空でないリスト(ND_LIST)を作成する
static Node *list(void) {
    char *input = input_str();
    Node *node = assign();
    if (consume(',')) {
        node = new_node_list(node, input);
        Vector *lists = node->lst;
        vec_push(lists, assign());
        while (consume(',')) {
            vec_push(lists, assign());
        }
    }
    return node;
}

//代入（右結合）
static Node *assign(void) {
    Node *node = logical_or();
    char *input = input_str();
    if (consume('=')) {
        node = new_node('=', node, assign(), input);
        input = input_str();
    }
    return node;
}

//論理和（左結合）
static Node *logical_or(void) {
    Node *node = logical_and();
    for (;;) {
        char *input = input_str();
        if (consume(TK_LOR)) {
            node = new_node(ND_LOR, node, logical_and(), input);
        } else {
            return node;
        }
    }
}

//論理積（左結合）
static Node *logical_and(void) {
    Node *node = equality();
    for (;;) {
        char *input = input_str();
        if (consume(TK_LAND)) {
            node = new_node(ND_LAND, node, equality(), input);
        } else {
            return node;
        }
    }
}

//等価演算（左結合）
static Node *equality(void) {
    Node *node = relational();
    for (;;) {
        char *input = input_str();
        if (consume(TK_EQ)) {
            node = new_node(ND_EQ, node, relational(), input);
        } else if (consume(TK_NE)) {
            node = new_node(ND_NE, node, relational(), input);
        } else {
            return node;
        }
    }
}

//関係演算（左結合）
static Node *relational(void) {
    Node *node = add();
    for (;;) {
        char *input = input_str();
        if (consume('<')) {
            node = new_node('<', node, add(), input);
        } else if (consume(TK_LE)) {
            node = new_node(ND_LE, node, add(), input);
        } else if (consume('>')) {
            node = new_node('<', add(), node, input);
        } else if (consume(TK_GE)) {
            node = new_node(ND_LE, add(), node, input);
        } else {
            return node;
        }
    }
}

//加減算（左結合）
static Node *add(void) {
    Node *node = mul();
    for (;;) {
        char *input = input_str();
        if (consume('+')) {
            node = new_node('+', node, mul(), input);
        } else if (consume('-')) {
            node = new_node('-', node, mul(), input);
        } else {
            return node;
        }
    }
}

//乗除算、剰余（左結合）
static Node *mul(void) {
    Node *node = unary();
    for (;;) {
        char *input = input_str();
        if (consume('*')) {
            node = new_node('*', node, unary(), input);
        } else if (consume('/')) {
            node = new_node('/', node, unary(), input);
        } else if (consume('%')) {
            node = new_node('%', node, unary(), input);
        } else {
            return node;
        }
    }
}

//前置単項演算子（右結合）
static Node *unary(void) {
    char *input = input_str();
    if (consume('+')) {
        return unary();
    } else if (consume('-')) {
        return new_node('-', new_node_num(0, input), unary(), input);
    } else if (consume('!')) {
        return new_node('!', NULL, unary(), input);
    } else if (consume(TK_INC)) {
        return new_node(ND_INC_PRE, NULL, post_unary(), input);
    } else if (consume(TK_DEC)) {
        return new_node(ND_DEC_PRE, NULL, post_unary(), input);
    } else if (consume('*')) {
        return new_node(ND_INDIRECT, NULL, unary(), input);
    } else if (consume('&')) {
        return new_node(ND_ADDRESS, NULL, unary(), input);
    } else {
        return post_unary();
    }
}

//後置単項演算子（左結合）
static Node *post_unary(void) {
    Node *node = term();
    char *input = input_str();
    if (consume(TK_INC)) {
        return new_node(ND_INC, node, NULL, input);
    } else if (consume(TK_DEC)) {
        return new_node(ND_DEC, node, NULL, input);
    } else {
        return node;
    }
}

//終端記号：数値、識別子（変数、関数）、カッコ
static Node *term(void) {
    char *name;
    char *input = input_str();
    if (consume('(')) {
        Node *node = assign();
        if (!consume(')')) {
            error("開きカッコに対応する閉じカッコがありません: %s", input_str());
        }
        return node;
    } else if (consume(TK_NUM)) {
        return new_node_num(tokens[token_pos-1]->val, input);
    } else if (consume_ident(&name)) {
        if (consume('(')) { //関数コール
            Node *node = new_node_func_call(name, input);
            if (consume(')')) return node;
            node->lhs = list();
            if (node->lhs->type != ND_LIST) {
                node->lhs = new_node_list(node->lhs, input);
            }
            if (!consume(')')) {
                error("関数コールの開きカッコに対応する閉じカッコがありません: %s", input_str());
            }
            return node;
        } else {
            return new_node_ident(name, input);
        }
    } else {
        error("終端記号でないトークンです: %s", input);
        return NULL;
    }
}
