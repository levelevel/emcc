#define _PARSE_C_

#include "emcc.h"

//型のサイズ
// - 配列の場合、要素のサイズ*要素数
// - 構造体の場合、アラインメントで単位切り上げ
long size_of(const Type *tp) {
    assert(tp);
    switch (tp->type) {
    case VOID:     return 1;
    case BOOL:     return sizeof(_Bool);
    case CHAR:     return sizeof(char);
    case SHORT:    return sizeof(short);
    case INT:      return sizeof(int);
    case LONG:     return sizeof(long);
    case LONGLONG: return sizeof(long long);
    case FLOAT:    return sizeof(float);
    case DOUBLE:   return sizeof(double);
    case LONGDOUBLE:return sizeof(long double);
    case ENUM:     return sizeof(int);
    case STRUCT:
    case UNION:    return tp->node->val;
    case PTR:      return sizeof(void*);
    case ARRAY:
        if (tp->array_size<0) return sizeof(void*); //ポインタと同じ扱い
        else                  return tp->array_size * size_of(tp->ptr_of);
    case FUNC:     return sizeof(void*);
    default:    //NEST
        assert(0);
    }
    _ERROR_;
    return -1;
}

//型のアラインメント
// - 配列の場合、要素のアラインメント
// - 構造体の場合、メンバー内の最大のアラインメント
int align_of(const Type *tp) {
    int align = -1;
    assert(tp);
    switch (tp->type) {
    case ARRAY:
        if (tp->array_size<0) align = size_of(tp); //ポインタと同じ扱い
        else                  align = align_of(tp->ptr_of);
        break;
    case STRUCT:
    case UNION:;
        int size = tp->node->lst->len;
        Node **nodes = (Node**)tp->node->lst->data;
        for (int i=0; i<size; i++) {
            int n = align_of(nodes[i]->tp);
            if (n>align) align = n;
        }        
        break;
    default:
        align = size_of(tp);
        break;
    }
    return align;
}

//ローカル変数のRBPからのoffset（バイト数）を返し、var_stack_sizeを更新する。
int get_var_offset(const Type *tp) {
    int size = size_of(tp);
    assert(size);
    int align_size = align_of(tp);
    cur_funcdef->var_stack_size += size;
    // アラインメント（align_sizeバイト単位に切り上げ）
    if (size>0) cur_funcdef->var_stack_size = (cur_funcdef->var_stack_size + (align_size-1))/align_size * align_size;
    return cur_funcdef->var_stack_size;
}

//構造体(共用体)メンバのoffset(member->offset)とパディングを含めたサイズ(member->val)、
//構造体(共用体)のサイズを設定する(node->val)。
//構造体(共用体)のStorage Class属性をメンバにもコピーする。
void set_struct_size(Node *node, int base_offset) {
    Type *tp = node->tp;
    assert(type_is_struct_or_union(tp));
    if (node->lst==NULL) return;
    assert(node->lst->len);
    int max_align_size = 0, max_size = 0, offset = base_offset;
    StorageClass sclass = get_storage_class(tp);
    for (int i=0; i<lst_len(node->lst); i++) {
        Node *member = get_lst_node(node->lst, i);
        assert(member->type==ND_MEMBER_DEF);
        int size = size_of(member->tp);        
        assert(size>0);
        int align_size = align_of(member->tp);
        if (align_size>max_align_size) max_align_size = align_size;
        if (      size>max_size)       max_size       = size;
        offset = (offset +(align_size-1))/align_size * align_size;
        member->offset = (tp->type==STRUCT ? offset : base_offset);
        if (node_is_anonymouse_struct_or_union(member)) {    //無名構造体・共用体
            set_struct_size(member, tp->type==STRUCT ? offset : 0);
        }
        offset += size;
        if (sclass!=get_storage_class(member->tp)) {
            member->tp = dup_type(member->tp);
            set_storage_class(member->tp, sclass);
        }
    }
    int total_size = (offset +(max_align_size-1))/max_align_size * max_align_size;
    node->val = node->type==ND_STRUCT_DEF ? total_size : max_size;
    
    //パディングを含めたサイズ(member->val)を設定する
        int last_offset = node->val;    //全体のサイズ
    for (int i=lst_len(node->lst)-1; i>=0; i--) {
        Node *member = get_lst_node(node->lst, i);
        if (tp->type==STRUCT) {
            member->val = last_offset - member->offset;
            last_offset = member->offset;
        } else {
            member->val = last_offset;
        }
    }
}

