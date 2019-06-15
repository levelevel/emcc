#define _PARSE_C_

#include "9cc.h"

//型のサイズ
// - 配列の場合、要素のサイズ*要素数
// - 構造体の場合、アラインメントで単位切り上げ
long size_of(const Type *tp) {
    assert(tp);
    switch (tp->type) {
    case VOID:     return sizeof(void);
    case CHAR:     return sizeof(char);
    case SHORT:    return sizeof(short);
    case INT:      return sizeof(int);
    case LONG:     return sizeof(long);
    case LONGLONG: return sizeof(long long);
    case PTR:      return sizeof(void*);
    case ARRAY:
        if (tp->array_size<0) return sizeof(void*);
        else return tp->array_size * size_of(tp->ptr_of);
    case FUNC:     return sizeof(void*);
    case CONST:    assert(0);
    }
    _ERROR_;
    return -1;
}

//型のアラインメント
// - 配列の場合、要素のアラインメント
// - 構造体の場合、メンバー内の最大のアラインメント
int align_of(const Type *tp) {
    assert(tp);
    if (tp->type==ARRAY) return align_of(tp->ptr_of);
    return size_of(tp);
}

int node_is_const(Node *node, long *valp) {
    /*代入は副作用を伴うので定数とはみなさない。
    if (node->type == '=') {
        return node_is_const(node->rhs, val);   //厳密にはlhsへの型変換を考慮すべき？
    }*/

    long val, val1, val2;

    if (node->type==ND_TRI_COND) {
        if (!node_is_const(node->lhs, &val)) return 0;
        if (val) {
            if (!node_is_const(node->rhs->lhs, &val)) return 0;
        } else {
            if (!node_is_const(node->rhs->rhs, &val)) return 0;
        }
        if (valp) *valp = val;
        return 1;
    }

    if (node->lhs && !node_is_const(node->lhs, &val1)) return 0;
    if (node->rhs && !node_is_const(node->rhs, &val2)) return 0;
    switch ((int)node->type) {
    case ND_NUM:  val = node->val;    break;
    case ND_LAND: val = val1 && val2; break;
    case ND_LOR:  val = val1 || val2; break;
    case '&':     val = val1 &  val2; break;
    case '^':     val = val1 ^  val2; break;
    case '|':     val = val1 |  val2; break;
    case ND_EQ:   val = val1 == val2; break;
    case ND_NE:   val = val1 != val2; break;
    case '<':     val = val1 <  val2; break;
    case ND_LE:   val = val1 <= val2; break;
    case '+':     val = val1 +  val2; break;
    case '-':     val = val1 -  val2; break;
    case '*':     val = val1 *  val2; break;
    case '/':     val = val1 /  val2; break;
    case '%':     val = val1 %  val2; break;
    case '!':     val = !val1;        break;
    default:
        return 0;
    }
    if (valp) *valp = val;
    return 1;
}

//nodeがアドレス+定数の形式になっているかどうかを調べる。varpにND_ADDRESS(&var)のノード、valpに定数を返す
int node_is_const_or_address(Node *node, long *valp, Node **varp) {
    long val, val1, val2;
    Node *var1=NULL, *var2=NULL;

    if (node->type==ND_ADDRESS &&
        (node->rhs->type==ND_GLOBAL_VAR || type_is_static(node->rhs->tp))) {
        if (varp) *varp = node;
        if (valp) *valp = 0;
        return 1;
    }
    if ((node->type==ND_GLOBAL_VAR || type_is_static(node->tp))
        && node->tp->type==ARRAY) {
        if (varp) *varp = new_node(ND_ADDRESS, NULL, node, node->tp->ptr_of, node->input);
        if (valp) *valp = 0;
        return 1;
    }
    if (node->type==ND_LIST) {
        Node **nodes = (Node**)node->lst->data;
        if (node->lst->len==1)
            return node_is_const_or_address(nodes[0], valp, varp);
        return 0;
    }

    if (node->lhs && !node_is_const_or_address(node->lhs, &val1, &var1)) return 0;
    if (node->rhs && !node_is_const_or_address(node->rhs, &val2, &var2)) return 0;
    if (var1 && var2) {
        return 0;
    } else if (var1) {
        if (node->type=='+' || node->type=='-') {
            if (varp) *varp = var1;
        } else {
            return 0;
        }
    } else if (var2) {
        if (node->type=='+') {
            if (varp) *varp = var2;
        } else {
            return 0;
        }
    }

    switch ((int)node->type) {
    case ND_NUM:  val = node->val;    break;
    case ND_LAND: val = val1 && val2; break;
    case ND_LOR:  val = val1 || val2; break;
    case '&':     val = val1 &  val2; break;    //bitwise and
    case '^':     val = val1 ^  val2; break;
    case '|':     val = val1 |  val2; break;
    case ND_EQ:   val = val1 == val2; break;
    case ND_NE:   val = val1 != val2; break;
    case '<':     val = val1 <  val2; break;
    case ND_LE:   val = val1 <= val2; break;
    case '+':     val = val1 +  val2; break;
    case '-':     val = val1 -  val2; break;
    case '*':     val = val1 *  val2; break;
    case '/':     val = val1 /  val2; break;
    case '%':     val = val1 %  val2; break;
    case '!':     val = !val1;        break;
    default:
        return 0;
    }
    if (valp) *valp = val;
    return 1;
}

