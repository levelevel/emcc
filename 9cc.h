#include <errno.h>
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
    TK_SHORT,       //short
    TK_INT,         //int
    TK_LONG,        //long
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
    TK_BREAK,       //break
    TK_CONTINUE,    //continue
    TK_SIZEOF,      //sizeof
    TK_ALIGNOF,     //_Alignof (C11)
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
    ND_LOCAL_VAR_DEF,   //ローカル変数の定義
    ND_GLOBAL_VAR_DEF,  //グローバル変数の定義
    ND_RETURN,
    ND_IF,
    ND_WHILE,
    ND_FOR,
    ND_BREAK,
    ND_CONTINUE,
    ND_BLOCK,       //{...}
    ND_LIST,        //コンマリスト
    ND_FUNC_CALL,   //関数コール
    ND_FUNC_DEF,    //関数定義
    ND_EMPTY,       //空のノード
} NDtype;

typedef struct _Type Type;
struct _Type {
    enum {CHAR, SHORT, INT, LONG, LONGLONG, PTR, ARRAY} type;
    Type *ptr_of;
    long array_size;  //typeがARRAYの場合の配列サイズ。未定義の場合は-1
};

typedef struct _Node Node;
struct _Node {
    NDtype type;    //nodeの型：演算子、ND_INDENTなど
    Node *lhs;
    Node *rhs;
    Vector *lst;    //typeがND_BLOCKの場合のstmtのリスト
                    //typeがND_LISTの場合のasignのリスト
    long val;       //typeがND_NUMの場合の値
                    //typeがND_STRINGの場合のstring_vecのインデックス
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
    int var_stack_size; //ローカル変数のために必要となるスタックサイズ（offset）
} Funcdef;

//型がinteger型であるか
#define type_is_integer(_tp) (CHAR<=(_tp)->type && (_tp)->type<=LONGLONG)

//ノードがポインタ（PTR||ARRAY）であるか
#define node_is_ptr(_node) ((_node)->tp->type==PTR || (_node)->tp->type==ARRAY)

//アサーション
#define COMPILE_ERROR 0
#define _ERROR_ assert(COMPILE_ERROR)
#define _NOT_YET_(node) error_at((node)->input, "未実装です（%s:%d）",__FILE__,__LINE__) 

//グローバル変数 ----------------------------------------
#ifndef EXTERN
#define EXTERN extern
#endif

EXTERN int verbose;

// トークナイズした結果のトークン列はこのVectorに保存する
EXTERN Vector *token_vec;
EXTERN Token **tokens;  //token_vec->data;
EXTERN int token_pos;   //tokensの現在位置

//break時のジャンプ先のラベルを示すスタック
EXTERN Vector *break_stack;     //value=文字列
//continue時のジャンプ先のラベルを示すスタック
EXTERN Vector *continue_stack;  //value=文字列
#define stack_push(vec, elem) vec_push(vec, elem)
#define stack_pop(vec) ((vec)->len--)
#define stack_get(vec) vec_get(vec, vec->len - 1)

//文字列リテラル
EXTERN Vector *string_vec;      //value=文字列リテラル

//グローバル変数
EXTERN Map *global_vardef_map;     //key=name, value=Vardef

//現在の関数定義
EXTERN Funcdef *cur_funcdef;

//識別子（関数コール）の管理
//プログラム内で定義された関数と、外部関数の両方を含む
EXTERN Map *func_map;       //key=name, value=Funcdef

//プログラム（関数定義）の管理
EXTERN Map *funcdef_map;    //key=name, value=Funcdef

// parse.c
long size_of(const Type *tp);
int align_of(const Type *tp);
void tokenize(char *p);
void print_tokens(void);
void program(void);
int node_is_const(Node *node, long *val);
int node_is_const_or_address(Node *node, long *valp, Node **varp);

// codegen.c
void print_functions(void);

// util.c
Vector *new_vector(void);
void vec_push(Vector *vec, void *elem);
void *vec_get(Vector *vec, int idx);

Map *new_map(void);
void map_put(Map *map, char *key, void *val);
int map_get(const Map *map, char *key, void**val);

const char* get_type_str(const Type *tp);
const char* get_func_args_str(const Node *node);

EXTERN char *filename;
EXTERN char *user_input;
void error_at(const char*loc, const char*fmt, ...);
void warning_at(const char*loc, const char*fmt, ...);
void error(const char*fmt, ...);
void warning(const char*fmt, ...);
void run_test(void);
