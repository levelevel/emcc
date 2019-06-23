#include <errno.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

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
    TK_VOID,        //void
    TK_CHAR,        //char
    TK_SHORT,       //short
    TK_INT,         //int
    TK_LONG,        //long
    TK_TYPEOF,      //typeof（非標準）
    TK_SIGNED,      //signed
    TK_UNSIGNED,    //unsigned
//  TK_VOLATILE,    //type_qualifier
//  TK_RESTRICT,    //type_qualifier
    TK_CONST,       //type_qualifier
    TK_AUTO,        //storage_class
    TK_REGISTER,    //storage_class
    TK_STATIC,      //storage_class
    TK_EXTERN,      //storage_class
    TK_INC,         // ++
    TK_DEC,         // --
    TK_EQ,          // ==
    TK_NE,          // !=
    TK_LE,          // <=
    TK_GE,          // >=
    TK_LAND,        // &&
    TK_LOR,         // ||
    TK_PLUS_ASSIGN, // +=
    TK_MINUS_ASSIGN,// -=
    TK_RETURN,      //return
    TK_IF,          //if
    TK_ELSE,        //else
    TK_WHILE,       //while
    TK_FOR,         //for
    TK_BREAK,       //break
    TK_CONTINUE,    //continue
    TK_SIZEOF,      //sizeof
    TK_ALIGNOF,     //_Alignof (C11)
    TK_3DOTS,       // ...
    TK_EOF,         //入力の終わり
} TKtype;

typedef struct {
    TKtype type;    //トークンの型
    long val;       //typeがTK_TOKENの場合、値
    char *str;      //typeがTK_STRING/TK_IDENTの場合、その文字列
    char *input;    //トークン文字列（エラーメッセージ用）
} Token;

//抽象構文木 ----------------------------------------
typedef enum {
    ND_UNDEF = 0,
    ND_NOT   = '!',
    ND_MOD   = '%',
    ND_AND   = '&',
    ND_MUL   = '*',
    ND_PLUS  = '+',
    ND_MINUS = '-',
    ND_DIV   = '/',
    ND_LT    = '<',
    ND_ASSIGN= '=',
    ND_GT    = '>',
    ND_XOR   = '^',
    ND_OR    = '|',
    ND_NUM,         //整数のノードの型
    ND_STRING,
    ND_IDENT,       //IDENT:中間的なタイプであり、最終的にND_LOCAL_VARなどに置き換わる
    ND_LOCAL_VAR,   //ローカル変数の参照
    ND_GLOBAL_VAR,  //グローバル変数の参照
    ND_CAST,        //キャスト
    ND_INC,         // a++
    ND_DEC,         // a--
    ND_INC_PRE,     // ++a
    ND_DEC_PRE,     // --a
    ND_INDIRECT,    // *（間接参照）
    ND_ADDRESS,     // &（アドレス演算子）
    ND_EQ,          // ==
    ND_NE,          // !=
    ND_LE,          // <=, >=
    ND_LAND,        // &&
    ND_LOR,         // ||
    ND_TRI_COND,    // A ? B : C（三項演算子）
    ND_PLUS_ASSIGN, // +=
    ND_MINUS_ASSIGN,// -=
    ND_LOCAL_VAR_DEF,   //ローカル変数の定義・宣言
    ND_GLOBAL_VAR_DEF,  //グローバル変数の定義・宣言
    ND_RETURN,
    ND_IF,
    ND_WHILE,
    ND_FOR,
    ND_BREAK,
    ND_CONTINUE,
    ND_BLOCK,       //{ }
    ND_LIST,        //コンマリスト
    ND_FUNC_CALL,   //関数コール
    ND_FUNC_DEF,    //関数定義
    ND_FUNC_DECL,   //関数宣言
    ND_VARARGS,     //...
    ND_EMPTY,       //空のノード
} NDtype;

typedef enum {
    VOID = 1,
    CHAR,
    SHORT,
    INT,
    LONG,
    LONGLONG,
    PTR,
    ARRAY,
    FUNC,           //関数
    CONST,          //const処理の一時的なデータ構造でのみ使用し、必ずptr_ofを持つ。
                    //親をconstで修飾する。親がいないときは型を修飾する。
    NEST,           //ネストした型宣言処理の一時的なデータ構造でのみ使用する。他のメンバーは未使用。
} Typ;

typedef enum {
    SC_UNDEF,
    SC_AUTO,
    SC_REGISTER,
    SC_STATIC,
    SC_EXTERN,
} StorageClass;

typedef struct _Type Type;
typedef struct _Node Node;
typedef Vector Stack;

struct _Type {
    Typ type;
    char is_unsigned;   //unsigned型
    char is_const;
    StorageClass sclass;
    Type *ptr_of;
    Node *node;         //typeがFUNCの場合のND_FUNC_DEFのノード
    long array_size;    //typeがARRAYの場合の配列サイズ。未定義の場合は-1
};