static long calc_node(Node *node, long val1, long val2) {
    long val;
    switch ((int)node->type) {
    case ND_LAND:   val = val1 && val2; break;
    case ND_LOR:    val = val1 || val2; break;
    case '&':       val = val1 &  val2; break;
    case '^':       val = val1 ^  val2; break;
    case '|':       val = val1 |  val2; break;
    case ND_EQ:     val = val1 == val2; break;
    case ND_NE:     val = val1 != val2; break;
    case '<':       val = val1 <  val2; break;
    case ND_LE:     val = val1 <= val2; break;
    case ND_SHIFTL: val = val1 << val2; break;
    case ND_SHIFTR: val = val1 >> val2; break;
    case '+':       val = val1 +  val2; break;
    case '-':       val = val1 -  val2; break;
    case '*':       val = val1 *  val2; break;
    case '/':       val = val1 /  val2; break;
    case '%':       val = val1 %  val2; break;
    case '!':       val = !val1;        break;
    case '~':       val = ~val1;        break;
    default:
        assert(0);
    }
    return val;
}

//nodeが定数になっているかどうかを調べる。valpに定数を返す
int node_is_constant(Node *node, long *valp) {
    long val, val1, val2;

    switch (node->type) {
    case ND_NUM:
    case ND_ENUM:
        val = node->val;
        break;
    case ND_TRI_COND:
        if (!node_is_constant(node->lhs, &val)) return 0;
        if (val) {
            if (!node_is_constant(node->rhs->lhs, &val)) return 0;
        } else {
            if (!node_is_constant(node->rhs->rhs, &val)) return 0;
        }
        if (valp) *valp = val;
        return 1;
    case ND_CAST:
        return node_is_constant(node->rhs, valp);
    case ND_LIST:
    case ND_INIT_LIST:
    case ND_LOCAL_VAR:
    case ND_GLOBAL_VAR:
    case ND_STRING:
    case ND_FUNC_CALL:
        return 0;
    default:
        if (node->lhs && !node_is_constant(node->lhs, &val1)) return 0;
        if (node->rhs && !node_is_constant(node->rhs, &val2)) return 0;
        val = calc_node(node, val1, val2);
    }
    if (valp) *valp = val;
    return 1;
}

//nodeが定数またはアドレス+定数の形式になっているかどうかを調べる。
//varpにND_ADDRESS(&var)のノード、valpに定数を返す。
//nodeが文字列リテラル(ND_STRING)の場合は1を返すがvalp/varpには何も返さない。
int node_is_constant_or_address(Node *node, long *valp, Node **varp) {
    long val, val1, val2;
    Node *var1=NULL, *var2=NULL;

    //グローバルな配列、関数は静的なアドレスに変換されるので定数
    if ((node->type==ND_GLOBAL_VAR || node_is_local_static_var(node))
        && (node->tp->type==ARRAY || node->tp->type==FUNC)) {
        if (varp) *varp = new_node(ND_ADDRESS, NULL, node, node->tp->ptr_of, node->token);
        if (valp) *valp = 0;
        return 1;
    }
    //グローバル配列int a[4][5];に対するa[1]は静的なアドレス
    if (node->type==ND_INDIRECT && node->tp->type==ARRAY
        && (node->rhs->type==ND_GLOBAL_VAR || node_is_local_static_var(node->rhs))) {
        if (varp) *varp = node;
        if (valp) *valp = 0;
        return 1;
    }

    switch (node->type) {
    case ND_NUM:
    case ND_ENUM:
        val = node->val;
        break;
    case ND_LIST:
        if (vec_len(node->lst)) {
            int size = vec_len(node->lst);
            for (int i=0; i<size; i++) {
                if (node_is_constant_or_address(vec_data(node->lst, i), valp, varp)==0) return 0;
            }
            return 1;
        }
        return 0;
    case ND_INIT_LIST:
        if (vec_len(node->lst))
            return node_is_constant_or_address(vec_data(node->lst, 0), valp, varp);
        return 0;
    case ND_LOCAL_VAR:
    case ND_GLOBAL_VAR:
        return 0;
    case ND_STRING:
        val = 0;
        break;
    case ND_ADDRESS:
        //dump_node(node,__func__);
        if (node->rhs->type==ND_GLOBAL_VAR || node_is_local_static_var(node->rhs)) {
            if (varp) *varp = node;
            if (valp) *valp = 0;
            return 1;
        } else if (node->rhs->type==ND_INDIRECT) {
            return node_is_constant_or_address(node->rhs->rhs, valp, varp);
        } else {
            return 0;
        }
    case ND_TRI_COND:
        if (!node_is_constant_or_address(node->lhs, &val, varp)) return 0;
        if (val) {
            if (!node_is_constant_or_address(node->rhs->lhs, &val, varp)) return 0;
        } else {
            if (!node_is_constant_or_address(node->rhs->rhs, &val, varp)) return 0;
        }
        if (valp) *valp = val;
        return 1;
    case ND_CAST:
        return node_is_constant_or_address(node->rhs, valp, varp);
    default:
        if (node->lhs && !node_is_constant_or_address(node->lhs, &val1, &var1)) return 0;
        if (node->rhs && !node_is_constant_or_address(node->rhs, &val2, &var2)) return 0;
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
        val = calc_node(node, val1, val2);
    }

    if (valp) *valp = val;
    return 1;
}

