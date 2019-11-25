#define EXTERN

#include "emcpp.h"

char *cpp(const char *fname) {
    char *p=NULL;
    return p;
}

static void usage(void) {
    fprintf(stderr,"Usage: emcpp [option] file\n");
    fprintf(stderr,"  -dt: dump tokens\n");
    exit(1);
}

static void read_opt(int argc, char*argv[]) {
    if (argc<=1) usage();
    error_ctrl   = ERC_EXIT;
    warning_ctrl = ERC_CONTINUE;
    note_ctrl    = ERC_CONTINUE;
    g_dump_token = 0;
    g_fp = stdout;

    for (; argc>1;  argc--, argv++) {
        if (strcmp(argv[1], "-dt")==0) {
            g_dump_token = 1;
        } else if (argc==2) {
            filename = argv[1];
            user_input = read_file(filename);
            break;
        } else {
            usage();
        }
    }
    //puts(user_input);
}

int main(int argc, char**argv) {
    user_input = "nothing";
    read_opt(argc, argv);

    pptoken_vec = new_vector();
    define_map = new_map();

    cpp_tokenize(user_input);
    pptokens             = (PPToken**)pptoken_vec->data;
    pptoken_pos          = 0;

    preprocessing_file();
}

void test_error(void){}
