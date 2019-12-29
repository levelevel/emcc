#define EXTERN

#include "emcc.h"

static ErCtrl get_ctrl(const char*opt) {
    switch(opt[2]) {
    case 'c': return ERC_CONTINUE;
    case 'e': return ERC_EXIT;
    case 'a': return ERC_ABORT;
    }
    fprintf(stderr, "Illegal option: %s\n", opt);
    exit(1);
}

static void usage(void) {
    fprintf(stderr,"Usage: emcc [option] {-s 'program' | file}\n");
    fprintf(stderr,"  -[ew][cea]: error/warnin制御。c:continue, e:exit, a:abort\n");
    fprintf(stderr,"  -p: parse only (no code generation)\n");
    fprintf(stderr,"  -d[t]: dump node [and type]\n");
    fprintf(stderr,"  -test: run self test\n");
    exit(1);
}

static void read_opt(int argc, char*argv[]) {
    //コンパイラオプション
    g_g_opt         = 0;    //-g

    //デバッグオプション
    g_dump_node     = 0;
    g_dump_type     = 0;
    g_parse_only    = 0;
    
    if (argc<=1) usage();

    for (; argc>1;  argc--, argv++) {
        if (strcmp(argv[1], "-g")==0) {
            g_g_opt = 1;
        } else if (strncmp(argv[1], "-e", 2)==0) {
            error_ctrl = get_ctrl(argv[1]);
        } else if (strncmp(argv[1], "-w", 2)==0) {
            warning_ctrl = get_ctrl(argv[1]);
        } else if (strcmp(argv[1], "-p")==0) {
            g_parse_only = 1;
        } else if (strcmp(argv[1], "-dt")==0) {
            g_dump_node = 1;
            g_dump_type = 1;
        } else if (strncmp(argv[1], "-d", 2)==0) {
            g_dump_node = 1;
        } else if (strcmp(argv[1], "-test")==0) {
            run_test();
            exit(0);
        } else if (argc==3 && strcmp(argv[1], "-s")==0) {
            g_filename = argv[1];
            g_user_input = argv[2];
            if (strstr(g_user_input, "main")==NULL) {
                char *buf = malloc(strlen(g_user_input) + 50);
                sprintf(buf, "int main(){%s}", g_user_input);
                g_user_input = buf;
            }
            break;
        } else if (argc==2) {
            g_filename = argv[1];
            g_user_input = read_file(g_filename);
            break;
        } else {
            usage();
        }
    }
}

int main(int argc, char*argv[])
{
    error_ctrl   = ERC_EXIT;
    warning_ctrl = ERC_CONTINUE;
    note_ctrl    = ERC_CONTINUE;

    read_opt(argc, argv);

    compile();

    return 0;
}

void compile(void) {
    error_cnt          = 0;
    warning_cnt        = 0;
    note_cnt           = 0;
    token_vec          = new_vector();

    // トークナイズ
    tokenize(g_user_input);

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
    cur_structdef      = NULL;
    cur_switch         = NULL;

    // パース
    translation_unit();
    if (g_parse_only) return;

    // 抽象構文木を下りながらコード生成
    gen_program();
}

void run_test(void) {
    test_error();   //test_src/test_error.c
    printf("run_test: OK\n");
}
