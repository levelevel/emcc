#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//トークン ------------------------------------------
enum {
    TK_NUM = 256,   //整数トークン
    TK_EOF,         //入力の終わり
};

typedef struct {
    int type;   //トークンの型
    int val;    //typeがTK_TOKENの場合の値
    char*input;    //トークン文字列（エラーメッセージ用）
} Token;

// トークナイズした結果のトークン列はこの配列に保存する
// 100個以上のトークンは来ないものとする
#define MAX_TOKENS  100
Token tokens[MAX_TOKENS];
int token_pos = 0;    //tokensの現在位置

//抽象構文木 ----------------------------------------
enum {
    ND_NUM = 256,   //トークンの型
};

typedef struct _Node {
    int type;   //演算子かND_NUM
    struct _Node *lhs;
    struct _Node *rhs;
    int val;    //typeがND_NUMの場合の値
} Node;

// エラーを報告するための関数
// printfと同じ引数を取る
void error(const char*fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// pが指している文字列をトークンに分割してtokensに保存する
void tokenize(char *p) {
    int i = 0;
    while (*p) {
        if (isspace(*p)) {
            p++;
        } else if (*p=='+' || *p=='-' || *p=='*' || *p=='/' || *p=='(' || *p==')') {
            tokens[i].type = *p;
            tokens[i].input = p;
            i++;
            p++;
        } else if (isdigit(*p)) {
            tokens[i].type = TK_NUM;
            tokens[i].input = p;
            tokens[i].val = strtol(p, &p, 10);
            i++;
        } else {
            error("トークナイズエラー: '%s'\n", p);
            exit(1);
        }
    }
    tokens[i].type = TK_EOF;
    tokens[i].input = p;
}

//次のトークンが期待した型かどうかをチェックし、
//期待した型の場合だけ入力を1トークン読み進めて真を返す
int consume(int type) {
    if (tokens[token_pos].type != type) return 0;
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
    add: mul
    add: add "+" mul
    add: add "-" mul
    mul: mul "*" term
    mul: mul "/" term
    term: num
    term: "(" add ")"
*/
Node *mul(void); 
Node *term(void); 

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
    Node *node = term();
    for (;;) {
        if (consume('*')) {
            node = new_node('*', node, term());
        } else if (consume('/')) {
            node = new_node('/', node, term());
        } else {
            return node;
        }
    }
}

Node *term(void) {
    if (consume('(')) {
        Node *node = add();
        if (!consume(')')) {
            error("開きカッコに対応する閉じカッコがありません: %s", tokens[token_pos].input);
        }
        return node;
    } else if (tokens[token_pos].type == TK_NUM) {
        return new_node_num(tokens[token_pos++].val);
    } else {
        error("数値でないトークンです: %s", tokens[token_pos].input);
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
        printf("  pop rdi\n");
        printf("  pop rax\n");

        switch(node->type) {
        case '+':
            printf("  add rax, rdi\n");
            break;
        case '-':
            printf("  sub rax, rdi\n");
            break;
        case '*':   //rax*rdi -> rdx:rax
            printf("  mul rdi\n");
            break;
        case '/':   //rdx:rax / rdi -> rax
            printf("  mov rdx, 0\n");
            printf("  div rdi\n");
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
    
    // トークナイズしてパースする
    tokenize(argv[1]);
    Node *node = add();

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