// static char*p; のような宣言ではPTRにはis_staticが設定されず、
// charだけに設定されているのでcharのところまで見に行く
int type_is_static(Type *tp) {
    if (tp->ptr_of) return type_is_static(tp->ptr_of);
    return tp->is_static;
}

int type_is_extern(Type *tp) {
    if (tp->ptr_of) return type_is_extern(tp->ptr_of);
    return tp->is_extern;
}

// ==================================================
// 以下はparse.cローカル

//次のトークンが期待した型かどうかをチェックし、
//期待した型の場合だけ入力を1トークン読み進めて真を返す
int consume(TKtype type) {
    if (tokens[token_pos]->type != type) return 0;
    token_pos++;
    return 1;
}

//次のトークンが識別子(TK_IDENT)かどうかをチェックし、
//その場合はnameを取得し、入力を1トークン読み進めて真を返す
int consume_ident(char**name) {
    if (tokens[token_pos]->type != TK_IDENT) return 0;
    *name = tokens[token_pos]->str;
    token_pos++;
    return 1;
}

//storage classを外したTypeの複製を返す。
Type *get_typeof(Type *tp) {
    Type *ret = malloc(sizeof(Type));
    memcpy(ret, tp, sizeof(Type));
    ret->is_static = 0;
    ret->is_extern = 0;
    if (tp->ptr_of) ret->ptr_of = get_typeof(tp->ptr_of);
    return ret;
}

//引数リスト(ND_LIST)の妥当性を確認
//関数定義(def_mode=1)の場合は名前付き宣言(declaration)であることを確認
void check_funcargs(Node *node, int def_mode) {
    int size = node->lst->len;
    Node **arg_nodes = (Node**)node->lst->data;
    for (int i=0; i < size; i++) {
        Node *arg = arg_nodes[i];
        if (arg->type==ND_VARARGS) continue;
        if (arg->tp->type==VOID) {
            if (i>0) error_at(arg->input, "ここではvoidを指定できません");
            continue;
        }
        if (def_mode && arg->name==NULL)
            error_at(arg->input, "関数定義の引数には名前が必要です");
    }
}

//ノードのタイプが等しいかどうかを判定する
int node_type_eq(const Type *tp1, const Type *tp2) {
    if ((tp1->type==PTR && tp2->type==ARRAY) ||
        (type_is_integer(tp1) && type_is_integer(tp2))) {
        ;   //一致とみなす（tp1=tp2の代入前提）
    } else {
        if (tp1->type != tp2->type) return 0;
    }  
    if (tp1->ptr_of) return node_type_eq(tp1->ptr_of, tp2->ptr_of);
    return 1;
}

//ローカル変数のRBPからのoffset（バイト数）を返し、var_stack_sizeを更新する。
int get_var_offset(const Type *tp) {
    int size = size_of(tp);
    int align_size = align_of(tp);
    cur_funcdef->var_stack_size += size;
    // アラインメント（align_sizeバイト単位に切り上げ）
    if (size>0) cur_funcdef->var_stack_size = (cur_funcdef->var_stack_size + (size-1))/align_size * align_size;
    return cur_funcdef->var_stack_size;
}

//関数定義のroot生成
Funcdef *new_funcdef(void) {
    Funcdef * funcdef;
    funcdef = calloc(1, sizeof(Funcdef));
    funcdef->ident_map = new_map();
    return funcdef;
}

//型情報の生成
Type *new_type_ptr(Type*ptr) {
    Type *tp = calloc(1, sizeof(Type));
    tp->type = PTR;
    tp->ptr_of = ptr;
    return tp;
}

Type *new_type_array(Type*ptr, size_t size) {
    Type *tp = calloc(1, sizeof(Type));
    tp->type = ARRAY;
    tp->ptr_of = ptr;
    tp->array_size = size;
    return tp;
}

Type *new_type(int type, int is_unsigned) {
    Type *tp = calloc(1, sizeof(Type));
    tp->type = type;
    tp->is_unsigned = is_unsigned;
    return tp;
}

//抽象構文木の生成（演算子）
Node *new_node(int type, Node *lhs, Node *rhs, Type *tp, char *input) {
    Node *node = calloc(1, sizeof(Node));
    node->type = type;
    node->lhs  = lhs;
    node->rhs  = rhs;
    node->tp   = tp;
    node->input = input;
    return node;
}

//抽象構文木の生成（数値）
Node *new_node_num(long val, char *input) {
    NDtype type = (val>UINT_MAX || val<INT_MIN) ? LONG : INT;
    Node *node = new_node(ND_NUM, NULL, NULL, new_type(type, 0), input);
    node->val = val;
//    fprintf(stderr, "val=%ld\n", val);
    return node;
}