struct _Node {
    NDtype type;    //nodeの型：演算子、ND_INDENTなど
    int offset;     //auto変数：ベースポインタからのoffset
                    //static変数：識別用index（global_index）
    long val;       //typeがND_NUMの場合の値
                    //typeがND_STRINGの場合のstring_vecのインデックス
    Node *lhs;
    Node *rhs;
    Vector *lst;    //typeがND_BLOCKの場合のstatementのリスト
                    //typeがND_LISTの場合のassignmentのリスト
    char *name;     //typeがND_LOCAL|GLOBAL_VAR[_DEF]の場合の変数名
                    //typeがND_FUNC_CALL|DEF|DECLの場合の関数名
    Type *tp;       //型情報
    char *input;    //トークン文字列（エラーメッセージ用）。Token.inputと同じ。
};

typedef struct {
    char *name;     //関数名
    Node *node;     //ND_FUNC_DEFのnode
    Type *tp;       //関数の型情報
    Map *ident_map; //ローカル変数：key=name, val=Node
    int var_stack_size; //ローカル変数のために必要となるスタックサイズ（offset）
} Funcdef;

//型がinteger型であるか
#define type_is_integer(_tp) (CHAR<=(_tp)->type && (_tp)->type<=LONGLONG)

//型・ノードがポインタ型（PTR||ARRAY）であるか
#define type_is_ptr(_tp) ((_tp)->type==PTR || (_tp)->type==ARRAY)
#define node_is_ptr(_node) type_is_ptr((_node)->tp)

//アサーション
#define COMPILE_ERROR 0
#define _ERROR_ assert(COMPILE_ERROR)
#define _NOT_YET_(node) error_at((node)->input, "未実装です（%s:%d in %s）",__FILE__,__LINE__,__func__) 

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

//文字列リテラル
EXTERN Vector *string_vec;      //value=文字列リテラル

//staticシンボル
EXTERN Vector *static_var_vec;  //value=node

//グローバルシンボル
EXTERN Map *global_symbol_map;  //key=name, value=Node

//スコープごとのシンボルの管理（グローバルシンボル(cur_funcdef->ident_map)→関数のローカルシンボル→ブロックのシンボル→...）
EXTERN Stack *symbol_stack;     //value=node

//現在の関数定義
EXTERN Funcdef *cur_funcdef;

//プログラム（関数定義）の管理
EXTERN Map *funcdef_map;    //key=name, value=Funcdef

//static変数用のユニークなIDを生成するためのindex
EXTERN int global_index;

//現在のトークン（エラー箇所）の入力文字列
#define input_str() (tokens[token_pos]->input)
//現在のトークンの型が引数と一致しているか
#define token_type() (tokens[token_pos]->type)
#define token_is(_tp) (token_type()==(_tp))
#define token_is_type_spec() (TK_VOID<=token_type() && token_type()<=TK_EXTERN)
#define next_token_type() (tokens[token_pos+1]->type)
#define next_token_is(_tp) (next_token_type()==(_tp))
#define next_token_is_type_spec() (TK_VOID<=next_token_type() && next_token_type()<=TK_EXTERN)

// tokenize.c
void tokenize(char *p);
void dump_tokens(void);

// parse_util.c
long size_of(const Type *tp);
int align_of(const Type *tp);
int node_is_const(Node *node, long *val);
int node_is_const_or_address(Node *node, long *valp, Node **varp);
int type_is_static(Type *tp);
int type_is_extern(Type *tp);

#ifdef _PARSE_C_
int consume(TKtype type);
int consume_ident(char**name);
Type *get_typeof(Type *tp);
void check_funcargs(Node *node, int def_mode);
int type_eq(const Type *tp1, const Type *tp2);
int node_type_eq(const Type *tp1, const Type *tp2);
int get_var_offset(const Type *tp);
Funcdef *new_funcdef(void);
Type *new_type_ptr(Type*ptr);
Type *new_type_func(Type*ptr);
Type *new_type_array(Type*ptr, size_t size);
Type *new_type(int type, int is_unsigned);
Node *new_node(int type, Node *lhs, Node *rhs, Type *tp, char *input);
Node *new_node(int type, Node *lhs, Node *rhs, Type *tp, char *input);
Node *new_node_num(long val, char *input);
void regist_var_def(Node *node);
Node *new_node_var_def(char *name, Type*tp, char *input);
Node *new_node_string(char *string, char *input);
Node *new_node_var(char *name, char *input);
Node *new_node_func_call(char *name, char *input);
Node *new_node_func_def(char *name, Type *tp, char *input);
Node *new_node_empty(char *input);
Node *new_node_block(char *input);
Node *new_node_list(Node *item, char *input);
#endif

// parse.c
void translation_unit(void);

// codegen.c
void gen_program(void);

// util.c
Vector *new_vector(void);
void vec_push(Vector *vec, void *elem);
void *vec_get(Vector *vec, int idx);

Map *new_map(void);
void map_put(Map *map, const char *key, void *val);
int  map_get(const Map *map, const char *key, void**val);

Stack *new_stack(void);
int   stack_push(Stack *stack, void*elem);
void *stack_pop(Stack *stack);
void *stack_get(Stack *stack, int idx);
#define stack_top(stack) stack_get(stack,stack->len-1)

char* get_type_str(const Type *tp);
char* get_func_args_str(const Node *node);
void dump_node(const Node *node, const char *str);

EXTERN char *filename;
EXTERN char *user_input;
void error_at(const char*loc, const char*fmt, ...);
void warning_at(const char*loc, const char*fmt, ...);
void error(const char*fmt, ...);
void warning(const char*fmt, ...);
void run_test(void);
