#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        } else if (*p=='+' || *p=='-') {
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

int main(int argc, char**argv)
{
    if (argc!=2) {
        fprintf(stderr,"引数の個数が正しくありません\n");
        return 1;
    }
    
    tokenize(argv[1]);

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // 式の最初は数でなければならないので、それをチェックして最初のmov命令を出力
    if (tokens[0].type != TK_NUM) error("最初の項が数値ではありません: %s\n", tokens[0].input);
    printf("  mov rax, %d\n", tokens[0].val);

    int i = 1;
    while (tokens[i].type != TK_EOF) {
        if (tokens[i].type == '+') {
            i++;
            if (tokens[i].type != TK_NUM) error("予期しないトークン： %s\n", tokens[i].input);
            printf("  add rax, %d\n", tokens[i].val);
            i++;
        } else if (tokens[i].type == '-') {
            i++;
            if (tokens[i].type != TK_NUM) error("予期しないトークン： %s\n", tokens[i].input);
            printf("  sub rax, %d\n", tokens[i].val);
            i++;
        } else {
            fprintf(stderr, "予期しないトークンです: %s\n", tokens[i].input);
        }
    }

    printf("  ret\n");
    return 0;
}