#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

//トークン ------------------------------------------
enum {
    TK_NUM = 256,   //整数トークン
    TK_EQ,          // ==
    TK_NE,          // !=
    TK_LE,          // <=
    TK_GE,          // >=
    TK_EOF,         //入力の終わり
};

typedef struct {
    int type;       //トークンの型
    int val;        //typeがTK_TOKENの場合の値
    char*input;     //トークン文字列（エラーメッセージ用）
} Token;

// トークナイズした結果のトークン列はこのVectorに保存する
Vector *token_vec = NULL;
Token **tokens;  //token_vec->data;
int token_pos;  //tokensの現在位置

//抽象構文木 ----------------------------------------
enum {
    ND_NUM = 256,   //トークンの型
};

typedef struct _Node Node;
struct _Node {
    int type;       //演算子かND_NUM
    Node *lhs;
    Node *rhs;
    int val;        //typeがND_NUMの場合の値
};

// エラーを報告するための関数
// printfと同じ引数を取る
void error(const char*fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

Token *new_token(void) {
    Token *token = calloc(1, sizeof(Token));
    vec_push(token_vec, token);
    return token;
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
                   *p=='<' || *p=='>') {
            token = new_token();
            token->type = *p;
            token->input = p;
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
}

//次のトークンが期待した型かどうかをチェックし、
//期待した型の場合だけ入力を1トークン読み進めて真を返す
int consume(int type) {
    if (tokens[token_pos]->type != type) return 0;
    token_pos++;
    return 1;
}

//抽象構文木の生成（演算子）
Node *new_node(int type, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->type = type;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

//抽象構文木の生成（数値）
Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->type = ND_NUM;
    node->val = val;
    return node;
}

/*  文法：
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
    term: "(" add ")"
*/
Node *relational(void);
Node *add(void);
Node *mul(void); 
Node *unary(void);
Node *term(void); 

Node *equality(void) {
    Node *node = relational();
    for (;;) {
        if (consume(TK_EQ)) {
            node = new_node(TK_EQ, node, relational());
        } else if (consume(TK_NE)) {
            node = new_node(TK_NE, node, relational());
        } else {
            return node;
        }
    }
}

Node *relational(void) {
    Node *node = add();
    for (;;) {
        if (consume('<')) {
            node = new_node('<', node, add());
        } else if (consume(TK_LE)) {
            node = new_node(TK_LE, node, add());
        } else if (consume('>')) {
            node = new_node('<', add(), node);
        } else if (consume(TK_GE)) {
            node = new_node(TK_LE, add(), node);
        } else {
            return node;
        }
    }
}

Node *add(void) {
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

Node *mul(void) {
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

Node *unary(void) {
    if (consume('+')) {
        return term();
    } else if (consume('-')) {
        return new_node('-', new_node_num(0), term());
    } else {
        return term();
    }
}

Node *term(void) {
    if (consume('(')) {
        Node *node = add();
        if (!consume(')')) {
            error("開きカッコに対応する閉じカッコがありません: %s", tokens[token_pos]->input);
        }
        return node;
    } else if (tokens[token_pos]->type == TK_NUM) {
        return new_node_num(tokens[token_pos++]->val);
    } else {
        error("数値でないトークンです: %s", tokens[token_pos]->input);
        return NULL;
    }
}

//抽象構文木を下りながらコード生成（スタックマシン）
void gen(Node*node) {
    if (node->type == ND_NUM) { //数値
        printf("  push %d\n", node->val);
    } else {                    //演算子
        //lhsとrhsを処理して結果をPUSHする
        gen(node->lhs);
        gen(node->rhs);

        //それらをPOPして、演算する
        printf("  pop rdi\n");  //rhs
        printf("  pop rax\n");  //lhs

        switch(node->type) {
        case TK_EQ: //"=="
            printf("  cmp rax, rdi\n");
            printf("  sete al\n");
            printf("  movzb rax, al\n");
            break;
        case TK_NE: //"!="
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            printf("  movzb rax, al\n");
            break;
        case '<':   //'>'もここで対応（構文木作成時に左右入れ替えてある）
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            printf("  movzb rax, al\n");
            break;
        case TK_LE: //"<="、">="もここで対応（構文木作成時に左右入れ替えてある）
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            printf("  movzb rax, al\n");
            break;
        case '+':
            printf("  add rax, rdi\n");
            break;
        case '-':   //rax(lhs)-rdi(rhs)
            printf("  sub rax, rdi\n");
            break;
        case '*':   //rax*rdi -> rdx:rax
            printf("  mul rdi\n");
            break;
        case '/':   //rdx:rax(lhs) / rdi(rhs) -> rax（商）, rdx（余り）
            printf("  mov rdx, 0\n");
            printf("  div rdi\n");
            break;
        case '%':   //rdx:rax / rdi -> rax（商）, rdx（余り）
            printf("  mov rdx, 0\n");
            printf("  div rdi\n");
            printf("  mov rax, rdx\n");
            break;
        default:
            error("不正なトークンです: '%c'\n", node->type);
        }

        printf("  push rax\n");
    }
}

int main(int argc, char**argv)
{
    if (argc!=2) {
        fprintf(stderr,"引数の個数が正しくありません\n");
        return 1;
    }

    if (strcmp(argv[1], "-test")==0) {
        run_test();
        return 0;
    }

    // トークナイズしてパースする
    token_vec = new_vector();
    tokenize(argv[1]);
    tokens = token_vec->data;
    token_pos = 0;
    Node *node = equality();

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // 抽象構文木を下りながらコード生成
    gen(node);

    // スタックトップに式全体の値が残っているはずなので
    // それをRAXにロードして関数からの返り値とする
    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}
