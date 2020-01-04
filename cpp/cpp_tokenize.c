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
    ENUM2STR(PPTK_EOF);
    default: return "PPTK_???";
    }
}
static PPToken *new_token(PPTKtype type, char *input) {
    PPToken *token = calloc(1, sizeof(PPToken));
    token->type = type;
    token->info.input = input;
    vec_push(pptoken_vec, token);
    return token;
}

//識別子の文字列を返す。
static char*get_ident(const char*ptop) {
    const char *p = ptop+1;
    int len = 1;
    while (is_alnum(*p)) {
        p++;
        len++;
    }
    char *name = malloc(len+1);
    memcpy(name, ptop, len);
    name[len] = 0;
    return name;
}

void dump_tokens(void) {
    int size = lst_len(pptoken_vec);
    for (int i=0; i<size; i++) {
        PPToken *token = lst_data(pptoken_vec, i);
        printf("[%02d] type=%-13s, len=%2d, ", i, get_PPTKtype_str(token->type), token->len);
        switch (token->type) {
        case PPTK_NEWLINE:
            printf("input=\"\\n\"\n");
            break;
        default:
            if (token->type==PPTK_NUM) printf("val=%ld, ", token->val);
            if (token->ident) printf("name=\"%s\"\n", token->ident);
            else              printf("input=\"%.*s\"\n", token->len, token->info.input);
            break;
        }
    }
}

void cpp_tokenize(char *p) {
    PPToken *token;
    int in_define = 0;
    while (*p) {
        if (*p == ' ' || *p == '\t') {
            token = new_token(PPTK_SPACE, p);
            while (*p==' ' || *p == '\t') p++;
            token->len = p - token->info.input;
            continue;
        } else if (*p == '\n') {
            token = new_token(PPTK_NEWLINE, p);
            p++;
            token->len = 1;
            in_define = 0;
            continue;
        } else if (*p == '\r') {
            if (*(p+1) != '\n') {
                token = new_token(PPTK_NEWLINE, p);
                token->len = 1;
            }
            p++;
            in_define = 0;
            continue;
        } else if (strncmp(p, "//", 2)==0) {
            token = new_token(PPTK_PPTOKEN, p);
            while (*p && *p!='\n') p++;
            token->len = p - token->info.input;
            if (in_define) {
                token->type = PPTK_SPACE;
                token->len = 1;
                token->info.input = " ";
            }
            continue;
        } else if (strncmp(p, "/*", 2)==0) {
            token = new_token(PPTK_PPTOKEN, p);
            char *q = strstr(p + 2, "*/");
            SrcInfo info = {p, g_cur_filename, g_cur_line};
            if (!q) error_at(&info, "コメントが閉じられていません");
            p = q + 2;
            token->len = p - token->info.input;
            if (in_define) {
                token->type = PPTK_SPACE;
                token->len = 1;
                token->info.input = " ";
            }
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
                if (token->type == PPTK_DEFINE) in_define = 1;
                goto NEXT_LOOP;
            }
        }

        if (is_alpha(*p)) {         //識別子
            token = new_token(PPTK_IDENT, p);
            p++;
            while (is_alnum(*p)) p++;
            token->len = p - token->info.input;
            token->ident = get_ident(token->info.input);
        } else if (isdigit(*p)) {   //数値
            token = new_token(PPTK_NUM, p);
            char *p0 = p;
            char *suffix;
            int is_U = 0, is_L = 0;
            if (strncmp(p, "0x", 2)==0 || strncmp(p, "0X", 2)==0) {
                token->val = strtoul(p, &p, 0);    //16進
            } else {
                token->val = strtol(p, &p, 0);     //10進、8進
            }
            suffix = p;
            SrcInfo info0 = {p0, g_cur_filename, g_cur_line};
            SrcInfo info = {suffix, g_cur_filename, g_cur_line};
            for (;;) {
                if (*p=='u' || *p=='U') {
                    if (is_U) error_at(&info, "不正な整数サフィックスです");
                    token->val = strtoul(p0, NULL, 0);  //unsignedで読み直す
                    p++; is_U = 1; continue;
                } else if (strncmp(p, "ll", 2)==0 || strncmp(p, "LL", 2)==0) {     //LLは無視
                    if (is_L) error_at(&info, "不正な整数サフィックスです");
                    p += 2; is_L = 1; continue;
                } else if (*p=='l' || *p=='L') {        //Lは無視
                    if (is_L) error_at(&info, "不正な整数サフィックスです");
                    p++; is_L = 1; continue;
                } else if (is_alnum(*p)) {
                    if (*suffix=='8' || *suffix=='9') error_at(&info0, "不正な8進表記です");
                    error_at(&info, "不正な整数サフィックスです");
                }
                break;
            }
            token->len = p - token->info.input;
        } else if (*p == '"') {     //文字列
            token = new_token(PPTK_STRING, p++);
            while (*p && (*p)!='"') {
                p++;
                if (*p=='\\' && *(p+1)=='"') {
                    p += 2;
                }
            }
            p++;
            token->len = p - token->info.input;
        } else if (*p == '\'') {    //文字
            token = new_token(PPTK_PPTOKEN, p++);
            if (*p=='\\') p++;
            while (*p && (*p)!='\'') p++;
            token->len = p - token->info.input;
        } else {
            token = new_token(PPTK_PPTOKEN, p++);
            token->len = 1;
        }
        NEXT_LOOP:;
    }
    token = new_token(PPTK_EOF, p);
    if (g_dump_token) dump_tokens();
}