// static char*p; のような宣言ではPTRにはsclassが設定されず、
// charだけに設定されているのでcharのところまで見に行く
StorageClass get_storage_class(Type *tp) {
    while (tp->ptr_of) tp = tp->ptr_of;
    return tp->tmp_sclass;
}
void set_storage_class(Type *tp, StorageClass sclass) {
    while (tp->ptr_of) tp = tp->ptr_of;
    tp->tmp_sclass = sclass;
}

int new_string_literal(String *string) {
    StringL *stringL = calloc(1, sizeof(StringL));
    stringL->string = *string;
    vec_push(string_vec, stringL);
    return vec_len(string_vec)-1;
}

String *get_string_literal(int index) {
    assert(index<vec_len(string_vec));
    StringL *stringL = vec_data(string_vec, index);
    return &(stringL->string);
}

void unuse_string_literal(int index) {
    assert(index<vec_len(string_vec));
    StringL *stringL = vec_data(string_vec, index);
    stringL->unused = 1;
}

// ==================================================
// 以下はparse.cローカル

static void error_with_note(const Node *node, const Node *prev, const char *msg1, const char *msg2) {
    SET_ERROR_WITH_NOTE;
    error_at(&node_info(node), msg1);
    note_at(&node_info(prev), msg2);
}

//nameをスコープの内側から検索してNodeを返す
Node *search_symbol(const char *name) {
    Node *node = NULL;
    for (int i=stack_len(symbol_stack)-1; i>=0; i--) {
        Map *symbol_map = (Map*)stack_get(symbol_stack, i);
        if (map_get(symbol_map, name, (void**)&node)!=0) {
            node->unused = 0;
            return node;
        }
    }
    return NULL;
}
Node *search_tagname(const char *name) {
    Node *node = NULL;
    for (int i=stack_len(tagname_stack)-1; i>=0; i--) {
        Map *tagname_map = (Map*)stack_get(tagname_stack, i);
        if (map_get(tagname_map, name, (void**)&node)!=0) {
            return node;
        }
    }
    return NULL;
}

//未登録の変数であればローカル変数またはグローバル変数として登録する
void regist_var_def(Node *node) {
    char *name = node->name;
    assert(name);
    Node *reg_node;
    StorageClass sclass = node->sclass = get_storage_class(node->tp);

    if (cur_structdef!=NULL) {      //構造体内であればメンバ
        if (map_get(cur_structdef->map, node->name, (void**)&reg_node)==0) {
            map_put(cur_structdef->map, name, node);
        } else {
            error_with_note(node, reg_node, "メンバの重複定義です", "以前の定義はここです");
        }
        return;
    } else if (cur_funcdef!=NULL) { //関数内であればローカル
        Map *symbol_map = stack_top(symbol_stack); 
        map_get(symbol_map, name, (void**)&reg_node);
        Node *reg_node2 = search_symbol(name);
        if (sclass==SC_EXTERN || node->tp->type==FUNC) {
            if (reg_node2 && !type_eq_ex_sclass(reg_node2->tp, node->tp)) {
                error_with_note(node, reg_node2, "ローカル変数の重複定義です", "以前の定義はここです");
            }
            map_put(symbol_map, name, node);
        } else if (reg_node==NULL) {
            switch (sclass) {
            case SC_STATIC:
                node->index = global_index++;
                vec_push(static_var_vec, node);
                break;
            case SC_EXTERN:
            case SC_TYPEDEF:
                break;
            default:
                node->offset = get_var_offset(node->tp);    //rbp-offset
            }
            if (node->type==ND_UNDEF) node->type = ND_LOCAL_VAR_DEF;
            map_put(symbol_map, name, node);
        } else {
            error_with_note(node, reg_node, "ローカル変数の重複定義です", "以前の定義はここです");
        }
        if (node->type==ND_UNDEF) node->type = ND_LOCAL_VAR_DEF;
    }

    if (cur_funcdef==NULL ||        //関数外であればグローバル
        node_is_extern(node)) {     //externであればグローバル（関数内であればローカルにも登録）
        if (node->type==ND_UNDEF) node->type = ND_GLOBAL_VAR_DEF;
        if (map_get(global_symbol_map, name, (void**)&reg_node)==0) {
            map_put(global_symbol_map, name, node);
        } else if (node->type!=ND_LOCAL_VAR_DEF && !node_type_eq_global(reg_node, node)) {
            error_with_note(node, reg_node, "型が一致しません", "以前の定義はここです");
        } else {
            int has_init_value1 = (reg_node->rhs!=NULL);
            int has_init_value2 = (node->rhs!=NULL);
            if (has_init_value1 && has_init_value2) {
                error_with_note(node, reg_node, "初期値付きグローバル変数の重複定義です", "以前の定義はここです");
            } else if (has_init_value2) {
                reg_node->unused = 1;
                map_put(global_symbol_map, name, node);
            }
        }
    }

    if (node_is_var_def(node) && node->tp->type==VOID) {
        error_at(&cur_token_info(), "不正なvoid指定です"); 
    }
}

