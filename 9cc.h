//可変長ベクタ ---------------------------------------
typedef struct {
    void **data;
    int capacity;
    int len;
} Vector;

//マップ --------------------------------------------
typedef struct {
    Vector *keys;
    Vector *vals;
} Map;

//トークン ------------------------------------------
typedef enum {
    TK_NUM = 256,   //整数トークン
    TK_INC,         // ++
    TK_DEC,         // --
    TK_EQ,          // ==
    TK_NE,          // !=
    TK_LE,          // <=
    TK_GE,          // >=
    TK_IDENT,       //識別子
    TK_RETURN,      //return
    TK_IF,          //if
    TK_WHILE,       //while
    TK_FOR,         //for
    TK_EOF,         //入力の終わり
} TKtype;

typedef struct {
    TKtype type;    //トークンの型
    int val;        //typeがTK_TOKENの場合、値
    char *name;     //typeがTK_IDENTの場合、その名前
    char *input;    //トークン文字列（エラーメッセージ用）
} Token;

//抽象構文木 ----------------------------------------
enum {
    ND_NUM = 256,   //整数のノードの型
    ND_EQ,          // ==
    ND_NE,          // !=
    ND_LE,          // <=, >=
    ND_IDENT,       //識別子のノードの型
    ND_RETURN,
    ND_IF,
    ND_WHILE,
    ND_FOR,
    ND_BLOCK,
    ND_EMPTY,       //空のノード
};

typedef struct _Node Node;
struct _Node {
    int type;       //演算子、ND_NUM、ND_IDENTのいずれか
    Node *lhs;
    Node *rhs;
    Vector *lst;    //typeがND_BLOCKの場合のstmtのリスト
    int val;        //typeがND_NUMの場合の値
    char *name;     //typeがND_IDENTの場合の変数名
};

//グローバル変数 ----------------------------------------
#ifndef EXTERN
#define EXTERN extern
#endif
// トークナイズした結果のトークン列はこのVectorに保存する
EXTERN Vector *token_vec;
EXTERN Token **tokens;  //token_vec->data;
EXTERN int token_pos;   //tokensの現在位置

//識別子の管理
EXTERN int ident_num;
EXTERN Map *ident_map; //key=name, val=ベースポインタからのoffset

//ステートメントの管理
EXTERN Node *code[100]; //stmt

// parse.c
void tokenize(char *p);
void print_tokens(void);
void program(void);

// codegen.c
void print_prologue(void);
void print_code(void);
void print_epilogue(void);

// util.c
Vector *new_vector(void);
void vec_push(Vector *vec, void *elem);

Map *new_map(void);
void map_put(Map *map, char *key, void *val);
char* map_get(const Map *map, char *key);

void run_test(void);
void error(const char*fmt, ...);
