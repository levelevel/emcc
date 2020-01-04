#ifdef _emcc
#include "gcc_def.h"
#endif

#include <errno.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <setjmp.h>

//エラー状態
typedef enum {
    ST_ERR=0,
    ST_OK=1,
    ST_WARN=2,
}Status;

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
#define get_lst_node(vec,i) ((Node*)vec_data(vec,i))

typedef struct {
    int *data;
    int capacity;
    int len;
} iVector;

//マップ --------------------------------------------
typedef struct {
    Vector *keys;
    Vector *vals;
} Map;
#define map_len(map) vec_len((map)->vals)
#define map_data(map,i) vec_data((map)->vals,i)

typedef Vector Stack;
typedef iVector iStack;

#ifndef EXTERN
#define EXTERN extern
#endif

EXTERN char *g_cur_filename;//論理的な処理中のファイル（includeファイルを指す場合がある）
EXTERN int   g_cur_line;
EXTERN char *g_filename;    //コンパイル対象のソースファイル（コマンドラインで指定されたもの）
EXTERN char *g_user_input;

//エラー出力のためのソースファイルの情報
typedef struct {
    char *input;    //トークン文字列（エラーメッセージ用）
    char *file;     //ソースファイル名
    int line;       //行番号
    int col;        //カラム位置
} SrcInfo;
void error_at  (const SrcInfo *info, const char *fmt, ...);
void warning_at(const SrcInfo *info, const char *fmt, ...);
void note_at   (const SrcInfo *info, const char *fmt, ...);
void error     (const char *fmt, ...);
void warning   (const char *fmt, ...);
void run_test(void);

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

// util.c
Vector *new_vector(void);
void vec_push(Vector *vec, void *elem);
void *vec_get(Vector *vec, int idx);
void *vec_del(Vector *vec, int idx);
void vec_copy(Vector *dst, Vector *src);

iVector *new_ivector(void);
void ivec_push(iVector *vec, int elem);

Map *new_map(void);
void map_put(Map *map, const char *key, void *val);
int map_get(const Map *map, const char *key, void**val);
int map_del(Map *map, const char *key);

Stack *new_stack(void);
int   stack_push(Stack *stack, void*elem);
void *stack_pop(Stack *stack);
void *stack_get(Stack *stack, int idx);
#define stack_top(stack) stack_get(stack,stack->len-1)
#define stack_len  vec_len
#define stack_data vec_data

iStack *new_istack(void);
int istack_push(iStack *stack, int elem);
int istack_pop(iStack *stack);
int istack_get(iStack *stack, int idx);

int is_alnum(int c);
int is_alpha(int c);
int is_xdigit(int c);

char *read_file(const char *path);