//関数の定義・宣言をチェックして登録する
//full_checkが0の場合は、定義か宣言かが未確定の時点でのチェックだけを行う
void regist_func(Node *node, int full_check) {
    assert(node->type==ND_FUNC_DEF || node->type==ND_FUNC_DECL);
    Node *def_node = search_symbol(node->name);
    if (def_node==NULL)  {
        map_put(global_symbol_map, node->name, node);
    } else if (def_node==node) {
        return;
    } else if (def_node->type==ND_GLOBAL_VAR_DEF) {
        error_at(&node_info(node), "'%s'は異なる種類のシンボルとして再定義されています", node->name);
    } else if (!type_eq_ex_sclass(def_node->tp, node->tp)) {
        error_with_note(node, def_node, "関数の型が一致しません", "以前の定義・宣言はここです");
    } else if (!full_check) {
        return;
    } else if (def_node->type==ND_FUNC_DEF && node->type==ND_FUNC_DEF) {
        error_with_note(node, def_node, "関数が再定義されています", "以前の定義はここです");
    } else if (def_node->type==ND_FUNC_DECL && node->type==ND_FUNC_DEF) {
        map_put(global_symbol_map, node->name, node);
    }
}

//enum値を登録
void regist_symbol(Node *node) {
    char *name = node->name;
    Node *node2;
    Map *symbol_map = stack_top(symbol_stack); 
    if (map_get(symbol_map, name, (void**)&node2)==0) {
        map_put(symbol_map, name, node);
    } else {
        error_with_note(node, node2, "enum値の重複定義です", "以前の定義はここです");
    }
}

//タグ名を登録(enum/struct/union)
//nodeが不完全定義(node-lst==NULL)であり、すでに同一タグで登録済みの場合は先行登録済みのノードを返す
//そうでない場合はnodeを返す
Node *regist_tagname(Node *node) {
    char *name = node->name;
    Node *ret_node = node;
    Node *prev_node = search_tagname(node->name);
    Node *node3;
    Map *tagname_map = stack_top(tagname_stack); 
    if (prev_node==NULL) {
        map_put(tagname_map, name, node);
    } else if ((prev_node->type==ND_ENUM_DEF   && node->type==ND_ENUM_DEF) ||
               (prev_node->type==ND_STRUCT_DEF && node->type==ND_STRUCT_DEF) ||
               (prev_node->type==ND_UNION_DEF  && node->type==ND_UNION_DEF)) {
        if (prev_node->lst!=NULL && node->lst!=NULL) {          //共に完全型
            if (map_get(tagname_map, name, (void**)&node3)) {
                //同一スコープ内での再定義はできない
                SET_ERROR_WITH_NOTE;
                error_at(&node_info(node), "%sの重複定義です", node->type==ND_ENUM_DEF?"enum":"struct/union");
                note_at(&node_info(node3), "以前の定義はここです");
            } 
            map_put(tagname_map, name, node);
        } else if (prev_node->lst==NULL && node->lst!=NULL) {   //先行が不完全、新規が完全
            map_put(tagname_map, name, node);
            prev_node->tp = node->tp;
        } else if (prev_node->lst!=NULL && node->lst==NULL) {   //先行が完全、新規が不完全
            ret_node = prev_node;   //不完全型を完全型に置き換えて返す
        } else {                                                //共に不完全
            ret_node = prev_node;   //不完全型を正規化
        }
    } else if (map_get(tagname_map, name, (void**)&node3)) {
        //同一スコープ内での再定義はできない
        error_with_note(node, node3, "タグの重複定義です", "以前の定義はここです");
    } else {
        //異なるスコープであれば問題ない
        map_put(tagname_map, name, node);
    }
    return ret_node;
}

//ラベルを登録
//ラベル定義より前にgotoが先にあった場合、gotoを登録する。その後ラベル定義があればそれで置き換える。
//ラベル定義のないgotoはcodegen時にチェックする。
void regist_label(Node *node) {
    Node *node2;
    if (map_get(cur_funcdef->label_map, node->name, (void**)&node2)!=0) {
        if (node->type==ND_GOTO) return;
        if (node2->type==ND_LABEL) error_at(&node_info(node), "ラベルが重複しています");
    }
    map_put(cur_funcdef->label_map, node->name, node);
}

//重複をチェックしてcase,defaultをcur_switch->mapに登録する。
void regist_case(Node *node) {
    if (cur_switch==NULL) error_at(&node_info(node), "switch文の中ではありません");
    char *name;
    if (node->type==ND_DEFAULT) {
        name = "Default";
    } else {
        char buf[64];
        if (node->val>=0) sprintf(buf, "Case%ld", node->val);
        else              sprintf(buf, "CaseM%ld", -node->val);
        name = malloc(strlen(buf)+1);
        strcpy(name, buf);
    }
    node->name = name;
    if (map_get(cur_switch->map, name, NULL)!=0)
        error_at(&node_info(node), "%sは重複しています", name);
    map_put(cur_switch->map, name, node);
}

