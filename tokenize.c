#include "9cc.h"

//トークンの種類を定義
typedef struct {
    char *name;
    int len;
    TKtype type;
} TokenDef;

//トークンの終わり判定が不要なもの
TokenDef TokenLst1[] = {
    {"++", 2, TK_INC},
    {"--", 2, TK_DEC},
    {"==", 2, TK_EQ},
    {"!=", 2, TK_NE},
    {">=", 2, TK_GE},
    {"<=", 2, TK_LE},
    {"&&", 2, TK_LAND},
    {"||", 2, TK_LOR},
    {"+=", 2, TK_PLUS_ASSIGN},
    {"-=", 2, TK_MINUS_ASSIGN},
    {"...",3, TK_3DOTS},
    {"!",  1, '!'},
    {"%",  1, '%'},
    {"&",  1, '&'},
    {"(",  1, '('},
    {")",  1, ')'},
    {"*",  1, '*'},
    {"+",  1, '+'},
    {",",  1, ','},
    {"-",  1, '-'},
    {"/",  1, '/'},
    {":",  1, ':'},
    {";",  1, ';'},
    {"<",  1, '<'},
    {"=",  1, '='},
    {">",  1, '>'},
    {"?",  1, '?'},
    {"[",  1, '['},
    {"]",  1, ']'},
    {"^",  1, '^'},
    {"{",  1, '{'},
    {"|",  1, '|'},
    {"}",  1, '}'},
    {NULL, 0, 0}
};

//トークンの終わりをis_alnum()で判定するもの
TokenDef TokenLst2[] = {
    {"void",     4, TK_VOID},
    {"char",     4, TK_CHAR},
    {"short",    5, TK_SHORT},
    {"int",      3, TK_INT},
    {"long",     4, TK_LONG},
    {"signed",   6, TK_SIGNED},
    {"unsigned", 8, TK_UNSIGNED},
    {"auto",     4, TK_AUTO},
    {"register", 8, TK_REGISTER},
    {"static",   6, TK_STATIC},
    {"extern",   6, TK_EXTERN},
//  {"volatile", 8, TK_VOLATILE},
//  {"restrict", 8, TK_RESTRICT},
    {"const",    5, TK_CONST},
    {"return",   6, TK_RETURN},
    {"if",       2, TK_IF},
    {"else",     4, TK_ELSE},
    {"while",    5, TK_WHILE},
    {"for",      3, TK_FOR},
    {"break",    5, TK_BREAK},
    {"continue", 8, TK_CONTINUE},
    {"sizeof",   6, TK_SIZEOF},
    {"typeof",   6, TK_TYPEOF},
    {"_Alignof", 8, TK_ALIGNOF},
    {NULL, 0, 0}
};


static Token *new_token(TKtype type, char *input) {
    Token *token = calloc(1, sizeof(Token));
    token->type = type;
    token->input = input;
    vec_push(token_vec, token);
    return token;
}

//識別子に使用できる文字
static int is_alnum(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') || (c == '_');
}
//識別子の先頭に使用できる文字
static int is_alpha(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_');
}

//識別子の文字列を返す。
static char*token_ident(const char*ptop) {
    const char *p = ptop+1;
    int len = 1;
    while (is_alnum(*p)) {
        p++;
        len++;
    }
    char *name = malloc(len+1);
    strncpy(name, ptop, len);
    name[len] = 0;
    return name;
}

//文字列リテラルの文字列を返す。
static char*token_string(const char*ptop) {
    const char *p = ptop+1;
    int len = 1;
    while ((*p)!='"') {
        p++;
        len++;
    }
    char *name = malloc(len+1);
    strncpy(name, ptop, len);
    name[len] = 0;
    return name;
}

// pが指している文字列をトークンに分割してtokensに保存する
void tokenize(char *p) {
    Token *token;
    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        //行コメントをスキップ
        if (strncmp(p, "//", 2)==0) {
            p += 2;
            while (*p && *p!='\n') p++;
            continue;
        }

        //ブロックコメントをスキップ
        if (strncmp(p, "/*", 2)==0) {
            char *q = strstr(p + 2, "*/");
            if (!q) error_at(p, "コメントが閉じられていません");
            p = q + 2;
            continue;
        }

        for (TokenDef *tk = TokenLst1; tk->name; tk++) {
            if (strncmp(p, tk->name, tk->len)==0) {
                token = new_token(tk->type, p);
                p += tk->len;
                goto NEXT_LOOP;
            }
        }
        for (TokenDef *tk = TokenLst2; tk->name; tk++) {
            if (strncmp(p, tk->name, tk->len)==0 && !is_alnum(p[tk->len])) {
                token = new_token(tk->type, p);
                p += tk->len;
                goto NEXT_LOOP;
            }
        }

        if (is_alpha(*p)) {         //識別子
            token = new_token(TK_IDENT, p);
            token->str = token_ident(p);
            p += strlen(token->str);
        } else if (isdigit(*p)) {   //数値
            token = new_token(TK_NUM, p);
            if (strncmp(p, "0x", 2)==0 || strncmp(p, "0X", 2)==0) {
                token->val = strtoul(p, &p, 0); //16進
            //  fprintf(stderr, "strtoul=%lu,%lx\n", token->val, token->val);
            } else {
                token->val = strtol(p, &p, 0);  //10進、8進
            //  fprintf(stderr, "strtol=%ld,%lx\n", token->val, token->val);
            }
        } else if (*p == '"') {     //文字列
            token = new_token(TK_STRING, p);
            token->str = token_string(++p);
            p += strlen(token->str) + 1;
        } else if (*p == '\'') {    //文字
            token = new_token(TK_NUM, p++);
            token->val = *p++;
            if (*p++ != '\'') error_at(p, "トークナイズエラー");
        } else {
            error_at(p, "トークナイズエラー");
            exit(1);
        }
        NEXT_LOOP:;
    }
    token = new_token(TK_EOF, p);
    //dump_tokens();
}

void dump_tokens(void) {
    Token **tk = (Token**)token_vec->data;
    Token *tp;
    for (int i=0; i<token_vec->len; i++) {
        tp = tk[i];
        if (tp->type < TK_NUM) {
            printf("%d: type='%c', input='%s'\n", i, tp->type, tp->input);
        } else {
            printf("%d: type='%d', val=%ld, input='%s'\n", i, tp->type, tp->val, tp->input);
        }
    }
}
