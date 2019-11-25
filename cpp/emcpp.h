#include "util.h"

typedef enum {
    PPTK_NUM = 256,
    PPTK_STRING,
    PPTK_IDENT,
    PPTK_SPACE,     //SPACE+TAB
    PPTK_NEWLINE,   //'\n'
    PPTK_IF,        //#if
    PPTK_IFDEF,     //#ifdef
    PPTK_IFNDEF,    //#ifndef
    PPTK_ELIF,      //#elif
    PPTK_ELSE,      //#else
    PPTK_ENDIF,     //#endif
    PPTK_INCLUDE,   //#include
    PPTK_DEFINE,    //#define
    PPTK_UNDEF,     //#undef
    PPTK_LINE,      //#line
    PPTK_ERROR,     //#error
    PPTK_PRAGMA,    //#pragma
    PPTK_PPTOKEN,
    PPTK_DEFARG,    //defineの引数に置換される要素
    PPTK_EOF,       //入力の終わり
} PPTKtype;

typedef struct {
    PPTKtype type;      //トークンの型
    char *ident;        //
    int len;            //トークンの長さ
    long val;           //PPTK_NUMの場合の値、PPTK_DEFARGの場合の引数のindex
    const char *input;  //トークン文字列（エラーメッセージ用）
} PPToken;

void preprocessing_file(void);
void cpp_tokenize(char *p);

EXTERN Vector *pptoken_vec;
EXTERN PPToken **pptokens;  //token_vec->data;
EXTERN int pptoken_pos;     //tokensの現在位置

typedef struct {
    char *name;
    Map *args;      //識別子のマップ。値は無し。
    Vector *body;   //PPTokenのリスト
} Define;
EXTERN Map *define_map;

//デバッグオプション
EXTERN int g_dump_token;
EXTERN FILE *g_fp;

//現在のトークン（エラー箇所）の入力文字列
#define input_str() (pptokens[pptoken_pos]->input)
