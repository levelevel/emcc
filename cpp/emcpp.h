#include "emcc.h"

typedef enum {
    PPTK_NUM = 256,
    PPTK_STRING,
    PPTK_IDENT,
    PPTK_SPACE,
    PPTK_TAB,
    PPTK_NEWLINE,
    PPTK_IF,
    PPTK_IFDEF,
    PPTK_IFNDEF,
    PPTK_ELIF,
    PPTK_ELSE,
    PPTK_ENDIF,
    PPTK_INCLUDE,
    PPTK_DEFINE,
    PPTK_UNDEF,
    PPTK_LINE,
    PPTK_ERROR,
    PPTK_PRAGMA,
    PPTK_PPTOKEN,
} PPTKtype;

typedef struct {
    PPTKtype type;  //トークンの型
    int len;        //トークンの長さ
    const char *input;    //トークン文字列（エラーメッセージ用）
} PPToken;

void preprocessing_file(void);
void cpp_tokenize(char *p);
int cpp_consume(PPTKtype type);