//storage classを外したTypeの複製を返す。
Type *get_typeof(Type *tp) {
    Type *ret = malloc(sizeof(Type));
    memcpy(ret, tp, sizeof(Type));
    ret->tmp_sclass = SC_UNDEF;
    if (tp->ptr_of) ret->ptr_of = get_typeof(tp->ptr_of);
    return ret;
}

//戻り値の妥当性をチェック
void check_return(Node *node) {
    assert(node->type==ND_RETURN);
    assert(cur_funcdef);
    assert(cur_funcdef->tp->type==FUNC);
    Type *func_tp = cur_funcdef->tp->ptr_of;
    Type *ret_tp = node->tp;
    Status sts;
    if (func_tp->type==VOID) {
        if (ret_tp==NULL) return;
        if (ret_tp->type==VOID) return;
        warning_at(&node_info(node), "void型関数が値を返しています");
    } else if (ret_tp==NULL || ret_tp->type==VOID) {
        warning_at(&node_info(node), "非void関数%sが値を返していません", cur_funcdef->func_name);
    } else if (!(func_tp->type==PTR && node->rhs->type==ND_NUM && node->rhs->val==0) &&   //右辺が0の場合は無条件にOK
        (sts=type_eq_check(func_tp, ret_tp))!=ST_OK) {
        if (sts==ST_WARN)
            warning_at(&node_info(node), "%s型の関数%sが%s型を返しています",
                get_type_str(func_tp), cur_funcdef->func_name, get_node_type_str(node));
        if (sts==ST_ERR)
            error_at(&node_info(node), "%s型の関数%sが%s型を返しています",
                get_type_str(func_tp), cur_funcdef->func_name, get_node_type_str(node));
    } else if (func_tp->type==PTR && !func_tp->ptr_of->is_const 
            && (ret_tp->type==PTR || ret_tp->type==ARRAY) && ret_tp->ptr_of->is_const) {
        warning_at(&node_info(node), "戻り値%sのconst情報は失われます", get_node_type_str(node));
    }
}

//関数が戻り値を返しているかをチェック（簡易版）
void check_func_return(Funcdef *funcdef) {
    Type *func_tp = cur_funcdef->tp->ptr_of;
    if (func_tp->type==VOID) return;    //void関数が値を返す場合はreturn側でチェックする
    Node **nodes = (Node**)funcdef->node->rhs->lst->data;
    int len = funcdef->node->rhs->lst->len;
    if (len && nodes[len-1]->type==ND_FUNC_END) len--;  //関数の最後にはND_FUNC_ENDがあるので除外
    if (len==0 || nodes[len-1]->type!=ND_RETURN) {
        SrcInfo info = cur_token_info();
        info.input -= 2;
        warning_at(&info, "関数が戻り値を返していません");
    }

}

//関数定義・宣言の引数リスト(ND_LIST)の妥当性を確認
//関数定義(def_mode=1)の場合は名前付き宣言(declaration)であることを確認
void check_funcargs(Node *node, int def_mode) {
    int size = lst_len(node->lst);
    Node **arg_nodes = (Node**)node->lst->data;
    for (int i=0; i < size; i++) {
        Node *arg = arg_nodes[i];
        if (arg->type==ND_VARARGS) continue;
        if (arg->tp->type==VOID) {
            if (i>0) error_at(&node_info(arg), "ここではvoidを指定できません");
            continue;
        }
        if (def_mode && arg->name==NULL)
            error_at(&node_info(arg), "関数定義の引数には名前が必要です");
    }
}

//関数コールの仮引数リストを返す
Vector *get_func_args(Node *node) {
    assert(node->type==ND_FUNC_CALL);
    if (node->rhs) {    //ND_FUNC_DEF|DECL/ND_LOCAL|GLOBAL_VAR_DEF(FUNC)
        if (node->rhs->lhs) {
            return node->rhs->lhs->lst;
        } else {
            Node *var_def;
            Type *func_tp = node->rhs->tp;
            if (func_tp->type==PTR) func_tp = func_tp->ptr_of;
            assert(func_tp->type==FUNC);
            var_def = func_tp->node;
            return var_def->lhs->lst;
        }
    }
    //dump_node(node,__func__);
    //abort();
    return NULL;
}

