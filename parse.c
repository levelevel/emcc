#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

static Token *new_token(void) {
    Token *token = calloc(1, sizeof(Token));
    vec_push(token_vec, token);
    return token;
}

static int is_alnum(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') || (c == '_');
}

// pが指している文字列をトークンに分割してtokensに保存する
void tokenize(char *p) {
    Token *token;
    while (*p) {
        if (isspace(*p)) {
            p++;
        } else if (strncmp(p, "==", 2)==0) {
            token = new_token();
            token->type = TK_EQ;
            token->input = p;
            p += 2;
        } else if (strncmp(p, "!=", 2)==0) {
            token = new_token();
            token->type = TK_NE;
            token->input = p;
            p += 2;
        } else if (strncmp(p, ">=", 2)==0) {
            token = new_token();
            token->type = TK_GE;
            token->input = p;
            p += 2;
        } else if (strncmp(p, "<=", 2)==0) {
            token = new_token();
            token->type = TK_LE;
            token->input = p;
            p += 2;
        } else if (*p=='+' || *p=='-' || *p=='*' || *p=='/' || *p=='%' || *p=='(' || *p==')' ||
                   *p=='<' || *p=='>' || *p==';' || *p=='=') {
            token = new_token();
            token->type = *p;
            token->input = p;
            p++;
        } else if (strncmp(p, "return", 6)==0 && !is_alnum(p[6])) {
            token = new_token();
            token->type = TK_RETURN;
            token->input = p;
            p += 6;
        } else if (*p>='a' && *p<='z') {
            token = new_token();
            token->type = TK_IDENT;
            token->input = p;   //変数名は*pで取得できる
            p++;
        } else if (isdigit(*p)) {
            token = new_token();
            token->type = TK_NUM;
            token->input = p;
            token->val = strtol(p, &p, 10);
        } else {
            error("トークナイズエラー: '%s'\n", p);
            exit(1);
        }
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
static int consume(int type) {
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

//抽象構文木の生成（変数）
static Node *new_node_ident(int name) {
    Node *node = calloc(1, sizeof(Node));
    node->type = ND_IDENT;
    node->name = name;
    return node;
}

/*  文法：
    program: stmt program
    stmt: "return" assign ";"
    stmt: assign ";"
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
    if (consume(TK_RETURN)) {
        node = new_node(ND_RETURN, assign(), NULL);
    } else {
        node = assign();
    }

    if (!consume(';')) {
        error(";'でないトークンです: %s", tokens[token_pos]->input);
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
        return new_node_ident(*tokens[token_pos++]->input);
    } else {
        error("数値でないトークンです: %s", tokens[token_pos]->input);
        return NULL;
    }
}
