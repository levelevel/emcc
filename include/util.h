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

//マップ --------------------------------------------
typedef struct {
    Vector *keys;
    Vector *vals;
} Map;
#define map_len(map) vec_len((map)->vals)
#define map_data(map,i) vec_data((map)->vals,i)

typedef Vector Stack;

void error_at(const char*loc, const char*fmt, ...);
void warning_at(const char*loc, const char*fmt, ...);
void note_at(const char*loc, const char*fmt, ...);
void error(const char*fmt, ...);
void warning(const char*fmt, ...);

#ifndef EXTERN
#define EXTERN extern
#endif

EXTERN char *filename;
EXTERN char *user_input;
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

