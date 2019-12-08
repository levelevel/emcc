#define _PARSE_C_

#include "emcc.h"

//トークンの種類を定義
typedef struct {
    char *name;
    int len;
    TKtype type;
} TokenDef;

#define TK(str) str, sizeof(str)-1
//トークンの終わり判定が不要なもの
TokenDef TokenLst1[] = {
    {TK(">>="), TK_SHIFTR_ASSIGN},
    {TK("<<="), TK_SHIFTL_ASSIGN},
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
    {TK("*="),  TK_MUL_ASSIGN},
    {TK("/="),  TK_DIV_ASSIGN},
    {TK("%="),  TK_MOD_ASSIGN},
    {TK("&="),  TK_AND_ASSIGN},
    {TK("^="),  TK_XOR_ASSIGN},
    {TK("|="),  TK_OR_ASSIGN},
    {TK("->"),  TK_ARROW},
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
    {TK("."),   '.'},
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
    {TK("_Bool"),    TK_BOOL},
    {TK("char"),     TK_CHAR},
    {TK("short"),    TK_SHORT},
    {TK("int"),      TK_INT},
    {TK("long"),     TK_LONG},
    {TK("float"),    TK_FLOAT},
    {TK("double"),   TK_DOUBLE},
    {TK("enum"),     TK_ENUM},
    {TK("struct"),   TK_STRUCT},
    {TK("union"),    TK_UNION},
    {TK("signed"),   TK_SIGNED},
    {TK("unsigned"), TK_UNSIGNED},
    {TK("auto"),     TK_AUTO},
    {TK("register"), TK_REGISTER},
    {TK("static"),   TK_STATIC},
    {TK("extern"),   TK_EXTERN},
    {TK("typedef"),  TK_TYPEDEF},
    {TK("volatile"), TK_VOLATILE},
    {TK("restrict"), TK_RESTRICT},
    {TK("_Atomic"),  TK_ATOMIC},
    {TK("const"),    TK_CONST},
    {TK("inline"),   TK_INLINE},
    {TK("_Noreturn"),TK_NORETURN},
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
    {TK("_Static_assert"), TK_SASSERT},
    {NULL, 0, 0}
};


