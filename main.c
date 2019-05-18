#define EXTERN

#include "9cc.h"

static char *read_file(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) error("cannot open %s: %s", path, strerror(errno));

    if (fseek(fp, 0, SEEK_END) == -1) error("%s: fseek: %s", path, strerror(errno));
    size_t size = ftell(fp);
    if (fseek(fp, 0, SEEK_SET) == -1) error("%s: fseek: %s", path, strerror(errno));

    char *buf = malloc(size+2);
    fread(buf, size, 1, fp);
    fclose(fp);

    if (buf[size-1] != '\n') buf[size++] = '\n';
    buf[size] = 0;
    return buf;
}

int main(int argc, char**argv)
{
    if (argc==2 && strcmp(argv[1], "-test")==0) {
        run_test();
        return 0;
    } else if (argc==3 && strcmp(argv[1], "-s")==0) {
        filename = argv[1];
        user_input = argv[2];
        if (strstr(user_input, "main")==NULL) {
            char *buf = malloc(strlen(user_input) + 50);
            sprintf(buf, "int main(){%s}", user_input);
            user_input = buf;
        }
    } else if (argc==2) {
        filename = argv[1];
        user_input = read_file(filename);
    } else {
        fprintf(stderr,"Usage: 9cc {-s 'program' | -test | filename\n");
        return 1;
    }

    // トークナイズしてパースする
    token_vec = new_vector();
    tokenize(user_input);

    tokens = (Token**)token_vec->data;
    token_pos = 0;
    string_vec = new_vector();
    func_map = new_map();
    funcdef_map = new_map();
    global_vardef_map = new_map();
    program();

    // 抽象構文木を下りながらコード生成
    print_functions();

    return 0;
}