//関数コールの妥当性を確認
void check_funccall(Node *node) {
    assert(node->type==ND_FUNC_CALL);
    int decl_size = 0;
    int call_size = 0;
    Node **decl_args = NULL;
    Node **call_args = NULL;
    if (node->lhs) {    //引数リスト(ND_LIST)
        call_size = lst_len(node->lhs->lst);
        call_args = (Node**)node->lhs->lst->data;
    }
    Vector *decl_list = get_func_args(node);
    if (decl_list) {
        decl_size = lst_len(decl_list);
        decl_args = (Node**)decl_list->data;
    }

    if (call_size==0) { //
        if (decl_size==0 || (decl_size==1 && decl_args[0]->tp->type==VOID)) return;
        error_at(&node_info(node), "引数の数が足りません");
    } else {
        for (int i=0;i<decl_size && i<call_size;i++) {
            if (decl_args[i]->type==ND_VARARGS) return; //唯一引数の数が一致しないケース
            Node *decl_arg = decl_args[i];
            Node *call_arg = call_args[i];
            Type *decl_tp = decl_arg->tp;
            Type *call_tp = call_arg->tp;
            switch (type_eq_check(decl_tp, call_tp)) {
            case ST_ERR:
                SET_ERROR_WITH_NOTE;
                error_at(&node_info(call_arg), "引数[%d]の型(%s:%s)が一致しません", i,
                    get_node_type_str(decl_arg),
                    get_node_type_str(call_arg));
                note_at(&node_info(decl_arg), "関数の定義はここです");
                break;
            case ST_WARN:
                warning_at(&node_info(call_arg), "引数[%d]の型(%s:%s)が一致しません", i,
                    get_node_type_str(decl_arg),
                    get_node_type_str(call_arg));
                note_at(&node_info(decl_arg), "関数の定義はここです");
                break;
            default:
                break;
            }
            if (decl_tp->type==PTR && !decl_tp->ptr_of->is_const &&
                (call_tp->type==PTR || call_tp->type==ARRAY) && call_tp->ptr_of->is_const) {
                warning_at(&node_info(call_arg), "戻り値%sのconst情報は失われます", get_node_type_str(call_arg));
            }
        }
        if (decl_size && decl_args[decl_size-1]->tp->type==VARARGS) decl_size--;
        if (decl_size>call_size)              error_with_note(call_args[call_size-1], decl_args[call_size], "引数の数が少なすぎます", "関数の定義・宣言はここです");
        if (decl_size && decl_size<call_size) error_with_note(call_args[decl_size], decl_args[decl_size-1], "引数の数が多すぎます", "関数の定義・宣言はここです");
    }
}

static int func_arg_eq(Node *node1, Node *node2) {
    if (node1==NULL || node2==NULL) return 1;
    assert(node1->lhs->type==ND_LIST);
    assert(node2->lhs->type==ND_LIST);
    //引数が空の関数宣言は引数をチェックしない
    //TODO: int func();とint func(char*fmt,...);はerrorかwarningにすべき
    if ((node1->type==ND_FUNC_DECL && vec_len(node1->lhs->lst)==0) ||
        (node2->type==ND_FUNC_DECL && vec_len(node2->lhs->lst)==0)) return 1;
    if (vec_len(node1->lhs->lst) != vec_len(node2->lhs->lst)) return 0;
    for (int i=0;i<vec_len(node1->lhs->lst);i++) {
        Node *arg1 = (Node*)vec_data(node1->lhs->lst, i);
        Node *arg2 = (Node*)vec_data(node2->lhs->lst, i);
        if (!type_eq(arg1->tp, arg2->tp)) return 0;
    }
    return 1;
}

//型が等しいかどうかを判定する
int type_eq(const Type *tp1, const Type *tp2) {
    if (tp1==tp2) return 1;
    if (tp1->type != tp2->type) return 0;
    if (tp1->is_unsigned != tp2->is_unsigned) return 0;
    if (tp1->is_const != tp2->is_const) return 0;
    if (tp1->tmp_sclass != tp2->tmp_sclass) return 0;
    if (tp1->array_size != tp2->array_size) return 0;
    if (tp1->type==FUNC && !func_arg_eq(tp1->node, tp2->node)) return 0;
    if (type_is_struct_or_union(tp1) && tp1->node!=tp2->node) return 0;
    if (tp1->ptr_of) return type_eq(tp1->ptr_of, tp2->ptr_of);
    return 1;
}

//global変数の型が等しいかどうかを判定する（externは無視する）
int type_eq_global(const Type *tp1, const Type *tp2) {
    if (tp1==tp2) return 1;
    if (tp1->type != tp2->type) return 0;
    if (tp1->is_unsigned != tp2->is_unsigned) return 0;
    if (tp1->is_const != tp2->is_const) return 0;
    if (tp1->tmp_sclass != tp2->tmp_sclass) {
        if (!(tp1->tmp_sclass == SC_EXTERN && tp2->tmp_sclass == SC_UNDEF ) &&
            !(tp1->tmp_sclass == SC_UNDEF  && tp2->tmp_sclass == SC_EXTERN)) return 0;
    }
    if (tp1->array_size != tp2->array_size) return 0;
    if (tp1->type==FUNC && !func_arg_eq(tp1->node, tp2->node)) return 0;
    if (type_is_struct_or_union(tp1) && tp1->node!=tp2->node) return 0;
    if (tp1->ptr_of) return type_eq_global(tp1->ptr_of, tp2->ptr_of);
    return 1;
}
int node_type_eq_global(const Node *node1, const Node *node2) {
    StorageClass sclass1 = node1->sclass, sclass2 = node2->sclass;
    if (sclass1 != sclass2) {
        if (!(sclass1 == SC_EXTERN && sclass2 == SC_UNDEF ) &&
            !(sclass1 == SC_UNDEF  && sclass2 == SC_EXTERN)) return 0;
    }
    return type_eq_global(node1->tp, node2->tp);
}

