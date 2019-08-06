#include <errno.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <setjmp.h>

//可変長ベクタ ---------------------------------------
typedef struct {
    void **data;
    int capacity;
    int len;
} Vector;
#define vec_len(vec) (vec)->len
#define vec_data(vec,i) ((vec)->data[i])
#define lst_len vec_len
#define lst_data vec_data

//マップ --------------------------------------------
typedef struct {
    Vector *keys;
    Vector *vals;
} Map;
#define map_len(map) vec_len((map)->vals)
#define map_data(map,i) vec_data((map)->vals,i)

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
    TK_ENUM,        //enum
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
    TK_TYPEDEF,     //storage_class
    TK_INC,         // ++
    TK_DEC,         // --
    TK_EQ,          // ==
    TK_NE,          // !=
    TK_LE,          // <=
    TK_GE,          // >=
    TK_LAND,        // &&
    TK_LOR,         // ||
    TK_SHIFTR,      // >>
    TK_SHIFTL,      // <<
    TK_PLUS_ASSIGN, // +=
    TK_MINUS_ASSIGN,// -=
    TK_GOTO,        //goto
    TK_CONTINUE,    //continue
    TK_BREAK,       //break
    TK_RETURN,      //return
    TK_IF,          //if
    TK_ELSE,        //else
    TK_SWITCH,
    TK_CASE,
    TK_DEFAULT,
    TK_WHILE,       //while
    TK_DO,          //do
    TK_FOR,         //for
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
    ND_BNOT  = '~',
    ND_NUM,         //整数のノードの型
    ND_STRING,      //文字列リテラル    name=文字列, val=string_vecのindex
    ND_TYPE_DECL,   //型の宣言　例：int; enum ABC {A,B,C}; enum ABC; struct ST; 
    ND_IDENT,       //IDENT:中間的なタイプであり、最終的にND_LOCAL_VARなどに置き換わる
    ND_ENUM_DEF,    //enum定義          name=enum名/NULL, lst=node(ND_ENUM)/NULL
    ND_ENUM,        //enum要素          name=要素名, val=値, lhs=node(ND_ENUN_DEF)
    ND_TYPEDEF,     //typedef           name=typedef名, tp->sclass=SC_TYPEDEF
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
    ND_SHIFTR,      // >>
    ND_SHIFTL,      // <<
    ND_TRI_COND,    // A ? B : C（三項演算子）
    ND_PLUS_ASSIGN, // +=
    ND_MINUS_ASSIGN,// -=
    ND_LOCAL_VAR_DEF,   //int A=B;      name=A, rhs=Node（"A=B"の形式の初期化式、初期値がない場合はNULL）
    ND_GLOBAL_VAR_DEF,  //int A=B;      同上
    ND_IF,          // if(A)B else C    lhs->lhs=A, lhs->rhs=B, rhs=C
    ND_SWITCH,      // switch(A)B       lhs=A, rhs=B, lst=node(ND_CASE,ND_DEFAULT)
    ND_LABEL,       // label:B          name=label, rhs=B
    ND_CASE,        // case A:B;        val=A(constant), lhs=A, rhs=B, name="case:%ld"
    ND_DEFAULT,     // default:A        rhs=A, name="default"
    ND_WHILE,       // while(A)B        lhs=A, rhs=B
    ND_DO,          // do A while(B);   lhs=A, rhs=B
    ND_FOR,         // for(A;B;C)D      lhs->lhs=A, lhs->rhs=B, rhs->lhs=C, rhs->rhs=D
    ND_GOTO,        // goto label;      name=label
    ND_CONTINUE,
    ND_BREAK,
    ND_RETURN,      // rhs=expression
    ND_BLOCK,       //{ }               lst=ノード(declaration/statement)
    ND_LIST,        //コンマリスト
    ND_FUNC_CALL,   //関数コール        name=関数名, lhs=引数リスト(ND_LIST)/NULL, 
                    //                 rhs=ND_FUNC_DEF|DECL/ND_LOCAL|GLOBAL_VAR_DEF(FUNC)
    ND_FUNC_DEF,    //関数定義          lhs=引数リスト(ND_LIST), rhs=ブロック(ND_BLOCK：関数本体)
    ND_FUNC_DECL,   //関数宣言          lhs=引数リスト(ND_LIST)
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
    ENUM,
    PTR,
    ARRAY,
    VARARGS,        //...
    FUNC,           //関数
    CONST,          //const処理の一時的なデータ構造でのみ使用し、必ずptr_ofを持つ。
                    //親をconstで修飾する。親がいないときは型を修飾する。
    NEST,           //ネストした型宣言処理の一時的なデータ構造でのみ使用する。他のメンバーは未使用。
} TPType;

