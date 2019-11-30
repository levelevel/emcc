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
    control_line        = "#" "include" pp_tokens new_line
                        | "#" "define" identifier replacement_list new-_ine
                        | "#" "define" identifier lparen identifier_list? ")" replacement_list new_line
                        | "#" "define" identifier lparen "..." ")" replacement_list new_line
                        | "#" "define" identifier lparen identifier_list "," "..." ")" replacement_list new_line
                        | "#" "undef" identifier new_line
                        | "#" "line" pp_tokens new_line
                        | "#" "error" pp_tokens? new_line
                        | "#" "pragma" pp_tokens? new_line
                        | "#" new_line
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
#define if_is_active() (cur_ppif_stat==PPIF_NULL||cur_ppif_stat==PPIF_TRUE)

//トークンを次の改行まで進めながら出力する
static void print_line(void) {
    PPToken **token = &pptokens[pptoken_pos];
    for (;(*token)->type != PPTK_EOF; token++) {
        if (if_is_active()) fprintf(g_fp, "%.*s", (*token)->len, (*token)->input);
        pptoken_pos++;
        if ((*token)->type == PPTK_NEWLINE) break;
    }
    if (!if_is_active()) fprintf(g_fp, "\n");
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
    *valp = pptokens[pptoken_pos]->val;
    pptoken_pos++;
    return 1;
}

//次のトークンが識別子(TK_IDENT)かどうかをチェックし、
//その場合はnameを取得し、入力を1トークン読み進めて真を返す
int ppconsume_ident(char **name) {
    skip_space();
    if (pptokens[pptoken_pos]->type != PPTK_IDENT) return 0;
    *name = pptokens[pptoken_pos]->ident;
    pptoken_pos++;
    return 1;
}

static PPToken *consume_directive(void) {
    int save_pptoken_pos = pptoken_pos;
    skip_space();
    if (ppconsume('#')) {
        skip_space();
        PPToken *token = pptokens[pptoken_pos];
        if (token->type>=PPTK_IF && token->type<=PPTK_PRAGMA) {
            pptoken_pos++;
            return token;
        }
    }
    pptoken_pos = save_pptoken_pos;
    return NULL;
}
static PPToken *next_is_directive(void) {
    PPToken *token;
    int save_pptoken_pos = pptoken_pos;
    token = consume_directive();
    pptoken_pos = save_pptoken_pos;
    return token;
}

static void check_ifblock(void) {
    if (ppif_stat_stack->len<2) error_at(input_str(), "対応する#ifがありません");
}

static int group_parts(void);
static int if_section(PPTKtype type);
static int if_group(PPTKtype type);
static int elif_group(void);
static int else_group(void);
static int endif_group(void);
static int control_line(PPTKtype type);
static int constant_expression(long *valp);

void preprocessing_file(void) {
    ppif_stat_stack = new_istack();
    cur_ppif_stat = PPIF_NULL;
    istack_push(ppif_stat_stack, cur_ppif_stat);
    if (!ppconsume(PPTK_EOF)) {
        group_parts();
    }
}

/*
    group_part          = if_section
                        | control_line
                        | text_line
                        | "#" non_directive
 */
static int group_parts(void) {
    while (!ppconsume(PPTK_EOF)) {
        PPToken *next_token = next_is_directive();
        if (next_token) {
            if (if_section(next_token->type)) continue;
            if (control_line(next_token->type)) continue;
            return 0;
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
    char *name;
    int is_true;
    switch (type) {
    case PPTK_IF:
        consume_directive();
        long val;
        if (!constant_expression(&val)) error_at(input_str(), "定数式が必要です");
        is_true = val;
        break;
    case PPTK_IFDEF:
        consume_directive();
        if (!ppconsume_ident(&name)) error_at(input_str(), "識別子が必要です");
        is_true = map_get(define_map, name, NULL);
        break;
    case PPTK_IFNDEF:
        consume_directive();
        if (!ppconsume_ident(&name)) error_at(input_str(), "識別子が必要です");
        is_true = !map_get(define_map, name, NULL);
        break;
        break;
    default:
        return 0;
    }

    if (if_is_active()) cur_ppif_stat = is_true?PPIF_TRUE:PPIF_FALSE;
    else             cur_ppif_stat = PPIF_SKIP;
    istack_push(ppif_stat_stack, cur_ppif_stat);
    skip_line();
    group_parts();
    return 1;
}
//    else_group          = "#" "else" new_line group_part*
static int elif_group(void) {
    PPToken *token = next_is_directive();
    if (token->type != PPTK_ELIF) return 0;
    check_ifblock();
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
    PPToken *token = next_is_directive();
    if (token->type != PPTK_ELSE) return 0;
    check_ifblock();
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
    PPToken *token = consume_directive();
    if (token->type!=PPTK_ENDIF) return 0;
    check_ifblock();
    skip_line();
    istack_pop(ppif_stat_stack);
    cur_ppif_stat = istack_get(ppif_stat_stack, ppif_stat_stack->len-1);
    return 1;
}

/*
    control_line        = "#" "include" pp_tokens new_line
                        | "#" "define" identifier replacement_list new-_ine
                        | "#" "define" identifier lparen identifier_list? ")" replacement_list new_line
                        | "#" "define" identifier lparen "..." ")" replacement_list new_line
                        | "#" "define" identifier lparen identifier_list "," "..." ")" replacement_list new_line
                        | "#" "undef" identifier new_line
                        | "#" "line" pp_tokens new_line
                        | "#" "error" pp_tokens? new_line
                        | "#" "pragma" pp_tokens? new_line
                        | "#" new_line
 */
static int control_line(PPTKtype type) {
    if (type==PPTK_DEFINE) {
        char *name;
        consume_directive();
        if (!ppconsume_ident(&name)) error_at(input_str(), "識別子が必要です");
        map_put(define_map, name, NULL);
    } else if (type==PPTK_UNDEF) {
        char *name;
        consume_directive();
        if (!ppconsume_ident(&name)) error_at(input_str(), "識別子が必要です");
        map_del(define_map, name);
    } else {
        return 0;
    }

    skip_line();
    return 1;
}

static int constant_expression(long *valp) {
    skip_space();
    if (ppconsume_num(valp)) return 1;
    return 0;
}