#define EXTERN

#include "9cc.h"

int main(int argc, char**argv)
{
    char *input;

    if (argc!=2) {
        fprintf(stderr,"引数の個数が正しくありません\n");
        return 1;
    }

    if (strcmp(argv[1], "-test")==0) {
        run_test();
        return 0;
    }

    input = argv[1];
    if (strstr(input, "main")==NULL) {
        input = malloc(strlen(input) + 50);
        sprintf(input, "int main(){%s}", argv[1]);
    }

    // トークナイズしてパースする
    token_vec = new_vector();
    tokenize(input);

    tokens = (Token**)token_vec->data;
    token_pos = 0;
    func_map = new_map();
    funcdef_map = new_map();
    program();

    // 抽象構文木を下りながらコード生成
    print_functions();

    return 0;
}