//  関数の戻り値の型が等しいかどうかを判定する（storage classは無視）
int type_eq_ex_sclass(const Type *tp1, const Type *tp2) {
    if (tp1==tp2) return 1;
    if (tp1->type != tp2->type) return 0;
    if (tp1->is_unsigned != tp2->is_unsigned) return 0;
    if (tp1->is_const != tp2->is_const) return 0;
//  if (tp1->sclass != tp2->sclass) return 0;
    if (tp1->array_size != tp2->array_size) return 0;
    if (tp1->type==FUNC && !func_arg_eq(tp1->node, tp2->node)) return 0;
    if (type_is_struct_or_union(tp1) && tp1->node!=tp2->node) return 0;
    if (tp1->ptr_of) return type_eq_ex_sclass(tp1->ptr_of, tp2->ptr_of);
    return 1;
}

//node1にnode2を渡す観点で型の一致を判定する
Status type_eq_check(const Type *tp1, const Type *tp2) {
    if (tp1==tp2) return ST_OK;
    //if (!tp1->is_const && tp2->is_const) {
    //    return ST_WARN; //引数、戻り値の場合はWARN。代入の場合はERRだがここではチェックしない
    //} else
    if (type_is_integer(tp1) && type_is_integer(tp2)) {
        ;   //inter同士はOK
    } else if (tp1->type==PTR && tp2->type==ARRAY) {
        ;   //ポインタに対する配列はOK
        if (tp1->ptr_of->type==VOID) return ST_OK;
    } else if (tp1->type==PTR && tp1->ptr_of->type==FUNC &&
               tp2->type==FUNC) {   //関数ポインタに対する関数名の代入
        tp1 = tp1->ptr_of;
    } else if (tp1->type==PTR && tp2->type==PTR &&
             (tp1->ptr_of->type==VOID || tp2->ptr_of->type==VOID)) {
            //voidポインタと非voidポインタの相互代入はOK
        return ST_OK;
    } else if (type_is_struct_or_union(tp1) && type_is_struct_or_union(tp2)) {
        //tp->node->tpまでたどることで不完全型が完全型になる
        if (tp1->node->tp != tp2->node->tp) return ST_ERR;
    } else if (type_is_struct_or_union(tp1) || type_is_struct_or_union(tp2)) {
        if (tp1->node != tp2->node) return ST_ERR;
    } else if (tp1->type != tp2->type) {
        if (tp1->type==VOID) return ST_ERR;
        return ST_WARN;
    }
    if (tp1->ptr_of) {
        //ポインタの種類が違う場合は警告とする
        return type_eq_check(tp1->ptr_of, tp2->ptr_of)==ST_OK ? ST_OK : ST_WARN;
    }
    return ST_OK;
}

//関数定義のroot生成
Funcdef *new_funcdef(void) {
    Funcdef * funcdef;
    funcdef = calloc(1, sizeof(Funcdef));
    funcdef->symbol_map  = new_map();
    funcdef->tagname_map = new_map();
    funcdef->label_map   = new_map();
    return funcdef;
}

//型情報の生成
Type *new_type_ptr(Type*ptr) {
    Type *tp = calloc(1, sizeof(Type));
    tp->type = PTR;
    tp->ptr_of = ptr;
    return tp;
}

