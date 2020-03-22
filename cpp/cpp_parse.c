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

//カッコで囲まれたマクロの実引数を1個取得する：(A,B)
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
//マクロの実引数を1個取得する
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
//macro_rangeで示されるトークン列を（マクロ展開は行わないで）##演算子だけを展開して
//新しいトークン列を生成し、macro_rangeに設定する: token1##token2 -> token1token2
//case1:
// macro_range(in)  : [0]token1 [1]## [2]token2 [3]token3 ...
// macro_range(out) : [0]token1token2 [1]token3 ...
//case2: token1の実引数がt11 t12、token2の実引数がt21 t22の場合
// macro_range(in)  : [0]token1    [1]## [2]token2    [3]token3 ...
//                    [0]tk11 tk12 [1]## [2]tk21 tk22 [3]token2 ...
//                            <---------------->連結範囲
// macro_range(out) : [0]tk11   [1]tk12tk21   [2]tk22 [3]token3 ...
#define ADD_TOKEN(t,r) {\
    vec_push(pptoken_vec, t);\
    pptokens = (PPToken**)pptoken_vec->data;\
    r->len++;}
static void expand_dsharp(PPTKrange *macro_range, Map *arg_map) {
    int start = macro_range->start;
    int len   = macro_range->len;
    macro_range->start = lst_len(pptoken_vec);    //戻り値：pptoken_vecの最後にトークン列を追加する
    macro_range->len = 0;
    for (int i=0; i<len; i++) {
        PPToken *token1 = pptokens[start+i];
        while (i<len-2 && pptokens[start+i+1]->type == PPTK_DSHARP) {
            PPToken *token2 = pptokens[start+i+2];
            //[0]=token1 [1]## [2]token2 [3]token3 ...
            //token1とtoken2を実引数に展開
            PPTKrange *range1=NULL, *range2=NULL;
            if (arg_map && token1->type==PPTK_IDENT && map_get(arg_map, token1->ident, (void**)&range1)) {
                //展開した最後のトークンをtoken1とし、残りは出力。
                token1 = pptokens[range1->start+range1->len-1];
                for (int j=0; j<range1->len-1; j++) ADD_TOKEN(pptokens[range1->start+j], macro_range);
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
            token1 = token12;
            i += 2;
            //[0]=token12 [1]token3 ...
            if (range2 && range2->len>1) {
                //展開した最後のトークンをtoken1とし、元のtoken1と残りは出力。
                vec_push(pptoken_vec, token1);
                pptokens = (PPToken**)pptoken_vec->data;
                macro_range->len++;
                for (int j=1; j<range2->len-1; j++) ADD_TOKEN(pptokens[range2->start+j], macro_range);
                token1 = pptokens[range2->start+range2->len-1];
            }
        }
        ADD_TOKEN(token1, macro_range);
    }
    //static PPToken token_EOF = {PPTK_EOF};
    //ADD_TOKEN(&token_EOF, macro_range);
}
//macro_rangeで示されるトークン列を（マクロ展開は行わないで）#演算子だけを展開して
//新しいトークン列を生成し、macro_rangeに設定する: #token -> "token"
// macro_range(in)  : [0]# [1]token1  [2]token2 ...
// macro_range(out) :     [0]"token1" [1]token2 ...
static void expand_sharp(PPTKrange *macro_range, Map *arg_map) {
    int start = macro_range->start;
    int len   = macro_range->len;
    macro_range->start = lst_len(pptoken_vec);    //戻り値：pptoken_vecの最後にトークン列を追加する
    macro_range->len = 0;
    for (int i=0; i<len; i++) {
        PPToken *token0 = pptokens[start+i];
        if (i<len-1 && token0->type == PPTK_SHARP) {
            PPToken *token1 = pptokens[start+i+1];  //文字列化するトークン
            PPTKrange *from_str_range = NULL;       //文字列化するトークン列はパラメータでなけらばならない
            if (arg_map && token1->type==PPTK_IDENT) {
                map_get(arg_map, token1->ident, (void**)&from_str_range);
            }
            if (from_str_range==NULL) error_at(&token0->info, "#の次がマクロのパラメータではありません");
            PPToken *str_token = calloc(sizeof(PPToken), 1);  //文字列化後のトークン
            str_token->type = PPTK_STRING;
            str_token->info = token0->info;
            int buf_len = 2;    //""
            for (int j=0; j<from_str_range->len; j++) {
                buf_len += pptokens[from_str_range->start+j]->len*2;  //2倍分のバッファを確保
            }
            char *p = str_token->info.input = malloc(buf_len+1);
            *p++ = '"';
            for (int j=0; j<from_str_range->len; j++) {
                PPToken *token = pptokens[from_str_range->start+j];
                char *q = token->info.input;
                for (int len=token->len; len; len--) {
                    if (token->type==PPTK_STRING && *q=='\\') *p++ = '\\';
                    if (*q=='"')  *p++ = '\\';
                    *p++ = *q++;
                    if (token->type==PPTK_SPACE) break; //複数スペースは1個にまとめる
                }
            }
            *p++ = '"';
            str_token->len = p - str_token->info.input;
            token0 = str_token;
            i++;
        }
        ADD_TOKEN(token0, macro_range);
    }
    //dump_token_range(macro_range, __func__);
}
//arg_mapに沿って仮引数を展開して
//新しいトークン列を生成し、macro_rangeに設定する： MAC(x) -> MAC(a)
static void expand_args(PPTKrange *macro_range, Map *arg_map) {
    int start = macro_range->start;
    int len   = macro_range->len;
    macro_range->start = lst_len(pptoken_vec);    //戻り値：pptoken_vecの最後にトークン列を追加する
    macro_range->len = 0;
    for (int i=0; i<len; i++) {
        PPToken *token = pptokens[start+i];
        PPTKrange *actual_arg_range = NULL;    //実引数
        if (token->type==PPTK_IDENT) {
            map_get(arg_map, token->ident, (void**)&actual_arg_range);
        }
        if (actual_arg_range) {
            for (int j=0; j<actual_arg_range->len; j++) {
                ADD_TOKEN(pptokens[actual_arg_range->start+j], macro_range);
            }
        } else {
            ADD_TOKEN(token, macro_range);
        }
    }
}
//tokenで示される識別子がマクロの場合に展開する
//マクロ展開された分だけpptoken_posが進む
static int expand_macro(PPToken *token) {
    assert(token->type==PPTK_IDENT);
    PPMacro *macro;
    if (!map_get(define_map, token->ident, (void*)&macro)) return 0;
    if (macro->in_use) return 0;    //多重展開防止
    macro->in_use = 1;

    Map *arg_map = NULL;
    if (macro->args) {
        if (!ppconsume('(')) return 0;
        arg_map = get_arg_map(macro->args);
        ppexpect(')');
    }

    PPTKrange macro_range = macro->range;
    //##演算子と#演算子は先に処理する。macro_rangeの中身は##/#演算子だけを処理した新しいトークン列となる。
    for (int i=0; i<macro_range.len; i++) {
        if (pptokens[macro_range.start+i]->type==PPTK_DSHARP) {
            expand_dsharp(&macro_range, arg_map);
            break;
        }
    }
    for (int i=0; i<macro_range.len; i++) {
        if (pptokens[macro_range.start+i]->type==PPTK_SHARP) {
            expand_sharp(&macro_range, arg_map);
            break;
        }
    }

    //仮引数を実引数に置換
    if (arg_map) expand_args(&macro_range, arg_map);

    //再帰的にマクロを展開しつつ出力
    for (int i=0; i<macro_range.len; i++) {
        PPToken *token = pptokens[macro_range.start+i];
        if (token->type==PPTK_IDENT) {
            int org_pptoken_pos = pptoken_pos;
            pptoken_pos = macro_range.start+i+1;
            int ret = expand_macro(token);
            if (ret) i += pptoken_pos - (macro_range.start+i+1);
            pptoken_pos = org_pptoken_pos;
            if (ret) continue;
        }
        fprintf(g_fp, "%.*s", token->len, token->info.input);
    }
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