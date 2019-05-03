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
    func_map = new_map();
    funcdef_map = new_map();
    program();

    // 抽象構文木を下りながらコード生成
    print_functions();

    return 0;
}
