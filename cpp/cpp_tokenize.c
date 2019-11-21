#include "emcpp.h"

//トークンの種類を定義
typedef struct {
    char *name;
    int len;
    PPTKtype type;
} PPTokenDef;

#define TK(str) str, sizeof(str)-1
//トークンの終わり判定が不要なもの
PPTokenDef TokenLst1[] = {
//  {TK(" "),  PPTK_SPACE},
//  {TK("\t"), PPTK_TAB},
//  {TK("\n"), PPTK_NEWLINE},
    {TK("#"), '#'},
    {TK("("), '('},
    {TK(")"), ')'},
    {TK(","), ','},
    {NULL, 0, 0}
};

//トークンの終わりをis_alnum()で判定するもの
PPTokenDef TokenLst2[] = {
    {TK("if"),      PPTK_IF},
    {TK("ifdef"),   PPTK_IFDEF},
    {TK("ifndef"),  PPTK_IFNDEF},
    {TK("elif"),    PPTK_ELIF},
    {TK("else"),    PPTK_ELSE},
    {TK("endif"),   PPTK_ENDIF},
    {TK("include"), PPTK_INCLUDE},
    {TK("define"),  PPTK_DEFINE},
    {TK("undef"),   PPTK_UNDEF},
    {NULL, 0, 0}
};
#define ENUM2STR(val) case val: return #val
const char *get_PPTKtype_str(PPTKtype type) {
    switch ((int)type) {
    ENUM2STR(PPTK_NUM);
    ENUM2STR(PPTK_STRING);
    ENUM2STR(PPTK_IDENT);
    ENUM2STR(PPTK_SPACE);
    ENUM2STR(PPTK_TAB);
    ENUM2STR(PPTK_NEWLINE);
    ENUM2STR(PPTK_IF);
    ENUM2STR(PPTK_IFDEF);
    ENUM2STR(PPTK_ELIF);
    ENUM2STR(PPTK_ELSE);
    ENUM2STR(PPTK_ENDIF);
    ENUM2STR(PPTK_INCLUDE);
    ENUM2STR(PPTK_DEFINE);
    ENUM2STR(PPTK_UNDEF);
    ENUM2STR(PPTK_LINE);
    ENUM2STR(PPTK_ERROR);
    ENUM2STR(PPTK_PRAGMA);
    ENUM2STR(PPTK_PPTOKEN);
    default: return "PPTK_???";
    }
}
static PPToken *new_token(PPTKtype type, char *input) {
    PPToken *token = calloc(1, sizeof(PPToken));
    token->type = type;
    token->input = input;
    vec_push(token_vec, token);
    return token;
}

void dump_tokens() {
    int size = lst_len(token_vec);
    for (int i=0; i<size; i++) {
        PPToken *token = lst_data(token_vec, i);
        printf("[%02d] type=%-13s, len=%2d, ", i, get_PPTKtype_str(token->type), token->len);
        switch (token->type) {
        case PPTK_TAB:
            printf("input=\"\\t\"\n");
            break;
        case PPTK_NEWLINE:
            printf("input=\"\\n\"\n");
            break;
        default:
            printf("input=\"%.*s\"\n", token->len, token->input);
            break;
        }
    }
}
void cpp_tokenize(char *p) {
    PPToken *token;
    while (*p) {
        if (*p == ' ') {
            token = new_token(PPTK_SPACE, p);
            while (*p==' ') p++;
            token->len = p - token->input;
            continue;
        } else if (*p == '\t') {
            token = new_token(PPTK_TAB, p);
            while (*p=='\t') p++;
            token->len = p - token->input;
            continue;
        } else if (*p == '\n') {
            token = new_token(PPTK_NEWLINE, p);
            p++;
            token->len = 1;
            continue;
        } else if (*p == '\r') {
            if (*(p+1) != '\n') {
                token = new_token(PPTK_NEWLINE, p);
                token->len = 1;
            }
            p++;
            continue;
        } else if (strncmp(p, "//", 2)==0) {
            token = new_token(PPTK_PPTOKEN, p);
            while (*p && *p!='\n') p++;
            token->len = p - token->input;
            continue;
        } else if (strncmp(p, "/*", 2)==0) {
            token = new_token(PPTK_PPTOKEN, p);
            char *q = strstr(p + 2, "*/");
            if (!q) error_at(p, "コメントが閉じられていません");
            p = q + 2;
            token->len = p - token->input;
            continue;
        }

        for (PPTokenDef *tk = TokenLst1; tk->name; tk++) {
            if (strncmp(p, tk->name, tk->len)==0) {
                token = new_token(tk->type, p);
                p += tk->len;
                token->len = tk->len;
                goto NEXT_LOOP;
            }
        }
        for (PPTokenDef *tk = TokenLst2; tk->name; tk++) {
            if (strncmp(p, tk->name, tk->len)==0 && !is_alnum(p[tk->len])) {
                token = new_token(tk->type, p);
                p += tk->len;
                token->len = tk->len;
                goto NEXT_LOOP;
            }
        }

        if (is_alpha(*p)) {         //識別子
            token = new_token(PPTK_IDENT, p);
            p++;
            while (is_alnum(*p)) p++;
            token->len = p - token->input;
        } else if (isdigit(*p)) {   //数値
            token = new_token(PPTK_NUM, p);
            if (strncmp(p, "0x", 2)==0 || strncmp(p, "0X", 2)==0) {
                p += 2;
                while (is_hex(*p)) p++;
            } else {
                p++;
                while (isdigit(*p)) p++;
            }
            token->len = p - token->input;
        } else if (*p == '"') {     //文字列
            token = new_token(PPTK_STRING, p++);
            while (*p && (*p)!='"') {
                p++;
                if (*p=='\\' && *(p+1)=='"') {
                    p += 2;
                }
            }
            p++;
            token->len = p - token->input;
        } else if (*p == '\'') {    //文字
            token = new_token(PPTK_PPTOKEN, p++);
            if (*p=='\\') p++;
            while (*p && (*p)!='\'') p++;
            token->len = p - token->input;
        } else {
            token = new_token(PPTK_PPTOKEN, p++);
            token->len = 1;
        }
        NEXT_LOOP:;
    }
    dump_tokens();
}
