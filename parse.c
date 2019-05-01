#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

typedef struct {
    char *name;
    int len;
    TKtype type;
} TokenDef;
TokenDef TokenLst1[] = {
    {"==", 2, TK_EQ},
    {"!=", 2, TK_NE},
    {">=", 2, TK_GE},
    {"<=", 2, TK_LE},
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
    {NULL, 0, 0}
};
TokenDef TokenLst2[] = {
    {"return", 6, TK_RETURN},
    {"if",     2, TK_IF},
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

//抽象構文木の生成（演算子）
static Node *new_node(int type, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->type = type;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

//抽象構文木の生成（数値）
static Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->type = ND_NUM;
    node->val = val;
    return node;
}

//抽象構文木の生成（識別子）
static Node *new_node_ident(char *name) {
    Node *node = calloc(1, sizeof(Node));
    node->type = ND_IDENT;
    node->name = name;

    //未登録の識別子であれば登録する
    if (map_get(ident_map, name)==NULL) {
        map_put(ident_map, name, (void*)(8L*ident_num++));
    }
    return node;
}

//抽象構文木の生成（空文）
static Node *new_node_empty(void) {
    Node *node = calloc(1, sizeof(Node));
    node->type = ND_EMPTY;
    return node;
}

//抽象構文木の生成（ブロック）
static Node *new_node_block(void) {
    Node *node = calloc(1, sizeof(Node));
    node->type = ND_BLOCK;
    node->lst  = new_vector();
    return node;
}

/*  文法：
    program: stmt program
    stmt: "return" assign ";"
    stmt: "if" "(" assign ")" stmt
    stmt: "while" "(" assign ")" stmt
    stmt: "for" "(" assign ";" assign ";" assign ")" stmt
    stmt: "{" block_items "}"
    stmt: assign ";"
    block_items: stmt
    block_items: stmt block_items
    assign: equality
    assign: equality "=" assign
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
    unary: term
    unary: "+" term
    unary: "-" term
    term: num
    term: ident
    term: "(" assign ")"
*/
static Node *stmt(void);
static Node *block_items(void);
static Node *assign(void);
static Node *equality(void);
static Node *relational(void);
static Node *add(void);
static Node *mul(void); 
static Node *unary(void);
static Node *term(void); 

void program(void) {
    int i = 0;
    while (tokens[token_pos]->type != TK_EOF) {
        code[i++] = stmt();
    }
    code[i] = NULL;
}

static Node *stmt(void) {
    Node *node;
    if (consume(';')) {
        return new_node_empty();
    } else if (consume(TK_RETURN)) {
        node = new_node(ND_RETURN, assign(), NULL);
    } else if (consume(TK_IF)) {
        if (!consume('(')) error("ifの後に開きカッコがありません: %s\n", tokens[token_pos]->input);
        node = assign();
        if (!consume(')')) error("ifの開きカッコに対応する閉じカッコがありません: %s\n", tokens[token_pos]->input);
        node = new_node(ND_IF, node, stmt());
        return node;
    } else if (consume(TK_WHILE)) {
        if (!consume('(')) error("whileの後に開きカッコがありません: %s\n", tokens[token_pos]->input);
        node = assign();
        if (!consume(')')) error("whileの開きカッコに対応する閉じカッコがありません: %s\n", tokens[token_pos]->input);
        node = new_node(ND_WHILE, node, stmt());
        return node;
    } else if (consume(TK_FOR)) {   //for(A;B;C)D
        Node *node1, *node2;
        if (!consume('(')) error("forの後に開きカッコがありません: %s\n", tokens[token_pos]->input);
        if (consume(';')) {
            node1 = new_node_empty();   //A
        } else {
            node1 = assign();   //A
            if (!consume(';')) error("forの1個目の;がありません: %s\n", tokens[token_pos]->input);
        }
        if (consume(';')) {
            node2 = new_node_empty();   //B
        } else {
            node2 = assign();   //B
            if (!consume(';')) error("forの2個目の;がありません: %s\n", tokens[token_pos]->input);
        }
        node = new_node(0, node1, node2);       //A,B
        if (consume(')')) {
            node1 = new_node_empty();   //C
        } else {
            node1 = assign();   //C
            if (!consume(')')) error("forの開きカッコに対応する閉じカッコがありません: %s\n", tokens[token_pos]->input);
        }
        node2 = new_node(0, node1, stmt());     //C,D
        node = new_node(ND_FOR, node, node2);   //(A,B),(C,D)
        return node;
    } else if (consume('{')) {      //{ ブロック }
        node = block_items();
        return node;
    } else {
        node = assign();
    }

    if (!consume(';')) {
        error(";'でないトークンです: %s", tokens[token_pos]->input);
    }
    return node;
}

static Node *block_items(void) {
    Node *node = new_node_block();
    Vector *blocks = node->lst;
    while (!consume('}')) {
        vec_push(blocks, stmt());
    }
    return node;
}

static Node *assign(void) {
    Node *node = equality();
    while (consume('=')) {
        node = new_node('=', node, assign());
    }
    return node;
}

static Node *equality(void) {
    Node *node = relational();
    for (;;) {
        if (consume(TK_EQ)) {
            node = new_node(ND_EQ, node, relational());
        } else if (consume(TK_NE)) {
            node = new_node(ND_NE, node, relational());
        } else {
            return node;
        }
    }
}

static Node *relational(void) {
    Node *node = add();
    for (;;) {
        if (consume('<')) {
            node = new_node('<', node, add());
        } else if (consume(TK_LE)) {
            node = new_node(ND_LE, node, add());
        } else if (consume('>')) {
            node = new_node('<', add(), node);
        } else if (consume(TK_GE)) {
            node = new_node(ND_LE, add(), node);
        } else {
            return node;
        }
    }
}

static Node *add(void) {
    Node *node = mul();
    for (;;) {
        if (consume('+')) {
            node = new_node('+', node, mul());
        } else if (consume('-')) {
            node = new_node('-', node, mul());
        } else {
            return node;
        }
    }
}

static Node *mul(void) {
    Node *node = unary();
    for (;;) {
        if (consume('*')) {
            node = new_node('*', node, unary());
        } else if (consume('/')) {
            node = new_node('/', node, unary());
        } else if (consume('%')) {
            node = new_node('%', node, unary());
        } else {
            return node;
        }
    }
}

static Node *unary(void) {
    if (consume('+')) {
        return term();
    } else if (consume('-')) {
        return new_node('-', new_node_num(0), term());
    } else {
        return term();
    }
}

static Node *term(void) {
    if (consume('(')) {
        Node *node = assign();
        if (!consume(')')) {
            error("開きカッコに対応する閉じカッコがありません: %s", tokens[token_pos]->input);
        }
        return node;
    } else if (tokens[token_pos]->type == TK_NUM) {
        return new_node_num(tokens[token_pos++]->val);
    } else if (tokens[token_pos]->type == TK_IDENT) {
        return new_node_ident(tokens[token_pos++]->name);
    } else {
        error("数値でないトークンです: %s", tokens[token_pos]->input);
        return NULL;
    }
}