typedef enum {
    SC_UNDEF,
    SC_AUTO,
    SC_REGISTER,
    SC_STATIC,
    SC_EXTERN,
    SC_TYPEDEF,
} StorageClass;

typedef struct Type Type;
typedef struct Node Node;
typedef Vector Stack;

struct Type {
    TPType          type;
    char            is_unsigned;    //unsigned型
    char            is_const;
    StorageClass    sclass;
    Type            *ptr_of;        //typeがPTR,ARRAY,FUNCの場合次のType
    Node            *node;          //typeがFUNCの場合のND_FUNC_DEFのノード
                                    //typeがENUMの場合のND_ENUM_DEFのノード
    long            array_size;     //typeがARRAYの場合の配列サイズ。未定義の場合は-1
};

struct Node {
    NDtype type;    //nodeの型：演算子、ND_INDENTなど
    char unused;    //無効（重複した宣言など：コード生成時には無視する）
    int offset;     //auto変数：ベースポインタからのoffset
                    //static変数：識別用index（global_index）
    long val;       //typeがND_NUMの場合の値
                    //typeがND_STRINGの場合のstring_vecのインデックス
    Node *lhs;
    Node *rhs;
    union {
    Vector *lst;    //typeがND_BLOCKの場合のstatementのリスト
                    //typeがND_LISTの場合のassignmentのリスト
    Map *map;       //typeがND_SWITCHの場合のND_CASEのマップ: key=node->val, val=node(ND_CASE)
    };
    char *name;     //typeがND_LOCAL|GLOBAL_VAR[_DEF]の場合の変数名
                    //typeがND_FUNC_CALL|DEF|DECLの場合の関数名
    Type *tp;       //型情報
    char *input;    //トークン文字列（エラーメッセージ用）。Token.inputと同じ。
};

typedef struct {
    char    *name;          //関数名
    Node    *node;          //ND_FUNC_DEFのnode
    Type    *tp;            //関数の型(常にFUNC)
    Map     *symbol_map;    //通常の識別子：key=name, val=Node(ND_LOCAL_VAR_DEFなど)
    Map     *tagname_map;   //タグ名：key=name, val=Node(ND_ENUN_DEFなど)
    Map     *label_map;     //ラベル：key=name, val=Node(ND_LABEL)
    int     var_stack_size; //ローカル変数のために必要となるスタックサイズ（offset）
} Funcdef;

//型がinteger型であるか
#define type_is_integer(_tp) (CHAR<=(_tp)->type && (_tp)->type<=ENUM)

//型・ノードがポインタ型（PTR||ARRAY）であるか
#define type_is_ptr(_tp) ((_tp)->type==PTR || (_tp)->type==ARRAY)
#define node_is_ptr(_node) type_is_ptr((_node)->tp)
#define node_is_var_def(node) ((node)->type==ND_LOCAL_VAR_DEF || (node)->type==ND_GLOBAL_VAR_DEF)

//アサーション
#define COMPILE_ERROR 0
#define _ERROR_ assert(COMPILE_ERROR)
#define _NOT_YET_(node) error_at((node)->input, "未実装です（%s:%d in %s）",__FILE__,__LINE__,__func__) 

//グローバル変数 ----------------------------------------
#ifndef EXTERN
#define EXTERN extern
#endif

// トークナイズした結果のトークン列はこのVectorに保存する
EXTERN Vector *token_vec;
EXTERN Token **tokens;  //token_vec->data;
EXTERN int token_pos;   //tokensの現在位置

//break時のジャンプ先のラベルを示す
EXTERN char *break_label;
//continue時のジャンプ先のラベルを示す
EXTERN char *continue_label;

//文字列リテラル
typedef struct String {
    const char *str;
    int size;                   //予約
    char unused;                //.text領域に出力する必要なし
} String;
EXTERN Vector *string_vec;      //value=String

//staticシンボル
EXTERN Vector *static_var_vec;  //value=Node

//グローバルシンボル
EXTERN Map *global_symbol_map;  //通常の識別子：key=name, val=Node(ND_GLOBAL_VAR_DEFなど)
EXTERN Map *global_tagname_map; //タグ名：key=name, val=Node(ND_ENUN_DEFなど)

//スコープごとのシンボルの管理
//（グローバルシンボル(global_symbol_map)→関数のローカルシンボル(cur_funcdef->ident_map)→ブロックのシンボル→...）
EXTERN Stack *symbol_stack;     //value=Map
EXTERN Stack *tagname_stack;    //value=Map

//現在の関数定義
EXTERN Funcdef *cur_funcdef;

//プログラム（関数定義）の管理
EXTERN Map *funcdef_map;    //key=name, value=Funcdef

