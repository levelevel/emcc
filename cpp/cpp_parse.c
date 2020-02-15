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
                        | "#" "define" identifier replacement_list new_line
                        | "#" "define" identifier lparen identifier_list? ")" replacement_list new_line
                        | "#" "define" identifier lparen "..." ")" replacement_list new_line
                        | "#" "define" identifier lparen identifier_list "," "..." ")" replacement_list new_line
                        | "#" "undef" identifier new_line
                        | "#" "line" pp_tokens new_line
                        | "#" "error" pp_tokens? new_line
                        | "#" "pragma" pp_tokens? new_line
                        | "#" new_line
    identifier_list     = identifier ( "," identifier )*
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

//次の改行までのトークンの数を返す。改行とその前のスペースはカウントしない
static int count_until_newline(void) {
    int pos = pptoken_pos;
    while (pptokens[pos]->type != PPTK_NEWLINE) pos++;
    pos--;
    while (pptokens[pos]->type == PPTK_SPACE) pos--;
    return pos - pptoken_pos + 1;
}

//次のトークンが期待したものかどうかをチェックし、
//期待したものの場合だけ入力を1トークン読み進めて真を返す
static int ppconsume_token(PPTKtype type) {
    if (pptokens[pptoken_pos]->type != type) return 0;
    pptoken_pos++;
    return 1;
}

//次のトークンが期待したものかどうかをチェックし、
//期待したものの場合だけ入力を1トークン読み進めて真を返す。スペースは読み飛ばす。
static int ppconsume(PPTKtype type) {
    int save_pptoken_pos = pptoken_pos;
    skip_space();
    if (pptokens[pptoken_pos]->type != type) {
        pptoken_pos = save_pptoken_pos;
        return 0;
    }
    pptoken_pos++;
    return 1;
}

//次のトークンが期待したものでない場合はエラーとする。スペースは読み飛ばす。
static void ppexpect(PPTKtype type) {
    skip_space();
    if (pptokens[pptoken_pos]->type != type) error_at(&cur_token_info(), "%cがありません", type);
    pptoken_pos++;
}

//次のトークンが数値(PPTK_NUM)かどうかをチェックし、
//その場合は数値を取得し、入力を1トークン読み進めて真を返す。スペースは読み飛ばす。
int ppconsume_num(long *valp) {
    skip_space();
    if (pptokens[pptoken_pos]->type != PPTK_NUM) return 0;
    *valp = pptokens[pptoken_pos]->val;
    pptoken_pos++;
    return 1;
}

//次のトークンが識別子(TK_IDENT)かどうかをチェックし、
//その場合はnameを取得し、入力を1トークン読み進めて真を返す。スペースは読み飛ばす。
int ppconsume_ident(char **name) {
    skip_space();
    if (pptokens[pptoken_pos]->type != PPTK_IDENT) return 0;
    *name = pptokens[pptoken_pos]->ident;
    pptoken_pos++;
    return 1;
}

//次のトークンが#かどうかをチェックし、
//その場合はnameを取得し、入力を1トークン読み進めて真を返す。スペースは読み飛ばす。
static PPToken *consume_directive(void) {
    int save_pptoken_pos = pptoken_pos;
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
    if (ppif_stat_stack->len<2) error_at(&cur_token_info(), "対応する#ifがありません");
}

static int group_parts(void);
static int expand_macro(PPToken *token);
static void text_line(void);
static int if_section(PPTKtype type);
static int if_group(PPTKtype type);
static int elif_group(void);
static int else_group(void);
static int endif_group(void);
static int control_line(PPTKtype type);
static int constant_expression(long *valp);
static void define_macro(const char*name);
static Vector *identifier_list(void);

