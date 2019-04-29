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
