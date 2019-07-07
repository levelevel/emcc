#define _PARSE_C_

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
    {TK("enum"),     TK_ENUM},
    {TK("signed"),   TK_SIGNED},
    {TK("unsigned"), TK_UNSIGNED},
    {TK("auto"),     TK_AUTO},
    {TK("register"), TK_REGISTER},
    {TK("static"),   TK_STATIC},
    {TK("extern"),   TK_EXTERN},
    {TK("typedef"),  TK_TYPEDEF},
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

// ==================================================
// 以下はparse.cローカル

int token_is_type_spec(void) {
    if (TK_VOID<=token_type() && token_type()<=TK_TYPEDEF) return 1;
    if (token_is(TK_IDENT)) {
        Node *node = search_symbol(token_str());
        if (node && node->type==ND_TYPEDEF) return 1;
    }
    return 0;
}
int next_token_is_type_spec(void) {
    if (TK_VOID<=next_token_type() && next_token_type()<=TK_TYPEDEF) return 1;
    if (next_token_is(TK_IDENT)) {
        Node *node = search_symbol(next_token_str());
        if (node && node->type==ND_TYPEDEF) return 1;
    }
    return 0;
}

//次のトークンが期待したものかどうかをチェックし、
//期待したものの場合だけ入力を1トークン読み進めて真を返す
int consume(TKtype type) {
    if (tokens[token_pos]->type != type) return 0;
    token_pos++;
    return 1;
}

//次のトークンが数値(TK_NUM)かどうかをチェックし、
//その場合は数値を取得し、入力を1トークン読み進めて真を返す
int consume_num(long *valp) {
    if (tokens[token_pos]->type != TK_NUM) return 0;
    *valp = tokens[token_pos]->val;
    token_pos++;
    return 1;
}

//次のトークンが識別子(TK_STRING)かどうかをチェックし、
//その場合はstrを取得し、入力を1トークン読み進めて真を返す
int consume_string(char **str) {
    if (tokens[token_pos]->type != TK_STRING) return 0;
    *str = tokens[token_pos]->str;
    token_pos++;
    return 1;
}

//次のトークンが識別子(TK_IDENT)かどうかをチェックし、
//その場合はnameを取得し、入力を1トークン読み進めて真を返す
int consume_ident(char **name) {
    if (tokens[token_pos]->type != TK_IDENT) return 0;
    *name = tokens[token_pos]->str;
    token_pos++;
    return 1;
}

//次のトークンがtypedef_nsme(TK_IDENT)かどうかをチェックし、
//その場合はそのNode(ND_TYPEDEF)を取得し、入力を1トークン読み進めて真を返す
int consume_typedef(Node **ret_node) {
    if (tokens[token_pos]->type != TK_IDENT) return 0;
    Node *node = search_symbol(token_str());
    if (node==NULL) return 0;
    if (node->type!=ND_TYPEDEF) return 0;
    token_pos++;
    *ret_node = node;
    return 1;
}

// 次のトークンが期待したものである場合、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(TKtype type) {
    if (tokens[token_pos++]->type == type) return;
    if (type<128) {
        error_at(input_str(), "%cが期待されています", type);
    } else {
        error_at(input_str(), "%sが期待されています", get_NDtype_str(type));
    }
}

//次のトークンが識別子(TK_IDENT)であればnameを取得し、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect_ident(char**name, const char*str) {
    if (tokens[token_pos]->type != TK_IDENT) {
        error_at(input_str(), "識別子（%s）が期待されています", str);
    }
    *name = tokens[token_pos]->str;
    token_pos++;
}