void preprocessing_file(void) {
    ppif_stat_stack = new_istack();
    cur_ppif_stat = PPIF_NULL;
    istack_push(ppif_stat_stack, cur_ppif_stat);
    if (!ppconsume_token(PPTK_EOF)) {
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
    while (!ppconsume_token(PPTK_EOF)) {
        PPToken *next_token = next_is_directive();
        if (next_token) {
            if (if_section(next_token->type)) continue;
            if (control_line(next_token->type)) continue;
            return 0;
        } else {
            text_line();
        }
    }
    return 1;
}

//カッコで囲まれたマクロの引数を1個取得する：(A,B)
static void read_actual_arg_paren(PPTKrange *range) {
    int cont = 1;
    while (cont) {
        switch (cur_token()->type) {
        case PPTK_NUM:
        case PPTK_STRING:
        case PPTK_IDENT:
        case PPTK_PPTOKEN:
        case PPTK_SPACE:
        case '(':
        case ',':
            pptoken_pos++;
            range->len++;
            break;
        case ')':
            pptoken_pos++;
            range->len++;
            cont = 0;
            break;
        default:
            cont = 0;
            break;
        }
    }
}
//マクロの引数を1個取得する
static int read_actual_arg(PPTKrange *range) {
    int cont = 1;
    while (cont) {
        switch (cur_token()->type) {
        case PPTK_NUM:
        case PPTK_STRING:
        case PPTK_IDENT:
        case PPTK_PPTOKEN:
        case PPTK_SPACE:
            pptoken_pos++;
            range->len++;
            break;
        case '(':
            pptoken_pos++;
            range->len++;
            read_actual_arg_paren(range);
            break;
        default:
            cont = 0;
            break;
        }
    }
    return range->len;
}
//マクロの仮引数リスト(arg_lst)から実引数のマップを作成する
static Map *get_arg_map(Vector *arg_lst) {
    Map *arg_map = new_map();
    int size = lst_len(arg_lst);
    for (int i=0; i<size; i++) {
        if (i>0 && !ppconsume(',')) error_at(&cur_token_info(), ",が必要です");
        PPTKrange *range = calloc(sizeof(PPTKrange), 1);
        while (cur_token()->type==PPTK_SPACE) pptoken_pos++;    //実引数の先頭のスペースを読み飛ばす
        range->start = pptoken_pos;
        range->len   = 0;
        read_actual_arg(range);
        while (range->len>0 && pptokens[range->start+range->len-1]->type==PPTK_SPACE) range->len--;   //実引数の最後のスペースを削除
        if (range->len) map_put(arg_map, lst_data(arg_lst, i), (char*)range); //実引数は省略可能
    }
    return arg_map;
}
//##演算子を展開する: token1##token2 -> token1token2
//token1,token2に対してマクロ展開は行わない
//token1token2はマクロ展開の対象
static void expand_ddash(PPTKrange *ret_range, int start, int len, Map *arg_map) {
    ret_range->start = lst_len(pptoken_vec);    //戻り値：pptoken_vecの最後にトークン列を追加する
    ret_range->len = 0;
    PPToken *token1 = pptokens[start];
    PPToken *token2 = pptokens[start+2];
    assert(pptokens[start+1]->type==PPTK_DSHARP);
    PPTKrange *range1=NULL, *range2=NULL;
    if (arg_map && token1->type==PPTK_IDENT && map_get(arg_map, token1->ident, (void**)&range1)) {
        token1 = pptokens[range1->start+range1->len-1];
    }
    if (arg_map && token2->type==PPTK_IDENT && map_get(arg_map, token2->ident, (void**)&range2)) {
        token2 = pptokens[range2->start];
    }
    //token1とtoken2を連結してtoken12を作る
    PPToken *token12 = calloc(sizeof(PPToken), 1);
    *token12 = *token1; 
    token12->len   = token1->len + token2->len;
    token12->ident = token12->info.input = malloc(token12->len+1);
    sprintf(token12->ident, "%.*s%.*s", token1->len, token1->info.input, token2->len, token2->info.input);

    if (range1) {
        for (int i=0; i<range1->len-1; i++) {
            vec_push(pptoken_vec, pptokens[range1->start+i]);
            pptokens = (PPToken**)pptoken_vec->data;
            ret_range->len++;
        }
    }
    vec_push(pptoken_vec, token12);
    pptokens = (PPToken**)pptoken_vec->data;
    ret_range->len++;
    if (range2) {
        for (int i=1; i<range2->len; i++) {
            vec_push(pptoken_vec, pptokens[range2->start+i]);
            pptokens = (PPToken**)pptoken_vec->data;
            ret_range->len++;
        }
    }
    for (int i=3; i<len; i++) {
        vec_push(pptoken_vec, pptokens[start+i]);
        pptokens = (PPToken**)pptoken_vec->data;
        ret_range->len++;
    }
}
//arg_mapに沿って仮引数を展開しつつ、rangeで示されるトークン列を出力する
static void print_token_by_range(PPTKrange *range, Map *arg_map) {
    for (int i=0; i<range->len; i++) {
        PPToken *token = pptokens[range->start+i];
        if (i<range->len-2 && pptokens[range->start+i+1]->type==PPTK_DSHARP) {
            //##演算子は優先処理
            PPTKrange tmp_range;
            expand_ddash(&tmp_range, range->start+i, range->len, arg_map);
            print_token_by_range(&tmp_range, arg_map);
            return;
        }
        if (token->type==PPTK_DSHARP) continue;
        PPTKrange *arg_range;
        if (arg_map && token->type==PPTK_IDENT && map_get(arg_map, token->ident, (void**)&arg_range)) {
            //マクロの仮引数を実引数に置き換える
            print_token_by_range(arg_range, arg_map);
        } else {
            if (token->type==PPTK_IDENT) {
                int org_pptoken_pos = pptoken_pos;
                pptoken_pos = range->start+i+1;
                int ret = expand_macro(token);
                if (ret) i += pptoken_pos - (range->start+i+1);
                pptoken_pos = org_pptoken_pos;
                if (ret) continue;
            }
            fprintf(g_fp, "%.*s", token->len, token->info.input);
        }
    }
}
//tokenで示される識別子がマクロの場合に展開する
static int expand_macro(PPToken *token) {
    assert(token->type==PPTK_IDENT);
    PPMacro *macro;
    Map *arg_map = NULL;
    if (!map_get(define_map, token->ident, (void*)&macro)) return 0;
    if (macro->in_use) return 0;
    if (macro->args) {
        if (!ppconsume('(')) return 0;
        arg_map = get_arg_map(macro->args);
        ppexpect(')');
    }
    macro->in_use = 1;
    print_token_by_range(&macro->range, arg_map);
    macro->in_use = 0;
    return 1;
}

static void text_line(void) {
    PPToken *token;
    while ((token=pptokens[pptoken_pos])->type != PPTK_EOF) {
        pptoken_pos++;
        if (if_is_active()) {
            if (token->type != PPTK_IDENT || !expand_macro(token)) {
                fprintf(g_fp, "%.*s", token->len, token->info.input);
            }
        }
        if (token->type == PPTK_NEWLINE) break;
    }
    if (!if_is_active()) fprintf(g_fp, "\n");
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
    if (!endif_group()) error_at(&cur_token_info(), "#endifがありません");

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
        if (!constant_expression(&val)) error_at(&cur_token_info(), "定数式が必要です");
        is_true = val;
        break;
    case PPTK_IFDEF:
        consume_directive();
        if (!ppconsume_ident(&name)) error_at(&cur_token_info(), "識別子が必要です");
        is_true = map_get(define_map, name, NULL);
        break;
    case PPTK_IFNDEF:
        consume_directive();
        if (!ppconsume_ident(&name)) error_at(&cur_token_info(), "識別子が必要です");
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
        if (!constant_expression(&val)) error_at(&cur_token_info(), "定数式が必要です");
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
                        | "#" "define" identifier replacement_list new_line
                        | "#" "define" identifier lparen identifier_list? ")" replacement_list new_line
                        | "#" "define" identifier lparen "..." ")" replacement_list new_line
                        | "#" "define" identifier lparen identifier_list "," "..." ")" replacement_list new_line
                        | "#" "undef" identifier new_line
                        | "#" "line" pp_tokens new_line
                        | "#" "error" pp_tokens? new_line
                        | "#" "pragma" pp_tokens? new_line
                        | "#" new_line
    identifier_list     = identifier ( "," identifier )*
 */
static int control_line(PPTKtype type) {
    if (type==PPTK_DEFINE) {
        char *name;
        consume_directive();
        if (!ppconsume_ident(&name)) error_at(&cur_token_info(), "識別子が必要です");
        define_macro(name);
    } else if (type==PPTK_UNDEF) {
        char *name;
        consume_directive();
        if (!ppconsume_ident(&name)) error_at(&cur_token_info(), "識別子が必要です");
        map_del(define_map, name);
    } else {
        return 0;
    }

    skip_line();
    return 1;
}

static int constant_expression(long *valp) {
    if (ppconsume_num(valp)) return 1;
#if 1    //定数式でない場合は0とする
    if (valp) *valp = 0;
    return 1;
#else
    return 0;
#endif
}

//マクロ定義を処理する
static void define_macro(const char*name) {
    PPMacro *macro = new_macro(name);
    if (ppconsume_token('(')) { //引数付きマクロ。スペースなしで(がある場合
        skip_space();
        macro->args = identifier_list();
        skip_space();
        if (!ppconsume(')')) error_at(&cur_token_info(), ")がありません");
    }
    skip_space();
    macro->range.start = pptoken_pos;
    macro->range.len   = count_until_newline();   
}
static Vector *identifier_list(void) {
    Vector *lst = new_vector();
    Map *map = new_map();
    char *name;
    if (ppconsume_ident(&name)) {
        vec_push(lst, name);    //第一引数
        map_put(map, name, 0);
        skip_space();
        while (ppconsume(',')) {//二番目以降の引数
            skip_space();
            SrcInfo *info = &cur_token_info();
            if (ppconsume_ident(&name)) {
                if (map_get(map, name, NULL)) error_at(info, "引数%sが重複しています", name);
                vec_push(lst, name);
                map_put(map, name, 0);
                skip_space();
            } else {
                error_at(info, "引数が必要です");
            }
        }
    }
    return lst;
}

PPMacro *new_macro(const char*name) {
    PPMacro *macro = calloc(sizeof(PPMacro), 1);
    macro->name = name;
    map_put(define_map, name, macro);
    return macro;
}