//未登録の変数であれば登録する
void regist_var_def(Node *node) {
    char *name = node->name;
    if (cur_funcdef && !type_is_extern(node->tp)) {  //関数内かつexternでなければローカル変数
        if (map_get(cur_funcdef->ident_map, name, NULL)==0) {
            Symdef *symdef = calloc(1, sizeof(Symdef));
            symdef->name = name;
            symdef->node = node;
            if (type_is_static(node->tp)) {
                symdef->offset = global_index++;
            } else {
                symdef->offset = get_var_offset(node->tp);
            }
            node->type = ND_LOCAL_VAR_DEF;
            map_put(cur_funcdef->ident_map, name, symdef);
        } else {
            error_at(node->input, "'%s'はローカル変数の重複定義です", name);
        }
    } else {            //グローバル変数
        if (node->type==0) node->type = ND_GLOBAL_VAR_DEF;
        if (map_get(global_symdef_map, name, NULL)==0) {
            Symdef *symdef = calloc(1, sizeof(Symdef));
            symdef->name = name;
            symdef->node = node;
            map_put(global_symdef_map, name, symdef);
        } else if (!type_is_extern(node->tp)) {
            error_at(node->input, "'%s'はグローバル変数の重複定義です", name);
        }
    }
}

//抽象構文木の生成（変数定義）
Node *new_node_var_def(char *name, Type*tp, char *input) {
    Node *node = new_node(0, NULL, NULL, tp, input);
    node->name = name;
    if (name) regist_var_def(node);
    return node;
}

//抽象構文木の生成（文字列リテラル）
Node *new_node_string(char *string, char *input) {
    Type *tp = new_type_array(new_type(CHAR, 0), strlen(string)+1);
    Node *node = new_node(ND_STRING, NULL, NULL, tp, input);
    node->val = string_vec->len;    //インデックス
    vec_push(string_vec, string);

    return node;
}

//抽象構文木の生成（識別子：ローカル変数・グローバル変数）
Node *new_node_var(char *name, char *input) {
    Node *node;
    NDtype type;
    Symdef *symdef;

    //定義済みの変数であるかをチェック
    if (cur_funcdef && map_get(cur_funcdef->ident_map, name, (void**)&symdef)!=0) {
        type = ND_LOCAL_VAR;
    } else if (map_get(global_symdef_map, name, (void**)&symdef)!=0) {
        type = ND_GLOBAL_VAR;
    } else {
        error_at(tokens[token_pos-1]->input, "'%s'は未定義の変数です", name);
    }

    node = new_node(type, NULL, NULL, symdef->node->tp, input);
    node->name = name;

    return node;
}

//抽象構文木の生成（関数コール）
Node *new_node_func_call(char *name, char *input) {
    Symdef *symdef = NULL;
    if (map_get(global_symdef_map, name, (void**)&symdef)==0) {
        warning_at(input, "未宣言の関数コールです。");
    }
    Node *node = new_node(ND_FUNC_CALL, NULL, NULL, symdef?symdef->node->tp:new_type(INT, 0), input);

    node->name = name;
//  node->lhs    //引数リスト

    return node;
}

//抽象構文木の生成（関数定義）
Node *new_node_func_def(char *name, Type *tp, char *input) {
    Node *node = new_node(ND_FUNC_DEF, NULL, NULL, tp, input);
    node->name = name;
//  node->lhs       //引数リスト(ND_LIST)
//  node->rhs       //ブロック(ND_BLOCK)

    cur_funcdef = new_funcdef();
    cur_funcdef->tp = tp;
    cur_funcdef->node = node;
    cur_funcdef->name = node->name;

    //関数を登録する
    Symdef *symdef;
    if (map_get(global_symdef_map, name, (void**)&symdef)==0)  {
        symdef = calloc(1, sizeof(Symdef));
        symdef->name = name;
        symdef->node = node;
        map_put(global_symdef_map, name, symdef);
    } else if (symdef->node->type==ND_FUNC_DEF) {
        error_at(input, "関数が再定義されています");
    } else if (symdef->node->type==ND_GLOBAL_VAR_DEF) {
        error_at(input, "'%s'は異なる種類のシンボルとして再定義されています", name);
    }
    return node;
}

//抽象構文木の生成（空文）
Node *new_node_empty(char *input) {
    Node *node = new_node(ND_EMPTY, NULL, NULL, NULL, input);
    return node;
}

//抽象構文木の生成（ブロック）
Node *new_node_block(char *input) {
    Node *node = new_node(ND_BLOCK, NULL, NULL, NULL, input);
    node->lst  = new_vector();
    return node;
}

//抽象構文木の生成（コンマリスト）
//型は最後の要素の型であるべきだがここでは設定しない
Node *new_node_list(Node *item, char *input) {
    Node *node = new_node(ND_LIST, NULL, NULL, NULL, input);
    node->lst  = new_vector();
    if (item) vec_push(node->lst, item);
    return node;
}