Type *new_type_func(Type*ptr, Node *node) {
    Type *tp = calloc(1, sizeof(Type));
    tp->type = FUNC;
    tp->ptr_of = ptr;
    tp->node = node;
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

Type *dup_type(const Type *tp) {
    Type *new_tp = malloc(sizeof(Type));
    *new_tp = *tp;
    if (tp->ptr_of) new_tp->ptr_of = dup_type(tp->ptr_of);
    return new_tp;
}

Node *dup_node(const Node *node) {
    Node *new_node = malloc(sizeof(Node));
    *new_node = *node;
    return new_node;
}

//抽象構文木の生成（演算子）
Node *new_node(int type, Node *lhs, Node *rhs, Type *tp, Token *token) {
    Node *node = calloc(1, sizeof(Node));
    node->type = type;
    node->lhs  = lhs;
    node->rhs  = rhs;
    node->tp   = tp;
    node->token = token;
    return node;
}

//抽象構文木の生成（数値）
Node *new_node_num(long val, Token *token) {
    NDtype type = (val>UINT_MAX || val<INT_MIN) ? LONG : INT;
    Node *node = new_node(ND_NUM, NULL, NULL, new_type(type, 0), token);
    node->val = val;
//    fprintf(stderr, "val=%ld\n", val);
    return node;
}

//抽象構文木の生成（変数定義）
Node *new_node_var_def(char *name, Type*tp, Token *token) {
    Node *node = new_node(ND_UNDEF, NULL, NULL, tp, token);
    node->name = name;
    if (name) regist_var_def(node);
    return node;
}

//抽象構文木の生成（文字列リテラル）
Node *new_node_string(String *string, Token *token) {
    Type *tp = new_type_array(new_type(CHAR, 0), string->size);
    Node *node = new_node(ND_STRING, NULL, NULL, tp, token);
    node->string = *string;
    node->index = new_string_literal(string); //インデックス

    return node;
}

//抽象構文木の生成（識別子：ローカル変数・グローバル変数）
Node *new_node_ident(char *name, Token *token) {
    Node *node, *var_def;
    NDtype type;
    StorageClass sclass = SC_UNDEF;
    Type *tp;
    long offset = 0;
    int index = 0;

    //定義済みの変数であるかをスコープの内側からチェック
    var_def = search_symbol(name);
    if (var_def) {
        switch (var_def->type) {
        case ND_LOCAL_VAR_DEF:
            if (node_is_extern(var_def)) type = ND_GLOBAL_VAR;
            else                         type = ND_LOCAL_VAR;
            break;
        case ND_GLOBAL_VAR_DEF:
        case ND_FUNC_DEF:       
        case ND_FUNC_DECL:
            type = ND_GLOBAL_VAR;
            break;
        case ND_ENUM:
            return var_def;
        default:
            dump_node(var_def,__func__);
            assert(0);
        }
        sclass = var_def->sclass;
        tp = var_def->tp;
        offset = var_def->offset;
        index  = var_def->index;
    } else {
        type = ND_IDENT;
        tp = NULL;
    }

    node = new_node(type, NULL, NULL, tp, token);
    node->name   = name;
    node->sclass = sclass;
    node->offset = offset;
    node->index  = index;
    //dump_node(node, __func__);

    return node;
}

//抽象構文木の生成（関数コール）
Node *new_node_func_call(char *name, Token *token) {
    Node *node, *func_node;
    Type *tp;
    func_node = search_symbol(name);
    if (func_node) {
        switch (func_node->type) {
        case ND_LOCAL_VAR_DEF:  //関数ポインタ
        case ND_GLOBAL_VAR_DEF: //関数ポインタ
            if (func_node->tp->type!=PTR || func_node->tp->ptr_of->type!=FUNC)
                error_at(&token->info, "%sは関数ではありません", name);
            node = new_node(func_node->type==ND_LOCAL_VAR_DEF?ND_LOCAL_VAR:ND_GLOBAL_VAR,
                            NULL, NULL, func_node->tp, func_node->token);
            node->name = name;
            node->offset = func_node->offset;
            func_node = node;
            // PTR->FUNC->int
            //            ^---ここを指す
            tp = func_node->tp->ptr_of->ptr_of;
            break;
        case ND_FUNC_DEF:       
        case ND_FUNC_DECL:
            assert(func_node->tp->type==FUNC);
            tp = func_node->tp->ptr_of;
            break;
        default:
            error_at(&token->info, "不正な関数コールです。");
        }
    } else {
        warning_at(&token->info, "未宣言の関数コールです。");
        tp = new_type(INT, 0);
    }
    node = new_node(ND_FUNC_CALL, NULL, func_node, tp, token);
    node->name = name;
    if (func_node) node->sclass = func_node->sclass;
//  node->lhs   //引数リスト
//  node->rhs   //コール先を示すノード

    return node;
}

//抽象構文木の生成（関数定義・宣言）
//この時点では宣言(ND_FUNC_DECL)として作成し、あとで定義であることが確定したら定義(ND_FUNC_DEF)に変更する
Node *new_node_func(char *name, Type *tp, Token *token) {
    Node *node = new_node(ND_FUNC_DECL, NULL, NULL, tp, token);
    node->name = name;
    node->sclass = get_storage_class(tp);
//  node->lhs       //引数リスト(ND_LIST)
//  node->rhs       //ブロック(ND_BLOCK)

    cur_funcdef = new_funcdef();
    cur_funcdef->tp = tp;
    cur_funcdef->node = node;
    cur_funcdef->func_name = node->name;

    //関数をシンボルとして仮登録する
    regist_func(node, 0);

    return node;
}

//抽象構文木の生成（空文）
Node *new_node_empty(Token *token) {
    Node *node = new_node(ND_EMPTY, NULL, NULL, NULL, token);
    return node;
}

//抽象構文木の生成（ブロック）
Node *new_node_block(Token *token) {
    Node *node = new_node(ND_BLOCK, NULL, NULL, NULL, token);
    node->lst  = new_vector();
    return node;
}

//抽象構文木の生成（コンマリスト）
//型は最後の要素の型であるべきだがここでは設定しない
Node *new_node_list(Node *item, Token *token) {
    Node *node = new_node(ND_LIST, NULL, NULL, NULL, token);
    node->lst  = new_vector();
    if (item) vec_push(node->lst, item);
    return node;
}

//抽象構文木の生成（初期化リスト）
Node *new_node_init_list(Node *item, Token *token) {
    Node *node = new_node(ND_INIT_LIST, NULL, NULL, NULL, token);
    node->lst  = new_vector();
    if (item) vec_push(node->lst, item);
    return node;
}
