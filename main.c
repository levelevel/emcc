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

int main(int argc, char*argv[])
{
    while (argc>1) {
        if (strcmp(argv[1], "-test")==0) {
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
            break;
        } else if (argc==2) {
            filename = argv[1];
            user_input = read_file(filename);
            break;
        } else {
            fprintf(stderr,"Usage: 9cc [-v] {-s 'program' | -test | filename\n");
            return 1;
        }
    }

    error_ctrl   = ERC_EXIT;
    warning_ctrl = ERC_CONTINUE;

    compile();

    return 0;
}

void compile(void) {
    error_cnt          = 0;
    warning_cnt        = 0;
    note_cnt           = 0;
    token_vec          = new_vector();

    // トークナイズ
    tokenize(user_input);

    tokens             = (Token**)token_vec->data;
    token_pos          = 0;
    
    break_label        = NULL;
    continue_label     = NULL;
    
    string_vec         = new_vector();
    static_var_vec     = new_vector();
    global_symbol_map  = new_map();
    global_tagname_map = new_map();
    symbol_stack       = new_stack();
    tagname_stack      = new_stack();
    stack_push(symbol_stack,  global_symbol_map);
    stack_push(tagname_stack, global_tagname_map);
    funcdef_map        = new_map();
    cur_funcdef        = NULL;
    global_index       = 0;
    cur_switch         = NULL;
    // パース
    translation_unit();

    // 抽象構文木を下りながらコード生成
    gen_program();
}
