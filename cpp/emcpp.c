#define EXTERN

#include "emcpp.h"

char *cpp(const char *fname) {
    char *p=NULL;
    return p;
}

static void usage(void) {
    fprintf(stderr,"Usage: emcpp [option] file\n");
    exit(1);
}

static void read_opt(int argc, char*argv[]) {
    if (argc<=1) usage();

    for (; argc>1;  argc--, argv++) {
        if (argc==2) {
            filename = argv[1];
            user_input = read_file(filename);
            break;
        } else {
            usage();
        }
    }
    puts(user_input);
}

int main(int argc, char**argv) {
    user_input = "nothing";
    read_opt(argc, argv);

    token_vec = new_vector();
    cpp_tokenize(user_input);
    tokens             = (Token**)token_vec->data;
    token_pos          = 0;
}

void test_error(void){}
