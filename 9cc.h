//可変長ベクタ ---------------------------------------
typedef struct {
    void **data;
    int capacity;
    int len;
} Vector;

//トークン ------------------------------------------
enum {
    TK_NUM = 256,   //整数トークン
    TK_EQ,          // ==
    TK_NE,          // !=
    TK_LE,          // <=
    TK_GE,          // >=
    TK_EOF,         //入力の終わり
};

typedef struct {
    int type;       //トークンの型
    int val;        //typeがTK_TOKENの場合の値
    char*input;     //トークン文字列（エラーメッセージ用）
} Token;

//抽象構文木 ----------------------------------------
enum {
    ND_NUM = 256,   //トークンの型
};

typedef struct _Node Node;
struct _Node {
    int type;       //演算子かND_NUM
    Node *lhs;
    Node *rhs;
    int val;        //typeがND_NUMの場合の値
};

//グローバル変数 ----------------------------------------
#ifndef EXTERN
#define EXTERN extern
#endif
// トークナイズした結果のトークン列はこのVectorに保存する
EXTERN Vector *token_vec;
EXTERN Token **tokens;  //token_vec->data;
EXTERN int token_pos;  //tokensの現在位置

// parse.c
void tokenize(char *p);
Node *equality(void);

// codegen.c
void gen(Node*node);

// util.c
Vector *new_vector(void);
void vec_push(Vector *vec, void *elem);
void run_test(void);
void error(const char*fmt, ...);