static Token *new_token(TKtype type, char *input) {
    Token *token = calloc(1, sizeof(Token));
    token->type = type;
    token->info.input = input;
    token->info.line = g_cur_line;
    token->info.file = g_cur_filename;
    vec_push(token_vec, token);
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

// 8進数：最大3桁
static int get_octa(char **pp) {
    char *p = *pp;
    int val = 0;
    for (int i=0; i<3; i++) {
        if (*p>='0' && *p<='7') {
            val = val*8 + *p++ - '0';
        } else {
            break;
        }
    }
    *pp = p;
    return val;
}

// 16進数：桁数制限なし
static int get_hexa(char **pp) {
    char *p = *pp;
    int val = 0;
    for (;;) {
        if (*p>='0' && *p<='9') {
            val = val*16 + *p++ - '0';
        } else if (*p>='a' && *p<='f') {
            val = val*16 + *p++ - 'a' + 10;
        } else if (*p>='A' && *p<='F') {
            val = val*16 + *p++ - 'A' + 10;
        } else {
            break;
        }
    }
    *pp = p;
    return val;
}

//リターン時、ppは次の文字を示す
static int get_escape_char(char **pp) {
    char *p = *pp;
    int c = *p++;
    if (c=='\\') {
        c = *p++;
        switch (c) {
        case 'a':  c = '\a'; break; // 7
        case 'b':  c = '\b'; break; // 8
        case 'f':  c = '\f'; break; //12
        case 'n':  c = '\n'; break; //10
        case 'r':  c = '\r'; break; //13
        case 't':  c = '\t'; break; // 9
        case 'v':  c = '\v'; break; //11
        case '\'':
        case '"':
        case '?':
        case '\\': break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':  --p; c = get_octa(&p); break;
        case 'x':       c = get_hexa(&p); break;
        default:;
            SrcInfo info = {p-1, g_cur_filename, g_cur_line};
            warning_at(&info, "未定義のエスケープシーケンス");
        }
    }

    *pp = p;
    return c;
}

//文字列リテラルの文字列を返す。戻り値の文字列はnul終端されているが、途中にnullを含むことがある。
//stringが指定されていれば、正しい長さを設定する。
static char*get_string(char**pp, String *string) {
    char *p, *ptop;
    p = ptop = *pp;
    int len = 0;
    //まずはエスケープシーケンスもそのまま取得する。
    while (*p && (*p)!='"') {
        p++;
        len++;
        if (*p=='\\' && *(p+1)=='"') {
            p += 2;
            len += 2;
        }
    }
    *pp = *p ? p+1 : p;

    char *buf = malloc(len+1);
    strncpy(buf, ptop, len);
    buf[len] = 0;
    char *q;
    //エスケープシーケンスを処理する
    for (p=q=buf; *p;) {
        if (*p == '\\') {
            *q++ = get_escape_char(&p);
        } else {
            *q++ = *p++;
        }
    }
    *q++ = 0;
    if (string) {
        string->buf = buf;
        string->size = q-buf;
    }
    return buf;
}

//Stringをアセンブラ用にエスケープした文字列を返す
//戻り値は次回コールまで有効
char *escape_string(const String *string) {
    //終端のNULLは出力しない
    String s2;
    s2 = *string;
    if (s2.buf[s2.size-1]=='\0') s2.size--;
    return escape_ascii(&s2);
}
char *escape_ascii(const String *string) {
    //終端のNULLも\000で出力する
    static char *str = NULL;
    static int str_size = 2;
    int new_size = string->size*4;    // 0 -> \000 を考慮して、余裕をもって4倍確保
    if (new_size > str_size) {
        while (new_size > str_size) str_size *= 2;
        str = realloc(str, str_size);
    }
    const char *p = string->buf;
    char *q = str;
    for (int size = string->size; size; p++, size--) {
        switch (*p) {
        case '\b': *q++ = '\\'; *q++ = 'b'; break;
        case '\f': *q++ = '\\'; *q++ = 'f'; break;
        case '\n': *q++ = '\\'; *q++ = 'n'; break;
        case '\r': *q++ = '\\'; *q++ = 'r'; break;
        case '\v': *q++ = '\\'; *q++ = 'v'; break;
        case '"':  *q++ = '\\'; *q++ = '"'; break;
        default: 
            if (*p<' ' || *p>'\x7f') {
                q += sprintf(q, "\\%03o", *p);
            } else {
                *q++ = *p;
            }
        }
    }
    *q = 0;
    return str;
}

//# 1 "test_src/expr.c"
static char *read_directive(char *p) {
    assert(*p=='#');
    p++;
    while (isspace(*p)) p++;
    g_cur_line = strtol(p, &p, 0) - 1;
    while (isspace(*p)) p++;
    assert(*p=='\"');
    p++;
    g_cur_filename = get_string(&p, NULL);
    while (*p && *p != '\n') p++;
    //fprintf(stderr, "%s:%d\n", file, line);
    return p;
}

// pが指している文字列をトークンに分割してtokensに保存する
void tokenize(char *p) {
    Token *token;
    int new_line = 1;
    g_cur_line = 1;
    g_cur_filename = g_filename;
    while (*p) {
        if (isspace(*p)) {
            if (*p == '\n') {
                new_line = 1;
                g_cur_line ++;
            }
            p++;
            continue;
        }

        if (new_line && *p == '#') {
            p = read_directive(p);
            continue;
        }
        new_line = 0;

        //行コメントをスキップ
        if (strncmp(p, "//", 2)==0) {
            p += 2;
            while (*p && *p!='\n') p++;
            continue;
        }

        //ブロックコメントをスキップ
        if (strncmp(p, "/*", 2)==0) {
            char *q = strstr(p + 2, "*/");
            SrcInfo info = {p, g_cur_filename, g_cur_line};
            if (!q) error_at(&info, "コメントが閉じられていません");
            while (p<q) {
                if (*p==('\n')) g_cur_line++;
                p++;
            }
            p = q + 2;
            continue;
        }

        //トークンの終わり判定が不要なもの（記号類）
        for (TokenDef *tk = TokenLst1; tk->name; tk++) {
            if (strncmp(p, tk->name, tk->len)==0) {
                token = new_token(tk->type, p);
                p += tk->len;
                goto NEXT_LOOP;
            }
        }
        //トークンの終わりをis_alnum()で判定するもの（予約語）
        for (TokenDef *tk = TokenLst2; tk->name; tk++) {
            if (strncmp(p, tk->name, tk->len)==0 && !is_alnum(p[tk->len])) {
                token = new_token(tk->type, p);
                p += tk->len;
                goto NEXT_LOOP;
            }
        }

        if (is_alpha(*p)) {         //識別子
            token = new_token(TK_IDENT, p);
            token->ident = get_ident(p);
            p += strlen(token->ident);
        } else if (isdigit(*p)) {   //数値
            token = new_token(TK_NUM, p);
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
            token->is_U = is_U;
            token->is_L = is_L;
        } else if (*p == '"') {     //文字列
            token = new_token(TK_STRING, p++);
            get_string(&p, &token->string);
        } else if (*p == '\'') {    //文字
            token = new_token(TK_NUM, p++);
            token->val = get_escape_char(&p);
            SrcInfo info = {p, g_cur_filename, g_cur_line};
            if (*p++ != '\'') error_at(&info, "トークナイズエラー：'が必要です");
        } else {
            SrcInfo info = {p, g_cur_filename, g_cur_line};
            error_at(&info, "トークナイズエラー");
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
            printf("%d: type='%c', input='%s'\n", i, tp->type, tp->info.input);
        } else {
            printf("%d: type='%d', val=%ld, input='%s'\n", i, tp->type, tp->val, tp->info.input);
        }
    }
}

// ==================================================
// 以下はparse.cローカル
void set_file_line(const Token *token) {
    if (!token) token = tokens[token_pos];
    g_cur_filename = token->info.file;
    g_cur_line     = token->info.line;
}

int token_is_type_spec(void) {
    if (TK_VOID<=token_type() && token_type()<=TK_TYPEDEF) return 1;
    if (token_is(TK_IDENT)) {
        Node *node = search_symbol(token_ident());
        if (node && node->type==ND_TYPEDEF) return 1;
    }
    return 0;
}
int next_token_is_type_spec(void) {
    if (TK_VOID<=next_token_type() && next_token_type()<=TK_TYPEDEF) return 1;
    if (next_token_is(TK_IDENT)) {
        Node *node = search_symbol(next_token_ident());
        if (node && node->type==ND_TYPEDEF) return 1;
    }
    return 0;
}

//次のトークンが期待したものかどうかをチェックし、
//期待したものの場合だけ入力を1トークン読み進めて真を返す
int consume(TKtype type) {
    if (tokens[token_pos]->type != type) return 0;
    set_file_line(tokens[token_pos]);
    token_pos++;
    return 1;
}

//次のトークンが数値(TK_NUM)かどうかをチェックし、
//その場合は数値を取得し、入力を1トークン読み進めて真を返す
int consume_num(long *valp) {
    if (tokens[token_pos]->type != TK_NUM) return 0;
    set_file_line(tokens[token_pos]);
    *valp = tokens[token_pos]->val;
    token_pos++;
    return 1;
}

//次のトークンが識別子(TK_STRING)かどうかをチェックし、
//その場合はstringを取得し、入力を1トークン読み進めて真を返す
int consume_string(String *string) {
    if (tokens[token_pos]->type != TK_STRING) return 0;
    set_file_line(tokens[token_pos]);
    *string = tokens[token_pos]->string;
    token_pos++;
    return 1;
}

//次のトークンが識別子(TK_IDENT)かどうかをチェックし、
//その場合はnameを取得し、入力を1トークン読み進めて真を返す
int consume_ident(char **name) {
    if (tokens[token_pos]->type != TK_IDENT) return 0;
    set_file_line(tokens[token_pos]);
    *name = tokens[token_pos]->ident;
    token_pos++;
    return 1;
}

//次のトークンがtypedef_nsme(TK_IDENT)かどうかをチェックし、
//その場合はそのNode(ND_TYPEDEF)を取得し、入力を1トークン読み進めて真を返す
int consume_typedef(Node **ret_node) {
    if (tokens[token_pos]->type != TK_IDENT) return 0;
    set_file_line(tokens[token_pos]);
    Node *node = search_symbol(token_ident());
    if (node==NULL) return 0;
    if (node->type!=ND_TYPEDEF) return 0;
    token_pos++;
    *ret_node = node;
    return 1;
}

// 次のトークンが期待したものである場合、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(TKtype type) {
    if (tokens[token_pos]->type == type) {
        set_file_line(tokens[token_pos]);
        token_pos++;
        return;
    }
    if (type<128) {
        error_at(&cur_token_info(), "%cが期待されています", type);
    } else {
        error_at(&cur_token_info(), "%sが期待されています", get_NDtype_str(type));
    }
}

//次のトークンが識別子(TK_STRING)である場合stringを取得し、入力を1トークン読み進める。
// それ以外の場合にはエラーを報告する。
void expect_string(String *string) {
    if (tokens[token_pos]->type != TK_STRING) {
        error_at(&cur_token_info(), "文字列リテラルが期待されています");
    }
    *string = tokens[token_pos]->string;
    set_file_line(tokens[token_pos]);
    token_pos++;
}

//次のトークンが識別子(TK_IDENT)であればnameを取得し、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect_ident(char**name, const char*str) {
    if (tokens[token_pos]->type != TK_IDENT) {
        error_at(&cur_token_info(), "識別子（%s）が期待されています", str);
    }
    set_file_line(tokens[token_pos]);
    *name = tokens[token_pos]->ident;
    token_pos++;
}
