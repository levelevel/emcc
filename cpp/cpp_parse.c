#include "emcpp.h"

/*  文法：
- A.3 Preprocessing directives http://port70.net/~nsz/c/c11/n1570.html#A.3
    preprocessing_file  = group_part*
    group_part          = if_section
                        | control_line
                        | text_line
                        | "#" non_directive
    if_section          = if_group elif_group? else_group? endif_line
    if_group            = "#" "if" constant_expression new_line group_part*
                        = "#" "ifdef" identifier new_line group_part*
                        = "#" "ifndef" identifier new_line group_part*
    elif_group          = "#" "elif" constant_expression new_line group_part*
    else_group          = "#" "else" new_line group_part*
    endif_line          = "#" "endif" new_line
    pp_token            = 
*/

typedef enum {  // if状態を示す
    PPIF_NULL,  // 有効（#if～#endifの外側）
    PPIF_TRUE,  // 有効（#if 1の状態）、次の#elifや#elseで常にPPIF_SKIPになる
    PPIF_FALSE, // 無効（#if 0の状態）、次に#elseがくればPPIF_TRUEになる
    PPIF_SKIP,  // #endifまで無効
} PPIFstate;

static PPIFstate cur_ppif_stat;     //現在のif状態
static iStack *ppif_stat_stack;     //PPITstate
#define is_active() (cur_ppif_stat==PPIF_NULL||cur_ppif_stat==PPIF_TRUE)

//トークンを次の改行まで進めながら出力する
static void print_line(void) {
    PPToken **token = &pptokens[pptoken_pos];
    for (;(*token)->type != PPTK_EOF; token++) {
        if (is_active()) fprintf(g_fp, "%.*s", (*token)->len, (*token)->input);
        pptoken_pos++;
        if ((*token)->type == PPTK_NEWLINE) break;
    }
    if (!is_active()) fprintf(g_fp, "\n");
}

//トークン上のスペースをスキップする
static void skip_space(void) {
    while (pptokens[pptoken_pos]->type == PPTK_SPACE) pptoken_pos++;
}

//トークンを次の改行までスキップする
static void skip_line(void) {
    while (pptokens[pptoken_pos]->type != PPTK_NEWLINE) pptoken_pos++;
    pptoken_pos++;
    fprintf(g_fp, "\n");
}

//次のトークンが期待したものかどうかをチェックし、
//期待したものの場合だけ入力を1トークン読み進めて真を返す
static int ppconsume(PPTKtype type) {
    if (pptokens[pptoken_pos]->type != type) return 0;
    pptoken_pos++;
    return 1;
}

//次のトークンが数値(PPTK_NUM)かどうかをチェックし、
//その場合は数値を取得し、入力を1トークン読み進めて真を返す
int ppconsume_num(long *valp) {
    if (pptokens[pptoken_pos]->type != PPTK_NUM) return 0;
    *valp = strtol(pptokens[pptoken_pos]->input, NULL, 10);
    pptoken_pos++;
    return 1;
}

static PPTKtype consume_directive(void) {
    int save_pptoken_pos = pptoken_pos;
    skip_space();
    if (ppconsume('#')) {
        skip_space();
        PPTKtype type = pptokens[pptoken_pos]->type;
        if (type>=PPTK_IF && type<=PPTK_PRAGMA) {
            pptoken_pos++;
            return type;
        }
    }
    pptoken_pos = save_pptoken_pos;
    return 0;
}
static PPTKtype next_is_directive(void) {
    PPTKtype type;
    int save_pptoken_pos = pptoken_pos;
    type = consume_directive();
    pptoken_pos = save_pptoken_pos;
    return type;
}

static int group_parts(void);
static int if_section(PPTKtype type);
static int if_group(PPTKtype type);
static int elif_group(void);
static int else_group(void);
static int endif_group(void);
static int constant_expression(long *valp);

void preprocessing_file(void) {
    ppif_stat_stack = new_istack();
    cur_ppif_stat = PPIF_NULL;
    istack_push(ppif_stat_stack, cur_ppif_stat);
    if (!ppconsume(PPTK_EOF)) {
        group_parts();
    }
}

static int group_parts(void) {
    while (!ppconsume(PPTK_EOF)) {
        PPTKtype type = next_is_directive();
        if (type!=0) {
            if (!if_section(type)) return 0;
        } else {
            print_line();
        }
    }
    return 1;
}

/*
    if_section          = if_group elif_group? else_group? endif_line
    if_group            = "#" "if" constant_expression new_line group_part*
                        = "#" "ifdef" identifier new_line group_part*
                        = "#" "ifndef" identifier new_line group_part*
    elif_group          = "#" "elif" constant_expression new_line group_part*
    else_group          = "#" "else" new_line group_part*
    endif_line          = "#" "endif" new_line
*/
static int if_section(PPTKtype type) {
    if (!if_group(type)) return 0;
    elif_group();
    else_group();
    if (!endif_group()) error_at(input_str(), "#endifがありません");

    return 1;
}
/*
    if_group            = "#" "if" constant_expression new_line group_part*
                        = "#" "ifdef" identifier new_line group_part*
                        = "#" "ifndef" identifier new_line group_part*
*/
static int if_group(PPTKtype type) {
    long val = 0;
    switch (type) {
    case PPTK_IF:
        consume_directive();
        if (!constant_expression(&val)) error_at(input_str(), "定数式が必要です");
        if (is_active()) cur_ppif_stat = val?PPIF_TRUE:PPIF_FALSE;
        else             cur_ppif_stat = PPIF_SKIP;
        istack_push(ppif_stat_stack, cur_ppif_stat);
        break;
    case PPTK_IFDEF:
        break;
    case PPTK_IFNDEF:
        break;
    default:
        return 0;
    }
    skip_line();
    group_parts();
    return 1;
}
//    else_group          = "#" "else" new_line group_part*
static int elif_group(void) {
    PPTKtype type = next_is_directive();
    if (type != PPTK_ELIF) return 0;
    if (cur_ppif_stat==PPIF_TRUE) {
        cur_ppif_stat = PPIF_SKIP;
    } else if (cur_ppif_stat==PPIF_FALSE) {
        long val = 0;
        consume_directive();
        if (!constant_expression(&val)) error_at(input_str(), "定数式が必要です");
        if (val) cur_ppif_stat = PPIF_TRUE;
    } else {
        cur_ppif_stat = PPIF_SKIP;
    }
    skip_line();
    group_parts();
    return 1;
}
//    else_group          = "#" "else" new_line group_part*
static int else_group(void) {
    PPTKtype type = next_is_directive();
    if (type != PPTK_ELSE) return 0;
    if (cur_ppif_stat==PPIF_TRUE) {
        cur_ppif_stat = PPIF_SKIP;
    } else if (cur_ppif_stat==PPIF_FALSE) {
        cur_ppif_stat = PPIF_TRUE;
    } else {
        cur_ppif_stat = PPIF_SKIP;
    }
    skip_line();
    group_parts();
    return 1;
}
//    endif_line          = "#" "endif" new_line
static int endif_group(void) {
    PPTKtype type = consume_directive();
    if (type!=PPTK_ENDIF) return 0;
    skip_line();
    istack_pop(ppif_stat_stack);
    cur_ppif_stat = istack_get(ppif_stat_stack, ppif_stat_stack->len-1);
    return 1;
}

static int constant_expression(long *valp) {
    skip_space();
    if (ppconsume_num(valp)) return 1;
    return 0;
}