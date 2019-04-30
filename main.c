#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXTERN

#include "9cc.h"

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
    tokens = (Token**)token_vec->data;
    token_pos = 0;
    program();

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // プロローグ
    // 変数26個分の領域を確保する
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n"); //8*26=208

    // 抽象構文木を下りながらコード生成
    for (int i=0; code[i]; i++) {
        gen(code[i]);
        // 式の評価結果としてスタックに一つの値が残っている
        // はずなので、スタックが溢れないようにポップしておく
        printf("  pop rax\n");
    }

    // エピローグ
    // 最後の式の結果がRAXに残っているのでそれが返り値になる
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return 0;
}