//ラベル・static変数のユニークなIDを生成するためのindex
EXTERN int global_index;

//現在処理中のswitch文: cur_switch->valをラベルの識別indexとして用いる
EXTERN Node *cur_switch;

typedef enum {
    ERC_CONTINUE,
    ERC_EXIT,
    ERC_LONGJMP,
    ERC_ABORT,
} ErCtrl;
EXTERN ErCtrl error_ctrl;       // エラー発生時の処理
EXTERN ErCtrl warning_ctrl;
EXTERN ErCtrl note_ctrl;
EXTERN jmp_buf jmpbuf;
EXTERN int error_cnt;
EXTERN int warning_cnt;
EXTERN int note_cnt;
#define SET_ERROR_WITH_NOTE  {note_ctrl = error_ctrl; error_ctrl = ERC_CONTINUE;}

//現在のトークン（エラー箇所）の入力文字列
#define input_str() (tokens[token_pos]->input)

//トークンへのアクセス
#define token_str()  (tokens[token_pos]->str)
#define token_type() (tokens[token_pos]->type)
#define token_is(_tp) (token_type()==(_tp))
#define next_token_str()  (tokens[token_pos+1]->str)
#define next_token_type() (tokens[token_pos+1]->type)
#define next_token_is(_tp) (next_token_type()==(_tp))

// main.c
void compile(void);

// tokenize.c
char *escape_str(const char *str);
void tokenize(char *p);
void dump_tokens(void);
int token_is_type_spec(void);
int next_token_is_type_spec(void);

#ifdef _PARSE_C_
int consume(TKtype type);
int consume_typedef(Node **node);
int consume_num(long *valp);
int consume_string(char **str);
int consume_ident(char**name);
void expect(TKtype type);
void expect_ident(char**name, const char*str);
#endif

// parse_util.c
long size_of(const Type *tp);
int align_of(const Type *tp);
int node_is_const(Node *node, long *val);
int node_is_const_or_address(Node *node, long *valp, Node **varp);
#define type_is_static(tp) (get_storage_class(tp)==SC_STATIC)
#define type_is_extern(tp) (get_storage_class(tp)==SC_EXTERN)
#define type_is_typedef(tp) (get_storage_class(tp)==SC_TYPEDEF)
#define node_is_local_static_var(node) ((node)->type==ND_LOCAL_VAR && type_is_static((node)->tp))
StorageClass get_storage_class(Type *tp);
int new_string(const char *str);
const char* get_string(int index);
void unuse_string(int index);

#ifdef _PARSE_C_
Node *search_symbol(const char *name);
void regist_var_def(Node *node);
void regist_func(Node *node, int full_check);
void regist_symbol(Node *node);
void regist_tagname(Node *node);
void regist_label(Node *node);
void regist_case(Node *node);

Type *get_typeof(Type *tp);
void check_return(Node *node);
void check_func_return(Funcdef *funcdef);
void check_funcargs(Node *node, int def_mode);
void check_funccall(Node *node);
int type_eq(const Type *tp1, const Type *tp2);
int type_eq_global_local(const Type *tp1, const Type *tp2);
int type_eq_func(const Type *tp1, const Type *tp2);
int type_eq_assign(const Type *tp1, const Type *tp2);
int get_var_offset(const Type *tp);
Funcdef *new_funcdef(void);
Type *new_type_ptr(Type*ptr);
Type *new_type_func(Type*ptr, Node *node);
Type *new_type_array(Type*ptr, size_t size);
Type *new_type(int type, int is_unsigned);
Node *new_node(int type, Node *lhs, Node *rhs, Type *tp, char *input);
Node *new_node(int type, Node *lhs, Node *rhs, Type *tp, char *input);
Node *new_node_num(long val, char *input);
Node *new_node_var_def(char *name, Type*tp, char *input);
Node *new_node_string(char *string, char *input);
Node *new_node_ident(char *name, char *input);
Node *new_node_func_call(char *name, char *input);
Node *new_node_func(char *name, Type *tp, char *input);
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
#define stack_len  vec_len
#define stack_data vec_data

char* get_type_str(const Type *tp);
char* get_func_args_str(const Node *node);
const char *get_NDtype_str(NDtype type);
void dump_node(const Node *node, const char *str);
void dump_type(const Type *tp, const char *str);

EXTERN char *filename;
EXTERN char *user_input;
void error_at(const char*loc, const char*fmt, ...);
void warning_at(const char*loc, const char*fmt, ...);
void note_at(const char*loc, const char*fmt, ...);
void error(const char*fmt, ...);
void warning(const char*fmt, ...);
void run_test(void);

void test_error(void);
