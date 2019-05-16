#include <stdarg.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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
    TK_STRING,      //文字列
    TK_IDENT,       //識別子
    TK_CHAR,        //char
    TK_INT,         //int
    TK_INC,         // ++
    TK_DEC,         // --
    TK_EQ,          // ==
    TK_NE,          // !=
    TK_LE,          // <=
    TK_GE,          // >=
    TK_LAND,        // &&
    TK_LOR,         // ||
    TK_RETURN,      //return
    TK_IF,          //if
    TK_ELSE,        //else
    TK_WHILE,       //while
    TK_FOR,         //for
    TK_SIZEOF,      //sizeof
    TK_EOF,         //入力の終わり
} TKtype;

typedef struct {
    TKtype type;    //トークンの型
    int val;        //typeがTK_TOKENの場合、値
    char *str;      //typeがTK_STRING/TK_IDENTの場合、その文字列
    char *input;    //トークン文字列（エラーメッセージ用）
} Token;

//抽象構文木 ----------------------------------------
typedef enum {
    ND_NUM = 256,   //整数のノードの型
    ND_STRING,
    ND_LOCAL_VAR,   //ローカル変数の参照
    ND_GLOBAL_VAR,  //グローバル変数の参照
    ND_INC,
    ND_DEC,
    ND_INC_PRE,
    ND_DEC_PRE,
    ND_INDIRECT,    // *（間接参照）
    ND_ADDRESS,     // &（アドレス演算子）
    ND_EQ,          // ==
    ND_NE,          // !=
    ND_LE,          // <=, >=
    ND_LAND,        // &&
    ND_LOR,         // ||
    ND_VAR_DEF,     //変数の定義
    ND_RETURN,
    ND_IF,
    ND_WHILE,
    ND_FOR,
    ND_BLOCK,       //{...}
    ND_LIST,        //コンマリスト
    ND_FUNC_CALL,   //関数コール
    ND_FUNC_DEF,    //関数定義
    ND_EMPTY,       //空のノード
} NDtype;

typedef struct _Type Type;
struct _Type {
    enum {CHAR, INT, PTR, ARRAY} type;
    Type *ptr_of;
    long array_size;  //typeがARRAYの場合の配列サイズ
};

typedef struct _Node Node;
struct _Node {
    int type;       //nodeの型：演算子、ND_INDENTなど
    Node *lhs;
    Node *rhs;
    Vector *lst;    //typeがND_BLOCKの場合のstmtのリスト
                    //typeがND_LISTの場合のasignのリスト
    int val;        //typeがND_NUMの場合の値
                    //typeがND_STRINGの場合のインデックス
    char *name;     //typeがND_IDENTの場合の変数名
    Type *tp;       //型情報：typeがND_NUM、ND_IDENT、ND_FUNC_DEFの場合の場合は
                    //トークナイズ時に設定。それ以外は評価時に設定。
    char *input;    //トークン文字列（エラーメッセージ用）。Token.inputと同じ。
};

typedef struct {
    char *name;     //変数名
    Node *node;     //ノード（型情報、初期値、ローカル・グローバルなどもここから取得する）
    int offset;     //ベースポインタからのoffset
} Vardef;

typedef struct {
    char *name;     //関数名
    Node *node;     //ND_FUNC_DEFのnode
    Type *tp;       //関数の型情報
    Map *ident_map; //ローカル変数：key=name, val=Vardef
} Funcdef;

//ノードがポインタ（PTR||ARRAY）であるか
#define node_is_ptr(_node) ((_node)->tp->type==PTR || (_node)->tp->type==ARRAY)

//グローバル変数 ----------------------------------------
#ifndef EXTERN
#define EXTERN extern
#endif
// トークナイズした結果のトークン列はこのVectorに保存する
EXTERN Vector *token_vec;
EXTERN Token **tokens;  //token_vec->data;
EXTERN int token_pos;   //tokensの現在位置

//文字列リテラル
EXTERN Vector *string_vec;

//グローバル変数
EXTERN Map *global_vardef_map;     //key=name, value=Vardef

//現在の関数定義
EXTERN Funcdef *cur_funcdef;

//識別子（関数コール）の管理
//プログラム内で定義された関数と、外部関数の両方を含む
EXTERN Map *func_map;       //key=name, value=Funcdef

//プログラム（関数定義）の管理
EXTERN Map *funcdef_map;    //key=name, value=Funcdef

//ローカル変数が使用しているスタックサイズ
EXTERN int var_stack_size;

// parse.c
int size_of(const Type *tp);
void tokenize(char *p);
void print_tokens(void);
void program(void);

// codegen.c
void print_functions(void);

// util.c
Vector *new_vector(void);
void vec_push(Vector *vec, void *elem);

Map *new_map(void);
void map_put(Map *map, char *key, void *val);
int map_get(const Map *map, char *key, void**val);

const char* get_type_str(const Type *tp);
const char* get_func_args_str(const Node *node);

void error(const char*fmt, ...);
void warning(const char*fmt, ...);
void run_test(void);
