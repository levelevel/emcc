#include "9cc.h"

//トークンの種類を定義
typedef struct {
    char *name;
    int len;
    TKtype type;
} TokenDef;

#define TK(str) str, sizeof(str)-1
//トークンの終わり判定が不要なもの
TokenDef TokenLst1[] = {
    {TK("++"),  TK_INC},
    {TK("--"),  TK_DEC},
    {TK("=="),  TK_EQ},
    {TK("!="),  TK_NE},
    {TK(">="),  TK_GE},
    {TK("<="),  TK_LE},
    {TK("&&"),  TK_LAND},
    {TK("||"),  TK_LOR},
    {TK(">>"),  TK_SHIFTR},
    {TK("<<"),  TK_SHIFTL},
    {TK("+="),  TK_PLUS_ASSIGN},
    {TK("-="),  TK_MINUS_ASSIGN},
    {TK("..."), TK_3DOTS},
    {TK("!"),   '!'},
    {TK("%"),   '%'},
    {TK("&"),   '&'},
    {TK("("),   '('},
    {TK(")"),   ')'},
    {TK("*"),   '*'},
    {TK("+"),   '+'},
    {TK(","),   ','},
    {TK("-"),   '-'},
    {TK("/"),   '/'},
    {TK(":"),   ':'},
    {TK(";"),   ';'},
    {TK("<"),   '<'},
    {TK("="),   '='},
    {TK(">"),   '>'},
    {TK("?"),   '?'},
    {TK("["),   '['},
    {TK("]"),   ']'},
    {TK("^"),   '^'},
    {TK("{"),   '{'},
    {TK("|"),   '|'},
    {TK("}"),   '}'},
    {TK("~"),   '~'},
    {NULL, 0, 0}
};

//トークンの終わりをis_alnum()で判定するもの
TokenDef TokenLst2[] = {
    {TK("void"),     TK_VOID},
    {TK("char"),     TK_CHAR},
    {TK("short"),    TK_SHORT},
    {TK("int"),      TK_INT},
    {TK("long"),     TK_LONG},
    {TK("signed"),   TK_SIGNED},
    {TK("unsigned"), TK_UNSIGNED},
    {TK("auto"),     TK_AUTO},
    {TK("register"), TK_REGISTER},
    {TK("static"),   TK_STATIC},
    {TK("extern"),   TK_EXTERN},
//  {TK("volatile"), TK_VOLATILE},
//  {TK("restrict"), TK_RESTRICT},
    {TK("const"),    TK_CONST},
    {TK("goto"),     TK_GOTO},
    {TK("continue"), TK_CONTINUE},
    {TK("break"),    TK_BREAK},
    {TK("return"),   TK_RETURN},
    {TK("if"),       TK_IF},
    {TK("else"),     TK_ELSE},
    {TK("switch"),   TK_SWITCH},
    {TK("case"),     TK_CASE},
    {TK("default"),  TK_DEFAULT},
    {TK("while"),    TK_WHILE},
    {TK("do"),       TK_DO},
    {TK("for"),      TK_FOR},
    {TK("sizeof"),   TK_SIZEOF},
    {TK("typeof"),   TK_TYPEOF},
    {TK("_Alignof"), TK_ALIGNOF},
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
