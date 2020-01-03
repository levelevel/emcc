#define _PARSE_C_

#include "emcc.h"

/*  文法：
- A.2.4 External Definitions  http://port70.net/~nsz/c/c11/n1570.html#A.2.4
    translation_unit        = external_declaration*
    external_declaration    = function_definition | declaration
    function_definition     = declaration_specifiers declarator compound_statement
- A.2.2 Declarations          http://port70.net/~nsz/c/c11/n1570.html#A.2.2
    declaration             = declaration_specifiers ( init_declarator ( "," init_declarator )* )? ";"
                            | static_assert_declaration
    declaration_specifiers  = "typeof" "(" identifier ")"
                            | storage_class_specifier declaration_specifiers*
                            | type_specifier          declaration_specifiers*
                            | type_qualifier          declaration_specifiers*
                            | function_specifier      declaration_specifiers*
                            | alignment_specifier     declaration_specifiers* (未実装)
    init_declarator         = declarator ( "=" initializer )?
    storage_class_specifier = "typedef" | "static" | "extern" | "auto" | "register"
    type_specifier          = "void" | "_Bool" | "char" | "short" | "int" | "long" | "signed" | "unsigned"
                            | struct_or_union_specifier | enum_specifier | typedef_name
    struct_or_union_specifier = ( "struct" | "union" ) identifier? "{" struct_declaration+ "}"
                            | ( "struct" | "union" ) identifier
    struct_declaration      = specifier_qualifier_list struct_declarator_list* ";"
                            | static_assert_declaration
    specifier_qualifier_list = type_specifier specifier_qualifier_list*
                            | type_qualifier specifier_qualifier_list*
    struct_declarator_list  = struct_declarator ( "," struct_declarator )*
    struct_declarator       = declarator
                            | declarator? ":" constant_expression
    enum_specifier          = "enum" identifier? "{" enumerator ( "," enumerator )* ","? "}"
                            | "enum" identifier
    enumerator              = enumeration_constant ( "=" constant-expression )?
    type_qualifier          = "const" | "restrict" | "volatile" | "_Atomic"
    function_specifier      = "inline" | "_Noreturn"
    declarator              = pointer? direct_declarator
    direct_declarator       = identifier 
                            | "(" declarator ")"
                            | direct_declarator "[" assignment_expression? "]"
                            | direct_declarator "(" parameter_type_list? ")"    //関数
    parameter_type_list     = parameter_declaration ( "," parameter_declaration )* ( "," "..." )?
    parameter_declaration   = declaration_specifiers ( declarator | abstract_declarator )?
    initializer             = assignment_expression
                            | "{" init_list "}"
                            | "{" init_list "," "}"
    init_list               = initializer
                            | init_list "," initializer
    static_assert_declaration = "_Static_assert" "(" constant_expression "," string_literal ")" ";"
    //配列のサイズは定数の場合のみサポートする
    array_def               = "[" constant_expression? "]" ( "[" constant_expression "]" )*
    pointer                 = ( "*" type_qualifier* )*
    type_name               = "typeof" "(" identifier ")"
                            | specifier_qualifier_list abstract_declarator*
    abstract_declarator     = pointer? direct_abstract_declarator
    direct_abstract_declarator = "(" abstract_declarator ")"
                            | direct_abstract_declarator? "[" assignment_expression? "]"
                            | direct_abstract_declarator? "(" parameter_type_list? ")"  //関数
- A.2.3 Statements            http://port70.net/~nsz/c/c11/n1570.html#A.2.3
    statement               = labeled_statement
                            | compound_statement    // { ... }
                            | expression? ";"       // expression_statement
                            | selection_statement
                            | iteration_statement
                            | jump_statement
        selection_statement = "if" "(" expression ")" ( "else" expression )? statement
                            | "switch" "(" expression ")" statement
        iteration_statement = "while" "(" expression ")" statement
                            | "do" statement "while" "(" expression ")" ";"
                            | "for" "(" expression? ";" expression? ";" expression? ")" statement
                            | "for" "(" declaration     expression? ";" expression? ")" statement
        labeled_statement   = identifier ":" statement
                            | "case" constant_statement ":" statement
                            | "default" ":" statement
        compound_statement  = "{" declaration | statement* "}"
        jump_statement      = "goto" identifier ";"
                            | "continue" ";"
                            | "break" ";"
                            | "return" expression? ";"
- A.2.1 Expressions
    expression              = assignment_expression ( "," assignment_expression )*
    constant_expression     = conditional_expression
    assignment_expression   = conditional_expression
                            | unary_expression ( "=" | "+=" | "-=" | "*=" | "/=" | "%=" | 
                                ">>=" | "<<=" | "&=" | "^=" | "|=" ) assignment_expression
    conditional_expression  = logical_OR_expression ( "?" expression ":" conditional_expression )?
    logical_OR_expression   = logical_AND_expression ( "||" logical_AND_expression )*
    logical_AND_expression  = inclusive_OR_expression ( "&&" inclusive_OR_expression )*
    inclusive_OR_expression = exclusive_OR_expression ( "&" exclusive_OR_expression )*
    exclusive_OR_expression = AND_expression ( "^" AND_expression )*
    AND_expression          = equality_expression ( "&" equality_expression )*
    equality_expression     = relational_expression ( ( "==" | "!=" ) relational_expression )*
    relational_expression   = shift_expression ( ( "<" | "<=" | ">" | ">=" ) shift_expression )*
    shift_expression        = additive_expression ( ( "<<" | ">>" ) additive_expression )*
    additive_expression     = multiplicative_expression ( ( "+" | "-" ) multiplicative_expression )*
    multiplicative_expression = cast_expression ( ( "*" | "/" | "%" ) cast_expression )*
    cast_expression         = unary_expression
                            | "(" type_name ")" cast_expression
    unary_expression        = postfix_expression
                            | ( "+" | "-" |  "!" | "~" | "*" | "&" ) cast_expression
                            | ( "++" | "--" )? unary_expression
                            | "sizeof" unary_expression
                            | "sizeof" "(" type_name ")"
                            | "_Alignof" unary_expression
                            | "_Alignof" "(" type_name ")"
    postfix_expression      = primary_expression 
                            | primary_expression "[" expression "]"
                            | primary_expression "(" assignment_expression? ( "," assignment_expression )* ")"
                            | primary_expression "." identifier
                            | primary_expression "->" identifier
                            | primary_expression "++"
                            | primary_expression "--"
    primary_expression      = num
                            | string_literal
                            | identifier
                            |  "(" expression ")"
*/
static void external_declaration(void);
static Node *function_definition(Type *tp, char *name);
static Node *declaration(Type *tp, char *name);
static Node *init_declarator(Type *decl_spec, Type *tp, char *name);
static Node *declarator(Type *decl_spec, Type *tp, char *name);
static Node *direct_declarator(Type *tp, char *name);
static Node *parameter_type_list(void);
static Node *parameter_declaration(void);
static Node *initializer(void);
static Node *init_list(void);
static Node* static_assert_declaration(void);
static Type *array_def(Type *tp);
static Type *pointer(Type *tp);
static Type *type_name(void);
static Type *abstract_declarator(Type *tp);
static Type *direct_abstract_declarator(Type *tp);
static Type *declaration_specifiers(int type_only);
static Node *struct_or_union_specifier(TPType type);
static Type *specifier_qualifier_list(void);
static Node *struct_declaration(void);
static Node *struct_declarator_list(Type *tp);
static Node *struct_declarator(Type *tp);
static void refresh_typedef(Node *struc);
static Node *enum_specifier(void);
static Node *enumerator(Node *enum_def, int default_val);

static Node *statement(void);
static Node *compound_statement(int is_func_body);
static Node *expression(void);
static Node *constant_expression(void);
static Node *assignment_expression(void);
static Node *conditional_expression(void);
static Node *equality_expression(void);
static Node *logical_OR_expression(void);
static Node *logical_AND_expression(void);
static Node *inclusive_OR_expression(void);
static Node *exclusive_OR_expression(void);
static Node *AND_expression(void);
static Node *relational_expression(void);
static Node *shift_expression(void);
static Node *additive_expression(void);
static Node *multiplicative_expression(void);
static Node *cast_expression(void);
static Node *unary_expression(void);
static Node *postfix_expression(void);
static Node *primary_expression(void); 

void translation_unit(void) {
    while (!token_is(TK_EOF)) {
        external_declaration();
        cur_funcdef = NULL;
    }
}

//関数のスコープの開始
static void begin_func_scope(void) {
    stack_push(symbol_stack,  cur_funcdef->symbol_map);
    stack_push(tagname_stack, cur_funcdef->tagname_map);
}
//ローカルスコープの開始
static void begin_local_scope(void) {
    stack_push(symbol_stack,  new_map());
    stack_push(tagname_stack, new_map());
}
static void end_scope(void) {
    stack_pop(symbol_stack);
    stack_pop(tagname_stack);
}

enum {
    CHK_INTEGER = 0x0001,   //整数の場合にエラーとする
    CHK_STRUCT  = 0x0002,   //構造体の場合にエラーとする
    CHK_UNION   = 0x0004,   //共用体の場合にエラーとする
    CHK_ARRAY   = 0x0008,   //配列の場合にエラーとする
    CHK_PTR     = 0x0010,   //ポインタの場合にエラーとする
    CHK_ALL     = 0x0FFF,   //上記すべて
    CHK_BINARY  = 0x1000,   //2項演算子のlhs/rhsに対してチェックする。このビットが0の場合はそのnode自身をチェックする
    CHK_CONST   = 0x2000,   //constの場合にエラーとする
};
#define CHK_NOT(_val) (CHK_ALL ^ (_val))
#define type_is_array(_t) ((_t)->type==ARRAY && (_t)->array_size>=0)
static void check_arg(Node *node, const SrcInfo *info, unsigned int check_mode, const char *msg) {
    if (check_mode & CHK_BINARY) {
        check_arg(node->lhs, info, check_mode ^ CHK_BINARY, msg);
        check_arg(node->rhs, info, check_mode ^ CHK_BINARY, msg);
        return;
    }
    Type *tp = node->tp;
    char *tp_str = get_type_str(tp);
    if (info==NULL) info = &node_info(node);
    if ((check_mode & CHK_INTEGER) && type_is_integer(tp)) error_at(info, "整数(%s)に対して%sの指定はできません", tp_str, msg);
    if ((check_mode & CHK_STRUCT)  && tp->type==STRUCT) error_at(info, "構造体(%s)に対して%sの指定はできません", tp_str, msg);
    if ((check_mode & CHK_UNION)   && tp->type==UNION)  error_at(info, "共用体(%s)に対して%sの指定はできません",  tp_str, msg);
    if ((check_mode & CHK_ARRAY)   && type_is_array(tp))error_at(info, "配列(%s)に対して%sの指定はできません", tp_str, msg);
    if ((check_mode & CHK_PTR)     && tp->type==PTR)    error_at(info, "ポインタ(%s)に対して%sの指定はできません", tp_str, msg);
    if ((check_mode & CHK_CONST)   && tp->is_const)     error_at(info, "読み取り専用変数(%s)に対して%sの実行はできません", tp_str, msg);
}

static void check_scalar(Node *node, const char *msg) {
    if (type_is_struct_or_union(node->tp)) error_at(&node_info(node), "%sにはスカラー値が必要です", msg);
}

//代入：node = rhs に対する型チェックを行う
void check_assignment(const Node *node, const Node *rhs, const SrcInfo *info){
    Status sts;
    if (!(node->tp->type==PTR && rhs->type==ND_NUM && rhs->val==0) &&  //ポインタの右辺が0の場合は無条件にOK
        (sts=type_eq_check(node->tp, rhs->tp))!=ST_OK) {
        if (sts==ST_ERR)
            error_at(info, "=の左右の型(%s:%s)が異なります", 
                get_node_type_str(node), get_node_type_str(rhs));
        if (sts==ST_WARN)
            warning_at(info, "=の左右の型(%s:%s)が異なります", 
                get_node_type_str(node), get_node_type_str(rhs));
    }
}

//    external_declaration    = function_definition | declaration
//    function_definition     = declaration_specifiers declarator compound_statement
//    declaration             = declaration_specifiers init_declarator ( "," init_declarator )* ";"
static void external_declaration(void) {
    Node *node;
    Type *tp;
    char *name;
    SrcInfo *info = &cur_token_info();
    //          <-> declaration_specifiers
    //             <-> pointer* (declarator|init_declarator)
    //                <-> identifier (declarator|init_declarator)
    // 型宣言：  int;
    // 関数定義：int * foo (int a){
    // 関数宣言：int * foo (int a);
    //          int * foo (int);
    // 変数定義：int * ptr ;
    //          int   abc = 1;
    //          int   ary [4];
    //                    ^ここまで読まないと区別がつかない
    // ネスト：　int * ( )
    //                ^ここでdeclarationが確定
    if (token_is_type_spec()) {
        tp = declaration_specifiers(0/*=type_only*/);
        if (consume(';')) {
            if (get_storage_class(tp)>SC_TYPEDEF) warning_at(info, "Storage Classは無視されます");
            if (tp->is_const) warning_at(info, "constは無視されます");
            new_node(ND_TYPE_DECL, NULL, NULL, tp, cur_token());
            return;
        }
        tp = pointer(tp);
        if (token_is('(')) {    //int * ()
            node = declaration(tp, NULL);
        } else if (!consume_ident(&name)) {
            error_at(&cur_token_info(), "型名の後に識別名がありません");
        } else if (token_is('(')) {
            node = function_definition(tp, name);
        } else {
            node = declaration(tp, name);
        }
    } else if (token_is(TK_SASSERT)) {
        node = declaration(NULL, NULL);
    } else if (consume(';')) {
        //仕様書に記載はない？が、空の ; を読み飛ばす。
    } else {
        error_at(&cur_token_info(), "関数・変数の定義がありません");
    }
    if (g_dump_node) dump_node(node, __func__);
}

//関数の定義: lhs=引数(ND_LIST)、rhs=ブロック(ND_BLOCK)
//トップレベルにある関数定義もここで処理する。
//    function_definition     = declaration_specifiers declarator compound_statement
static Node *function_definition(Type *tp, char *name) {
    Node *node;
    Type *decl_spec = tp;
    assert(name!=NULL);
    while (decl_spec->ptr_of) decl_spec = decl_spec->ptr_of;
    node = declarator(decl_spec, tp, name);
    if (token_is('{')) {    //関数定義
        check_funcargs(node->lhs, 1);   //引数リストの妥当性を確認（定義モード）

        begin_func_scope();
        node->rhs = compound_statement(1);
        end_scope();

        map_put(funcdef_map, node->name, cur_funcdef);
        check_func_return(cur_funcdef);        //関数の戻り値を返しているかチェック
        node->type = ND_FUNC_DEF;
    } else {                //関数宣言
        consume(';');
        check_funcargs(node->lhs, 0);   //引数リストの妥当性を確認（宣言モード）
        node->type = ND_FUNC_DECL;
    }
    regist_func(node, 1);

    return node;
}

//    declaration             = declaration_specifiers ( init_declarator ( "," init_declarator )* )? ";"
//                            | static_assert_declaration
//    init_declarator         = declarator ( "=" initializer )?
//    declarator              = pointer? direct_declarator
//declaration_specifiers, pointer, identifierまで先読み済み
static Node *declaration(Type *tp, char *name) {
    Node *node;
    Type *decl_spec;

    if (tp==NULL) {
        if (token_is(TK_SASSERT)) {
            return static_assert_declaration();
        }
        SrcInfo *info = &cur_token_info();
        //declaration_specifiers, pointer, identifierを先読みしていなければdecl_specを読む
        decl_spec = declaration_specifiers(0/*=type_only*/);
        if (consume(';')) {
            if (get_storage_class(decl_spec)>SC_TYPEDEF) warning_at(info, "Storage Classは無視されます");
            if (decl_spec->is_const) warning_at(info, "constは無視されます");
            node = new_node(ND_TYPE_DECL, NULL, NULL, decl_spec, cur_token());
            return node;
        }
    } else {
        // 関数定義：int * foo (){}
        // 変数定義：int * ptr ;
        //          int   abc = 1;
        //          int   ary [4];
        //                   ^ここまで先読み済みであるので、ポインタを含まないdecl_spec（int）を取り出す
        // int * ( *x )
        //       ^ここまで先読み済み
        decl_spec = tp;
        while (decl_spec->ptr_of) decl_spec = decl_spec->ptr_of;
    }

    if (name==NULL && token_is(';')) {
        //変数名なしが許されるのは無名構造体・共用体のみ
        if (!(type_is_struct_or_union(decl_spec) && node_is_noname(decl_spec->node))) {
            expect_ident(NULL, "変数名");
        }
        node = decl_spec->node;
    } else {
        node = init_declarator(decl_spec, tp, name);
        if (type_is_struct_or_union(decl_spec)) {
            node->lst = decl_spec->node->lst;
            node->map = decl_spec->node->map;
        }

        if (consume(',')) {
            Node *last_node;
            node = new_node_list(node, node->token);
            Vector *lists = node->lst;
            vec_push(lists, last_node=init_declarator(decl_spec, NULL, NULL));
            while (consume(',')) {
                vec_push(lists, last_node=init_declarator(decl_spec, NULL, NULL));
            }
            node->tp = last_node->tp;
        }
    }
    set_storage_class(node->tp, SC_UNDEF);

    expect(';');
    return node;
}

//    init_declarator         = declarator ( "=" initializer )?
//    declarator              = pointer? direct_declarator
//declaration_specifiers, pointer, identifierまで先読みの可能性あり
static Node *init_declarator(Type *decl_spec, Type *tp, char *name) {
    Node *node, *rhs=NULL;
    Token *token = cur_token();

    node = declarator(decl_spec, tp, name);
    tp   = node->tp;
    name = node->name;

    if (get_storage_class(tp)==SC_TYPEDEF) {
        node->type = ND_TYPEDEF;
        node->sclass = SC_TYPEDEF;
        regist_var_def(node);
        return node;
    }

    //初期値: rhsに初期値を設定する
    if (consume('=')) {
        if (node_is_extern(node)) error_at(&token->info, "extern変数は初期化できません");
        //node->rhsに変数=初期値の形のノードを設定する。->そのまま初期値設定のコード生成に用いる
        rhs = initializer();
        long val;
        if (tp->type==ARRAY) {  //左辺が配列
            if (rhs->tp->type==ARRAY) {
                //文字列リテラルで初期化できるのはcharの配列だけ
                if (rhs->type==ND_STRING && tp->ptr_of->type!=CHAR)
                    error_at(&node_info(rhs), "%sを文字列リテラルで初期化できません", get_node_type_str(node));
                if (tp->array_size<0) {
                    if (rhs->type==ND_INIT_LIST) {
                        tp->array_size = rhs->lst->len;
                    } else {
                        tp->array_size = rhs->tp->array_size;
                    }
                }
            } else if (rhs->type==ND_INIT_LIST) {
                if (tp->array_size<0) {
                    tp->array_size = rhs->lst->len;
                }
            } else {
                error_at(&node_info(rhs), "配列の初期値が配列形式になっていません");
            }
        }
        if (node_is_constant(rhs, &val)) {
            rhs = new_node_num(val, rhs->token);
        }
        if (rhs->type!=ND_INIT_LIST) check_assignment(node, rhs, &token->info);
        node->rhs = new_node('=', NULL, rhs, tp, token);
    }

    //初期値により配列のサイズが決まるケースがあるので、regist_var_def()は初期値の確定後に行う必要あり
    regist_var_def(node);   //ND_UNDEF -> ND_(LOCAL|GLOBAL)_VAR_DEFに確定する。ND_FUNC_DECLはそのまま
    if (rhs) node->rhs->lhs = new_node_ident(name, token);  //=の左辺
    node->val = size_of(node->tp);

    //初期値のないサイズ未定義のARRAYはエラー
    //externの場合はOK
    if (node->tp->type==ARRAY && node->tp->array_size<0 && !node_is_extern(node) &&
        (node->rhs==NULL || node->rhs->type!='='))
        error_at(&cur_token_info(), "配列のサイズが未定義です");

    //グローバルスカラー変数の初期値は定数または固定アドレスでなければならない
    if ((node->type==ND_GLOBAL_VAR_DEF || node_is_local_static_var(node)) && 
        node->tp->type!=ARRAY && node->rhs &&
        node->rhs->rhs->type!=ND_STRING) {  //文字列リテラルはここではチェックしない
        long val=0;
        Node *var=NULL;
        if (!node_is_constant_or_address(node->rhs->rhs, &val, &var)) {
            error_at(&node_info(node->rhs->rhs), "%s変数の初期値が定数ではありません", 
                     node->type==ND_GLOBAL_VAR_DEF?"グローバル":"静的ローカル");
        }
        if (var) {
            if (val) {
                node->rhs->rhs = new_node('+', var, new_node_num(val, token), var->tp, token);
            } else {
                node->rhs->rhs = var;
            }
        }
    }

    if (node->type==ND_LOCAL_VAR_DEF && node_is_static(node)) {
        char *name = node->name;
        StorageClass sclass = node->sclass;
        //ローカルstatic変数は初期値を外してreturnする。初期化はvarderのリストから実施される。
        node = new_node(ND_LOCAL_VAR_DEF, NULL, NULL, node->tp, node->token);
        node->name = name;
        node->sclass = sclass;
    }
    return node;
}

//    declarator              = pointer* direct_declarator
//    direct_declarator       = identifier 
//                            | "(" declarator ")"
//                            | direct_declarator "[" assignment_expression? "]"
//                            | direct_declarator "(" parameter_type_list? ")"  //関数宣言
//declaration_specifiers, pointer, identifierまで先読み済みの可能性あり
//戻り値のnodeはname、lhs（関数の場合の引数）、tp以外未設定
static Node *declarator(Type *decl_spec, Type *tp, char *name) {
    Node *node;
    if (tp==NULL) {
        tp = pointer(decl_spec);
        name = NULL;
    }
    node = direct_declarator(tp, name);

    return node;
}
static void replace_nest(Type *p, Type *tp);
static Node *direct_declarator(Type *tp, char *name) {
    Node *node=NULL;
    Type *nest_tp = NULL;
    Token *token = cur_token();
    if (name == NULL) {
        if (consume('(')) {
            node = declarator(new_type(NEST, 0), NULL, name);
            nest_tp = node->tp;
            name = node->name;
            expect(')');
        } else {
            expect_ident(&name, "変数名・関数名");
        }
    }
    if (consume('(')) { //関数定義・宣言確定
        Funcdef *org_funcdef = cur_funcdef;
        tp = new_type_func(tp, node);
        if (nest_tp) {  //関数のポインタ定義 int(*fp)();
            cur_funcdef = new_funcdef();    //一時的にcur_funcdefを作成しスコープを生成
            begin_func_scope();
            node->lhs = parameter_type_list();
            end_scope();
            cur_funcdef = org_funcdef;      //一時的なcur_funcdefはここで捨てる
            replace_nest(nest_tp, tp);
            node->tp = tp = nest_tp;
        } else {
            if (cur_funcdef && type_is_static(tp)) {
                error_at(&token->info, "ブロック内のstatic関数");
            }
            node = new_node_func(name, tp, token);  //ここで新たなcur_funcdef設定
            begin_func_scope();
            node->lhs = parameter_type_list();
            end_scope();
            if (org_funcdef) cur_funcdef = org_funcdef;
            tp->node = node;
        }
        expect(')');
        if (node->type==ND_FUNC_DECL) regist_func(node, 1);
    } else {    //変数
        if (token_is('[')) tp = array_def(tp);
        node = new_node(ND_UNDEF, NULL, NULL, tp, NULL);
        node->name = name;
        node->token = token;

        switch (tp->type) {
        case ENUM:
        case STRUCT:
        case UNION:
            if (tp->node->lst==NULL && tp->tmp_sclass!=SC_TYPEDEF && tp->tmp_sclass!=SC_EXTERN) {
                SET_ERROR_WITH_NOTE;
                error_at(&node_info(node), "%sの型は不完全です", name);
                note_at(&node_info(tp->node), "型の宣言はここです");
            }
            break;
        default:
            break;
        }
    }
    return node;
}
static void replace_nest(Type *p, Type *tp) {
    for (;p->type==PTR;) {
        if (p->ptr_of->type==NEST) {
            p->ptr_of = tp;
            break;
        } else if (p->ptr_of->type==PTR) {
            p = p->ptr_of;
        } else {
            assert(0);
        }
    }
}

//    parameter_type_list     = parameter_declaration ( "," parameter_declaration )* ( "," "..." )?
//    parameter_declaration   = declaration_specifiers ( declarator | abstract_declarator )?
static Node *parameter_type_list(void) {
    Node *node = new_node_list(NULL, cur_token());
    Node *last_node;
    SrcInfo *info = NULL;
    if (!token_is_type_spec()) return node; //空のリスト

    vec_push(node->lst, last_node=parameter_declaration());
    if (last_node->tp->type==ARRAY && last_node->tp->array_size<0) last_node->tp->type = PTR;
    while (consume(',')) {
        Token *token = cur_token();
        if (token_is('}')){
            break;
        } else if (consume(TK_3DOTS)) { //...（可変長引数）
            last_node = new_node(ND_VARARGS, NULL, NULL, new_type(VARARGS, 0), token);
            info = &token->info;
        } else {
            last_node = parameter_declaration();
            last_node->token = token;
        }
        vec_push(node->lst, last_node);
        if (last_node->tp->type==ARRAY && last_node->tp->array_size<0) last_node->tp->type = PTR;
    }
    if (info && last_node->type!=ND_VARARGS)
        error_at(info, "...の位置が不正です");
    return node;
}
static int is_abstract_declarator(Type *tp) {
    int save_token_pos = token_pos;
    int ret = 0;
    if (consume('(') && pointer(tp) && consume(')')) ret = 1;
    token_pos = save_token_pos;
    return ret;
}
static Node *parameter_declaration(void) {
    Node *node;
    char *name;
    Token *token = cur_token();
    Type *tp = declaration_specifiers(0/*=type_only*/);
    tp = pointer(tp);
    if (consume_ident(&name)) {
        node = declarator(tp, tp, name);
        regist_var_def(node);
    } else if (token_is('(')) {    //関数のポインタ
        if (is_abstract_declarator(tp)) {
            tp = abstract_declarator(tp);
            node = tp->ptr_of->node;
            assert(node);
        } else {
        node = declarator(tp, tp, NULL);
        regist_var_def(node);
        }
    } else {
        tp = abstract_declarator(tp);
        node = new_node_var_def(NULL, tp, token);
    }
    return node;
}

//    initializer = assignment_expression
//                | "{" init_list "}"
//                | "{" init_list "," "}"
static Node *initializer(void) {
    Node *node;

    if (consume('{')) {
        node = init_list();
        consume(',');
        expect('}');
    } else {
        node = assignment_expression();
    }
    return node;
}

//    init_list   = initializer
//                | init_list "," initializer
static Node *init_list(void) {
    Node *node = new_node_init_list(NULL, cur_token());
    Node *last_node;

    vec_push(node->lst, last_node=initializer());
    while (consume(',') && !token_is('}')) {
        vec_push(node->lst, last_node=initializer());
    }
    node->tp = last_node->tp;

    return node;
}

//    static_assert_declaration = "_Static_assert" "(" constant_expression "," string_literal ")" ";"
static Node* static_assert_declaration(void) {
    Token *token = cur_token();
    expect(TK_SASSERT);
    expect('(');
    Node *node = constant_expression();
    expect(',');
    String string;
    expect_string(&string);
    expect(')');
    expect(';');
    if (node->val==0) error_at(&token->info, "static assertionに失敗しました: %s", string.buf);
    return new_node_empty(token);
}

//    配列のサイズは定数の場合のみサポートする
//    array_def               = "[" constant_expression? "]" ( "[" constant_expression "]" )*
static Type *array_def(Type *tp) {
    Node *node;
    Type *ret_tp = tp;
    SrcInfo *info;
    // int *a[10][2][3]
    if (consume('[')) {
        if (consume(']')) { //char *argv[];
            tp = new_type_array(tp, -1);    //最初だけ省略できる（初期化が必要）
        } else {
            info = &cur_token_info();
            node = constant_expression();
            if (node->val==0) error_at(info, "配列のサイズが0です");
            tp = new_type_array(tp, node->val);
            expect(']'); 
        }
        ret_tp = tp;
        // ret_tp=tp=ARRAY[10] -> PTR -> INT 
    }

    while (consume('[')) {
        info = &cur_token_info();
        node = constant_expression();
        if (node->val==0) error_at(info, "配列のサイズが0です");
        tp->ptr_of = new_type_array(tp->ptr_of, node->val);
        tp = tp->ptr_of;
        expect(']'); 
        // ARRAYのリストの最後に挿入してゆく
        // ret_tp=tp=ARRAY[10]                               -> PTR -> INT 
        // ret_tp=   ARRAY[10] -> tp=ARRAY[2]                -> PTR -> INT 
        // ret_tp=   ARRAY[10] ->    ARRAY[2] -> tp=ARRAY[3] -> PTR -> INT 
    }

    return ret_tp;
}

//    pointer                 = ( "*" type_qualifier* )*
static Type *pointer(Type *tp) {
    while (consume('*')) {
        tp = new_type_ptr(tp);
        while (1) {
            if (consume(TK_CONST)) {
                tp->is_const = 1;
            } else if (consume(TK_VOLATILE) || consume(TK_RESTRICT) || consume(TK_ATOMIC)) {
                //無視
            } else {
                break;
            }
        }
    }

    return tp;
}

//    type_name               = "typeof" "(" identifier ")"
//                            | specifier_qualifier_list abstract_declarator*
//    abstract_declarator     = pointer? direct_abstract_declarator
//    direct_abstract_declarator = "(" abstract_declarator ")"
//                            | direct_abstract_declarator? "[" assignment_expression "]"
//                            | direct_abstract_declarator? "(" parameter_type_list? ")"  //関数
static Type *type_name(void) {
    Type *tp = specifier_qualifier_list();
    tp = abstract_declarator(tp);
    switch (tp->type) {
    case ENUM:
    case STRUCT:
    case UNION:
        if (tp->node->lst==NULL) {
            error_at(&node_info(tp->node), "不完全な型の定義です");
        }
    default:
        break;
    }
    return tp;
}
static Type *abstract_declarator(Type *tp) {
    tp = pointer(tp);
    tp = direct_abstract_declarator(tp);
    return tp;
}
static Type *direct_abstract_declarator(Type *tp) {
    Type *nest_tp;
    Token *token = cur_token();
    if (consume('(')){
        nest_tp = abstract_declarator(new_type(NEST, 0));
        expect(')');
    }
    if (consume('(')){  //関数定義・宣言確定
        Node *node = new_node(0, NULL, NULL, NULL, token);
        Funcdef *org_funcdef = cur_funcdef;
        tp = new_type_func(tp, node);
        cur_funcdef = new_funcdef();    //一時的にcur_funcdefを作成しスコープを生成
        begin_func_scope();
        node->lhs = parameter_type_list();
        end_scope();
        cur_funcdef = org_funcdef;      //一時的なcur_funcdefはここで捨てる
        replace_nest(nest_tp, tp);
        node->tp = tp = nest_tp;
        expect(')');
    } else if (token_is('[')) {
        tp = array_def(tp);
    }
    return tp;
}

//    declaration_specifiers  = "typeof" "(" identifier ")"
//                            | storage_class_specifier declaration_specifiers*
//                            | type_specifier          declaration_specifiers*
//                            | type_qualifier          declaration_specifiers*
//                            | function_specifier      declaration_specifiers*
//    storage_class_specifier = "typedef" | "static" | "extern" | "auto" | "register"
//    type_specifier          = "void" | "_Bool" | "char" | "short" | "int" | "long" | "signed" | "unsigned"
//                            | struct_or_union_specifier | enum_specifier | typedef_name
//    type_qualifier          = "const" | "restrict" | "volatile" | "_Atomic"
//    function_specifier      = "inline" | "_Noreturn"
//type_only: type_specifierとtype_qualifierのみ認識する（構造体・共用体のメンバ用）
static Type *declaration_specifiers(int type_only) {
    Type *tp;

    if (consume(TK_TYPEOF)) {
        expect('(');
        char *name;
        expect_ident(&name, "変数名");
        Node *node = new_node_ident(name, NULL);
        tp = get_typeof(node->tp);
        expect(')');
    	return tp;
    }

    Node *node = NULL;
    SrcInfo *us_info;
    TPType type = 0;
    StorageClass sclass = SC_UNDEF;
    int is_unsigned = -1;   //-1:未指定、0:signed、1:unsigned
    int is_const  = 0;

    while (1) {
        SrcInfo *info = &cur_token_info();
        //type_specifier
        if (consume(TK_VOID)) {
            if (type) error_at(info, "型指定が不正です\n");
            type = VOID;
        } else if (consume(TK_BOOL)) {
            if (type) error_at(info, "型指定が不正です\n");
            type = BOOL;
        } else if (consume(TK_CHAR)) {
            if (type) error_at(info, "型指定が不正です\n");
            type = CHAR;
        } else if (consume(TK_SHORT)) {
            if (type && type!=INT) error_at(info, "型指定が不正です\n");
            type = SHORT;
        } else if (consume(TK_INT)) {
            if (type && type!=SHORT && type!=LONG && type!=LONGLONG) error_at(info, "型指定が不正です\n");
            type = INT;
        } else if (consume(TK_LONG)) {
            if      (!type)        type = LONG;
            else if (type==INT)    type = LONG;
            else if (type==LONG)   type = LONGLONG;
            else error_at(info, "型指定が不正です\n");
        } else if (consume(TK_FLOAT)) {
            if (type) error_at(info, "型指定が不正です\n");
            type = FLOAT;
        } else if (consume(TK_DOUBLE)) {
            if (type==LONG) type = LONGDOUBLE;
            else if (type) error_at(info, "型指定が不正です\n");
            else type = DOUBLE;
        } else if (consume(TK_STRUCT)) {
            if (type) error_at(info, "型指定が不正です\n");
            type = STRUCT;
            node = struct_or_union_specifier(type);
        } else if (consume(TK_UNION)) {
            if (type) error_at(info, "型指定が不正です\n");
            type = UNION;
            node = struct_or_union_specifier(type);
        } else if (consume(TK_ENUM)) {
            if (type) error_at(info, "型指定が不正です\n");
            type = ENUM;
            node = enum_specifier();
        } else if (consume_typedef(&node)) {
            if (type) error_at(info, "型指定が不正です\n");
            type = node->tp->type;
        } else if (consume(TK_SIGNED)) {
            if (is_unsigned>=0) error_at(info, "型指定が不正です\n");
            is_unsigned = 0;
            us_info = info;
        } else if (consume(TK_UNSIGNED)) {
            if (is_unsigned>=0) error_at(info, "型指定が不正です\n");
            is_unsigned = 1;
            us_info = info;
        //type_qualifier
        } else if (consume(TK_RESTRICT) || consume(TK_VOLATILE) || consume(TK_ATOMIC)) {
            //無視
        } else if (consume(TK_CONST)) {
            is_const = 1;
        //storage_class_specifier
        } else if (consume(TK_AUTO)) {
            if (sclass) error_at(info, "strage classが重複しています\n");
            sclass = SC_AUTO;
        } else if (consume(TK_REGISTER)) {
            if (sclass) error_at(info, "strage classが重複しています\n");
            sclass = SC_REGISTER;
        } else if (consume(TK_STATIC)) {
            if (sclass) error_at(info, "strage classが重複しています\n");
            sclass = SC_STATIC;
        } else if (consume(TK_EXTERN)) {
            if (sclass) error_at(info, "strage classが重複しています\n");
            sclass = SC_EXTERN;
        } else if (consume(TK_TYPEDEF)) {
            if (sclass) error_at(info, "strage classが重複しています\n");
            sclass = SC_TYPEDEF;
        //function_specifier
        } else if (consume(TK_INLINE) || consume(TK_NORETURN)) {
            //無視
        } else {
            if (!type && is_unsigned>=0) type = INT;
            if (!type) error_at(info, "型名がありません\n");
            break;
        }
        if (type_only && sclass!=SC_UNDEF) error_at(info, "storage classは指定できません");
        if (type==VOID && is_unsigned>=0)
            error_at(info, "void型の指定が不正です\n");
    }

    Type *top_tp;
    if (node) { //enum,typedef_name,struct,union
        if (is_unsigned>=0) error_at(us_info, "enum/typedef/struct/union名に対してsigned/unsignedの指定はできません\n");
        top_tp = tp = node->tp;
        top_tp = tp = dup_type(node->tp);
        while (tp->ptr_of) tp = tp->ptr_of; //PTRとARRAYを飛ばす
    } else if (type==BOOL) {
        if (is_unsigned>=0) error_at(us_info, "_Boolに対してsigned/unsignedの指定はできません\n");
        is_unsigned = 1;
        top_tp = tp = new_type(type, is_unsigned);
    } else {
        if (is_unsigned<0) is_unsigned = 0;
        top_tp = tp = new_type(type, is_unsigned);
    }
    tp->tmp_sclass = sclass;
    if (is_const) tp->is_const = 1;
    return top_tp;
}

//    struct_or_union_specifier = ( "struct" | "union" ) identifier? "{" struct_declaration+ "}"
//                            | ( "struct" | "union" ) identifier
//    struct_declaration      = specifier_qualifier_list struct_declarator_list* ";"
//                            | static_assert_declaration
//    specifier_qualifier_list = type_specifier specifier_qualifier_list*
//                            | type_qualifier specifier_qualifier_list*
//    struct_declarator_list  = struct_declarator ( "," struct_declarator )*
//    struct_declarator       = declarator
//                            | declarator? ":" constant_expression
static Node *struct_or_union_specifier(TPType type) {
    Node *node;
    if (type==STRUCT) node = new_node(ND_STRUCT_DEF, NULL, NULL, new_type(STRUCT, 0), cur_token());
    else              node = new_node(ND_UNION_DEF,  NULL, NULL, new_type(UNION,  0), cur_token());
    char *name = NULL;
    if (consume_ident(&name)) {
        node->name = name;
    } else {
        node->name = NODE_NONAME;
    }
    node->tp->node = node;

    Node *old_cur_structdef = cur_structdef;
    cur_structdef = node;

    if (consume('{')) { //完全定義
        node->lst = new_vector();
        node->map = new_map();
        if (name) node = regist_tagname(node);
        do {
            Node *member = struct_declaration();
            if (member->type==ND_LIST) {
                //int x,y,z;
                vec_copy(node->lst, member->lst);
            } else if (node_is_anonymouse_struct_or_union(member)) {
                //  struct {            //<==node
                //      int a;
                //      union {         //<==member: 無名構造体・共用体
                //          int  ua;    //スコープを親のレベルに引き上げる
                //          long ub;
                //      };
                //  }
                Vector *src_lst = member->tp->node->lst;
                for (int i=0; i<lst_len(src_lst); i++) {
                    regist_var_def((Node*)lst_data(src_lst,i));
                }
                vec_push(node->lst, member);
            } else {
                //int a;
                vec_push(node->lst, member);
            }
        } while (!consume('}'));
        refresh_typedef(node);  //不完全定義typedefがあれば完全定義に置き換える

        //構造体メンバのoffset(memb->offset)と、構造体のサイズを設定する(node->val)。
        set_struct_size(node, 0/*base_offset*/);
    } else if (name==NULL) {
        expect('{');    //error
    } else {            //不完全定義
        node = regist_tagname(node);
    }

    //dump_symbol(-1, __func__);
    //dump_tagname();
    //dump_type(node->tp,__func__);
    cur_structdef = old_cur_structdef;
    return node;
}
//    struct_declaration      = specifier_qualifier_list struct_declarator_list* ";"
//                            | static_assert_declaration
static Node *struct_declaration(void) {
    if (token_is(TK_SASSERT)) {
        return static_assert_declaration();
    }
    Type *decl_spec = specifier_qualifier_list();
    Node *node;
    if (token_is(';')) {
        //変数名なしが許されるのは無名構造体・共用体のみ
        if (!type_is_struct_or_union(decl_spec) || decl_spec->node->name[0]!='<') {
            expect_ident(NULL, "変数名");
        }
        node = new_node(ND_MEMBER_DEF, NULL, NULL, decl_spec, decl_spec->node->token);
        node->lst = decl_spec->node->lst;
        node->map = decl_spec->node->map;
    } else {
        node = struct_declarator_list(decl_spec);
    }
    expect(';');
    return node;
}
//    specifier_qualifier_list = type_specifier specifier_qualifier_list*
//                            | type_qualifier specifier_qualifier_list*
static Type *specifier_qualifier_list(void) {
    Type *tp = declaration_specifiers(1/*=type_only*/);
    return tp;
}
//    struct_declarator_list  = struct_declarator ( "," struct_declarator )*
static Node *struct_declarator_list(Type *decl_spec) {
    Node *node = struct_declarator(decl_spec);
    if (consume(',')) {
        Node *last_node;
        node = new_node_list(node, node->token);
        Vector *lists = node->lst;
        vec_push(lists, last_node=struct_declarator(decl_spec));
        while (consume(',')) {
            vec_push(lists, last_node=struct_declarator(decl_spec));
        }
        node->tp = last_node->tp;
    }
    return node;
}
//    struct_declarator       = declarator
//                            | declarator? ":" constant_expression
static Node *struct_declarator(Type *tp) {
    Node *node = declarator(tp, NULL, NULL);
    node->type = ND_MEMBER_DEF;
    if (tp->node) {
        node->lst = tp->node->lst;
        node->map = tp->node->map;
    }
    regist_var_def(node);
    return node;
}

//不完全定義のtypedefがあれば完全定義に置き換える(enum/struct/union)
static void refresh_typedef(Node *struc) {
    assert(struc->lst!=NULL);
    Map *symbol_map = stack_top(symbol_stack); 
    int size = lst_len(symbol_map->vals);
    Node **nodes = (Node**)symbol_map->vals->data;
    for (int i=0; i<size; i++) {
        Node *typdef = nodes[i];
        if (typdef->type!=ND_TYPEDEF) continue;
        switch (typdef->tp->type) {
        case ENUM:
        case STRUCT:
        case UNION:
            if (!node_is_noname(struc) && strcmp(struc->name, typdef->tp->node->name)==0) {
                assert(struc->tp->type==typdef->tp->type);
                typdef->tp->node = struc;
            }
            break;
        default:
            break;
        }
    }
}

//    enum_specifier          = "enum" identifier? "{" enumerator ( "," enumerator )* ","? "}"
//                            | "enum" identifier
//    enumerator              = enumeration_constant ( "=" constant-expression )?
static Node *enum_specifier(void) {
    Node *node = new_node(ND_ENUM_DEF, NULL, NULL, new_type(ENUM, 0), cur_token());
    char *name = NULL;
    if (consume_ident(&name)) {
        node->name = name;
    } else {
        node->name = NODE_NONAME;
    }
    if (consume('{')) {
        Node *em = enumerator(node, 0);
        node->lst = new_vector();
        vec_push(node->lst, em);
        while (consume(',')) {
            if (token_is('}')) break;
            em = enumerator(node, em->val+1);
            vec_push(node->lst, em);
        }
        expect('}');
        refresh_typedef(node);  //不完定義typedefがあれば完全定義に置き換える
    } else if (name==NULL) {
        expect('{');    //error
    }
    node->tp->node = node;
    if (name) node = regist_tagname(node);
    return node;
}
static Node *enumerator(Node *enum_def, int default_val) {
    Node *node = new_node(ND_ENUM, enum_def, NULL, new_type(INT, 0), cur_token());
    char *name;
    expect_ident(&name, "enum名");
    node->name = name;
    if (consume('=')) {
        Node *c = constant_expression();
        node->val = c->val;
    } else {
        node->val = default_val;
    }
    regist_symbol(node);
    return node;
}

static Node *statement(void) {
    Node *node, *lhs, *rhs;
    Token *token = cur_token();
    Token *token2;

    // labeled_statement
    if (token_is(TK_IDENT) && next_token_is(':')) {
        char *name;
        expect_ident(&name, "ラベル名");
        consume(':');
        rhs = statement();
        node = new_node(ND_LABEL, NULL, rhs, rhs->tp, token);
        node->name = name;
        regist_label(node);
        return node;
    } else if (consume(TK_CASE)) {
        lhs = constant_expression();
        expect(':');
        node = new_node(ND_CASE, lhs, statement(), lhs->tp, token);
        node->val = lhs->val;
        regist_case(node);
        return node;
    } else if (consume(TK_DEFAULT)) {
        expect(':');
        rhs = statement();
        node = new_node(ND_DEFAULT, NULL, rhs, rhs->tp, token);
        regist_case(node);
        return node;
    } else if (token_is('{')) {
        return compound_statement(0);    //{ ブロック }

    //selection_statement
    } else if (consume(TK_IF)) {        //if(A)B else C = {if(A){B}else{C}}
        Node *node_A, *node_B;
        begin_local_scope();
        expect('(');
        node_A = expression();          //A
        check_scalar(node_A, "条件部");
        expect(')');
        token2 = cur_token();
        begin_local_scope();
        node_B = statement();           //B
        end_scope();
        node = new_node(ND_UNDEF, node_A, node_B, NULL, token2); //lhs
        if (consume(TK_ELSE)) {
            begin_local_scope();
            node = new_node(ND_IF, node, statement(), NULL, token); //C
            end_scope();
        } else {
            node = new_node(ND_IF, node, NULL, NULL, token);
        }
        end_scope();
        return node;
    } else if (consume(TK_SWITCH)) {    //switch(A)B = {switch(A){B}}
        begin_local_scope();
        expect('(');
        node = expression();            //A
        check_scalar(node, "条件部");
        expect(')');
        node = new_node(ND_SWITCH, node, NULL, NULL, token);
        node->map = new_map();
        Node *org_cur_switch = cur_switch;
        cur_switch = node;
        begin_local_scope();
        node->rhs = statement();        //B
        end_scope();
        cur_switch = org_cur_switch;
        end_scope();
        return node;

    //iteration_statement
    } else if (consume(TK_WHILE)) { //while(A)B = {while(A){B}}       lhs=A, rhs=B
        begin_local_scope();
        expect('(');
        node = expression();            //A
        check_scalar(node, "条件部");
        expect(')');
        begin_local_scope();
        node = new_node(ND_WHILE, node, statement(), NULL, token);  //B
        end_scope();
        end_scope();
        return node;
    } else if (consume(TK_DO)) {    //do A while(B) = {do{A}while(B)}    lhs=A, rhs=B
        begin_local_scope();
        begin_local_scope();
        node = statement();             //A
        end_scope();
        if (!consume(TK_WHILE)) error_at(&cur_token_info(), "doに対応するwhileがありません");
        expect('(');
        node = new_node(ND_DO, node, expression(), NULL, token);    //B
        check_scalar(node->rhs, "条件部");
        end_scope();
        expect(')');
    } else if (consume(TK_FOR)) {   //for(A;B;C)D = {for(A;B;C){D}}      lhs->lhs=A, lhs->rhs=B, rhs->lhs=C, rhs->rhs=D
        Node *node1, *node2;
        begin_local_scope();
        expect('(');
        if (consume(';')) {
            node1 = new_node_empty(cur_token());
        } else if (token_is_type_spec()) {
            node1 = declaration(NULL, NULL);
        } else {
            node1 = expression();         //A
            expect(';');
        }
        if (consume(';')) {
            node2 = new_node_empty(cur_token());
        } else {
            node2 = expression();         //B
            check_scalar(node2, "条件部");
            expect(';');
        }
        node = new_node(ND_UNDEF, node1, node2, NULL, token);       //A,B
        if (consume(')')) {
            node1 = new_node_empty(cur_token());
        } else {
            node1 = expression();         //C
            expect(')');
        }
        begin_local_scope();
        node2 = new_node(ND_UNDEF, node1, statement(), NULL, token);     //C,D
        end_scope();
        node = new_node(ND_FOR, node, node2, NULL, token);   //(A,B),(C,D)
        end_scope();
        return node;

    //jump_statement
    } else if (consume(TK_GOTO)) {      //goto label;      name=label
        char *name;
        node = new_node(ND_GOTO, NULL, NULL, NULL, token);
        expect_ident(&name, "ラベル名");
        node->name = name;
        regist_label(node);
    } else if (consume(TK_CONTINUE)) {  //continue
        node = new_node(ND_CONTINUE, NULL, NULL, NULL, token);
    } else if (consume(TK_BREAK)) {     //break
        node = new_node(ND_BREAK, NULL, NULL, NULL, token);
    } else if (consume(TK_RETURN)) {
        if (token_is(';')) {
            node = new_node(ND_RETURN, NULL, NULL, NULL, token);
        } else {
            rhs = expression();
            node = new_node(ND_RETURN, NULL, rhs, rhs->tp, token);
            node->sclass = rhs->sclass;
        }
        check_return(node); //戻り値の妥当性をチェック

    //statement_expression
    } else if (consume(';')) {
        return new_node_empty(cur_token());
    } else {
        node = expression();
    }

    expect(';');
    return node;
}

static Node *compound_statement(int is_func_body) {
    Node *node;
    expect('{');

    begin_local_scope();
    node = new_node_block(cur_token());

    if (is_func_body) {
        //関数の先頭に static const char __func__[]="function name"; 相当の変数を登録する
        Token *token = cur_token();
        String string = {cur_funcdef->func_name, strlen(cur_funcdef->func_name)+1};
        Node *string_node = new_node_string(&string, token);
        Type *tp = string_node->tp;
        tp->ptr_of->is_const = 1;
        tp->ptr_of->tmp_sclass = SC_STATIC;
        Node *func_name_node = new_node_var_def("__func__", tp, token);
        Node *var_node = new_node_ident(func_name_node->name, token);  //=の左辺
        func_name_node->unused = 1; //参照されたときに0となり、有効になる
        func_name_node->rhs = new_node('=', var_node, string_node, tp, token);
        func_name_node->sclass = SC_STATIC;
    }

    Token *last_token = cur_token();  //'}'のはず
    while (!consume('}')) {
        Node *block_item;
        if (token_is_type_spec() || token_is(TK_SASSERT)) {
            SrcInfo *info = &cur_token_info();
            block_item = declaration(NULL, NULL);
            if (block_item->type==ND_TYPE_DECL && block_item->name==NULL &&
                type_is_struct_or_union(block_item->tp) && node_is_noname(block_item->tp->node)) {
                warning_at(info, "意味のない宣言です");
            }
        } else {
            block_item = statement();
        }
        if (block_item->type==ND_EMPTY) continue;
        vec_push(node->lst, block_item);
        last_token = cur_token();
    }
    end_scope();

    if (is_func_body) {
        Node *node2 = new_node(ND_FUNC_END, NULL, NULL, NULL, last_token);
        node2->name = cur_funcdef->func_name;
        vec_push(node->lst, node2);
    }
    return node;
}

//expr：単なる式またはそのコンマリスト（左結合）
//リストであればリスト(ND_LIST)を作成する
//    expression        = assignment_expression ( "," assignment_expression )* 
static Node *expression(void) {
    Token *token = cur_token();
    Node *node = assignment_expression();
    Node *last_node = node;
    if (consume(',')) {
        node = new_node_list(node, token);
        Vector *lists = node->lst;
        vec_push(lists, last_node=assignment_expression());
        while (consume(',')) {
            vec_push(lists, last_node=assignment_expression());
        }
    } else {
        return node;
    }
    node->tp = last_node->tp;
    return node;
}

//    constant_expression     = conditional_expression
// 戻り値のnode->valに評価された定数を設定
static Node *constant_expression(void) {
    Token *token = cur_token();
    Node *node = conditional_expression();
    long val;
    if (!node_is_constant(node, &val))
        error_at(&node_info(node), "定数式が必要です");
    node = new_node(ND_NUM, NULL, node, node->tp, token);
    node->val = val;
    return node;
}

#define copy_node_name(_n1,_n2) ((_n1)->name=(_n2)->name, (_n1)->disp_name=(_n2)->disp_name)
//代入（右結合）
//    assignment_expression   = conditional_expression
//                            | unary_expression ( "=" | "+=" | "-=" ) assignment_expression
static Node *assignment_expression(void) {
    Node *node = conditional_expression(), *rhs;
    int is_lvalue = 1;
    switch (node->type) {
    case ND_INC_PRE:
    case ND_DEC_PRE:
    case ND_INDIRECT:
    case ND_INC:
    case ND_DEC:
    case ND_LOCAL_VAR:
    case ND_GLOBAL_VAR:
        is_lvalue = 1;
        break;
    default:
        is_lvalue = 0;
        break;
    }
    Token *token = cur_token();
    if (consume('=')) {
        if (!is_lvalue || node->tp->type==ARRAY) error_at(&node_info(node), "左辺値ではありません");
        check_arg(node, &token->info, CHK_CONST, "代入");
        rhs = assignment_expression();
        if (rhs->type==ND_INDIRECT && rhs->tp->type!=PTR && rhs->tp->is_const) {
            rhs->tp = dup_type(rhs->tp);
            rhs->tp->is_const = 0;
        }
        check_assignment(node, rhs, &token->info);
        node = new_node('=', node, rhs, node->tp, token); //ND_ASSIGN
        copy_node_name(node, node->lhs);
    } else if (consume(TK_PLUS_ASSIGN)      // +=
            || consume(TK_MINUS_ASSIGN)     // -=
            || consume(TK_MUL_ASSIGN)       // *=
            || consume(TK_DIV_ASSIGN)       // /=
            || consume(TK_MOD_ASSIGN)       // %=
            || consume(TK_SHIFTR_ASSIGN)    // >>=
            || consume(TK_SHIFTL_ASSIGN)    // <<=
            || consume(TK_AND_ASSIGN)       // &=
            || consume(TK_XOR_ASSIGN)       // ^=
            || consume(TK_OR_ASSIGN)) {     // |=
        char *msg;
        NDtype type;    
        switch (last_token_type()) {
        case TK_PLUS_ASSIGN:   type = ND_PLUS_ASSIGN;   msg = "+="; break;
        case TK_MINUS_ASSIGN:  type = ND_MINUS_ASSIGN;  msg = "-="; break;
        case TK_MUL_ASSIGN:    type = ND_MUL_ASSIGN;    msg = "*="; break;
        case TK_DIV_ASSIGN:    type = ND_DIV_ASSIGN;    msg = "/="; break;
        case TK_MOD_ASSIGN:    type = ND_MOD_ASSIGN;    msg = "%="; break;
        case TK_SHIFTR_ASSIGN: type = ND_SHIFTR_ASSIGN; msg = ">>="; break;
        case TK_SHIFTL_ASSIGN: type = ND_SHIFTL_ASSIGN; msg = "<<="; break;
        case TK_AND_ASSIGN:    type = ND_AND_ASSIGN;    msg = "&="; break;
        case TK_XOR_ASSIGN:    type = ND_XOR_ASSIGN;    msg = "^="; break;
        case TK_OR_ASSIGN:     type = ND_OR_ASSIGN;     msg = "|="; break;
        default: abort(); break;
        }
        if (!is_lvalue || node->tp->type==ARRAY) error_at(&node_info(node), "左辺値ではありません");
        check_arg(node, &token->info, CHK_CONST, msg);
        rhs = assignment_expression(); 
        check_arg(rhs, &node_info(rhs), CHK_NOT(CHK_INTEGER), msg);
        node = new_node(type, node, rhs, node->tp, token);
        copy_node_name(node, node->lhs);
    }
    return node;
}

//三項演算子（右結合）
//    conditional_expression    = logical_OR_expression "?" expression ":" conditional_expression
static Node *conditional_expression(void) {
    Node *node = logical_OR_expression(), *sub_node, *lhs, *rhs;
    Token *token = cur_token();
    if (consume('?')) {
        check_scalar(node, "条件部");
        lhs = expression();
        expect(':');
        rhs = conditional_expression();
        sub_node = new_node(ND_UNDEF, lhs, rhs, lhs->tp, token);
        node = new_node(ND_TRI_COND, node, sub_node, lhs->tp, node->token);
        node->disp_name = "?:";
    }
    return node;
}

//論理和（左結合）
//    logical_OR_expression  = logical_AND_expression ( "||" logical_AND_expression )*
static Node *logical_OR_expression(void) {
    Node *node = logical_AND_expression();
    for (;;) {
        Token *token = cur_token();
        if (consume(TK_LOR)) {
            node = new_node(ND_LOR, node, logical_AND_expression(), new_type(INT, 0), token);
            check_arg(node, &token->info, CHK_BINARY|CHK_STRUCT|CHK_UNION, "論理和||");
        } else {
            break;
        }
    }
    return node;
}

//論理積（左結合）
//    logical_AND_expression = inclusive_OR_expression ( "&&" inclusive_OR_expression )*
static Node *logical_AND_expression(void) {
    Node *node = inclusive_OR_expression();
    for (;;) {
        Token *token = cur_token();
        if (consume(TK_LAND)) {
            node = new_node(ND_LAND, node, inclusive_OR_expression(), new_type(INT, 0), token);
            check_arg(node, &token->info, CHK_BINARY|CHK_STRUCT|CHK_UNION, "論理積&&");
        } else {
            break;
        }
    }
    return node;
}

//OR（左結合）
//    inclusive_OR_expression = exclusive_OR_expression ( "|" exclusive_OR_expression )*
static Node *inclusive_OR_expression(void) {
    Node *node = exclusive_OR_expression(), *rhs;
    for (;;) {
        Token *token = cur_token();
        if (consume('|')) {
            rhs = exclusive_OR_expression();
            node = new_node('|', node, rhs, new_type(INT, 0), token);
            check_arg(node, &token->info, CHK_BINARY|CHK_NOT(CHK_INTEGER), "ビット論理和|");
        } else {
            break;
        }
    }
    return node;
}

//EX-OR（左結合）
//    inclusive_OR_expression = AND_expression ( "|" AND_expression )*
static Node *exclusive_OR_expression(void) {
    Node *node = AND_expression(), *rhs;
    for (;;) {
        Token *token = cur_token();
        if (consume('^')) {
            rhs = AND_expression();
            node = new_node('^', node, rhs, new_type(INT, 0), token);
            check_arg(node, &token->info, CHK_BINARY|CHK_NOT(CHK_INTEGER), "ビット排他的論理和^");
        } else {
            break;
        }
    }
    return node;
}

//AND（左結合）
//    AND_expression = equality_expression ( "&" equality_expression )*
static Node *AND_expression(void) {
    Node *node = equality_expression(), *rhs;
    for (;;) {
        Token *token = cur_token();
        if (consume('&')) {
            rhs = equality_expression();
            node = new_node('&', node, rhs, new_type(INT, 0), token);
            check_arg(node, &token->info, CHK_BINARY|CHK_NOT(CHK_INTEGER), "ビット論理和&");
        } else {
            break;
        }
    }
    return node;
}

//等価演算（左結合）
//    equality_expression    = relational_expression ( "==" relational_expression | "!=" relational_expression )*
static Node *equality_expression(void) {
    Node *node = relational_expression();
    for (;;) {
        Token *token = cur_token();
        if (consume(TK_EQ)) {
            node = new_node(ND_EQ, node, relational_expression(), new_type(INT, 0), token);
            check_arg(node, &token->info, CHK_BINARY|CHK_STRUCT|CHK_UNION, "==");
        } else if (consume(TK_NE)) {
            node = new_node(ND_NE, node, relational_expression(), new_type(INT, 0), token);
            check_arg(node, &token->info, CHK_BINARY|CHK_STRUCT|CHK_UNION, "!=");
        } else {
            break;
        }
    }
    return node;
}

//関係演算（左結合）
//   relational_expression   = shift_expression ( ( "<" | "<=" | ">" | ">=" ) shift_expression )*
static Node *relational_expression(void) {
    Node *node = shift_expression();
    for (;;) {
        Token *token = cur_token();
        if (consume('<')) {
            node = new_node('<',   node, shift_expression(), new_type(INT, 0), token);
            check_arg(node, &token->info, CHK_BINARY|CHK_STRUCT|CHK_UNION, "<");
        } else if (consume(TK_LE)) {
            node = new_node(ND_LE, node, shift_expression(), new_type(INT, 0), token);
            check_arg(node, &token->info, CHK_BINARY|CHK_STRUCT|CHK_UNION, "<=");
        } else if (consume('>')) {
            node = new_node('<',   shift_expression(), node, new_type(INT, 0), token);
            check_arg(node, &token->info, CHK_BINARY|CHK_STRUCT|CHK_UNION, ">");
        } else if (consume(TK_GE)) {
            node = new_node(ND_LE, shift_expression(), node, new_type(INT, 0), token);
            check_arg(node, &token->info, CHK_BINARY|CHK_STRUCT|CHK_UNION, ">=");
        } else {
            break;
        }
    }
    return node;
}

//シフト（左結合）
//    shift_expression        = additive_expression ( ( "<<" | ">>" ) additive_expression )*
static Node *shift_expression(void) {
    Node *node = additive_expression(), *rhs;
    for (;;) {
        Token *token = cur_token();
        if (consume(TK_SHIFTL)) {
            rhs = additive_expression();
            node = new_node(ND_SHIFTL, node, rhs, node->tp, token);
            check_arg(node, &token->info, CHK_BINARY|CHK_NOT(CHK_INTEGER), "<<");
        } else if (consume(TK_SHIFTR)) {
            rhs = additive_expression();
            node = new_node(ND_SHIFTR, node, rhs, node->tp, token);
            check_arg(node, &token->info, CHK_BINARY|CHK_NOT(CHK_INTEGER), ">>");
        } else {
            break;
        }
    }
    return node;
}

//加減算（左結合）
//    additive_expression         = multiplicative_expression ( "+" multiplicative_expression | "-" multiplicative_expression )*
static Node *additive_expression(void) {
    Node *node, *lhs, *rhs;
    Type *tp;
    node = lhs = multiplicative_expression();
    for (;;lhs = node) {
        Token *token = cur_token();
        if (consume('+')) {
            rhs = multiplicative_expression();
            if (node_is_ptr(lhs) && node_is_ptr(rhs))
                error_at(&node_info(lhs), "ポインタ同士の加算です");
            tp = node_is_ptr(lhs) ? lhs->tp : rhs->tp;
            node = new_node('+', lhs, rhs, tp, token);
            node->disp_name = "+";
        } else if (consume('-')) {
            rhs = multiplicative_expression();
            if (node_is_ptr(lhs) && node_is_ptr(rhs)) {
                if (!type_eq(lhs->tp, rhs->tp)) 
                    error_at(&node_info(lhs), "異なるタイプのポインタによる減算です: %s vs %s",
                        get_node_type_str(lhs), get_node_type_str(rhs));
            } else if (!node_is_ptr(lhs) && node_is_ptr(rhs)) {
                error_at(&node_info(lhs), "ポインタによる減算です");
            }
            if (node_is_ptr(lhs)) {
                if (node_is_ptr(rhs)) tp = new_type(INT, 0);
                else                  tp = lhs->tp;
            } else {
                tp = rhs->tp;
            }
            node = new_node('-', lhs, rhs, tp, token);
            node->disp_name = "-";
        } else {
            break;
        }
    }
    return node;
}

//乗除算、剰余（左結合）
//    multiplicative_expression = cast_expression ( ( "*" | "/" | "%" ) cast_expression )*
static Node *multiplicative_expression(void) {
    Node *node = cast_expression();
    for (;;) {
        Token *token = cur_token();
        if (consume('*')) {
            node = new_node('*', node, cast_expression(), node->tp, token);
            check_arg(node, &token->info, CHK_BINARY|CHK_NOT(CHK_INTEGER), "乗算*");
        } else if (consume('/')) {
            node = new_node('/', node, cast_expression(), node->tp, token);
            check_arg(node, &token->info, CHK_BINARY|CHK_NOT(CHK_INTEGER), "除算/");
        } else if (consume('%')) {
            node = new_node('%', node, cast_expression(), node->tp, token);
            check_arg(node, &token->info, CHK_BINARY|CHK_NOT(CHK_INTEGER), "剰余%");
        } else {
            break;
        }
    }
    return node;
}

//キャスト（右結合）
//    cast_expression         = unary_expression
//                            | "(" type_name ")" cast_expression
static Node *cast_expression(void) {
    Node *node;
    Type *tp;
    Token *token = cur_token();
    if (token_is('(') && next_token_is_type_spec()) {
        expect('(');
        tp = type_name();
        expect(')');
        node = new_node(ND_CAST, NULL, cast_expression(), tp, token);
    } else {
        node = unary_expression();
    }
    return node;
}

//前置単項演算子（右結合）
//    unary_expression        = postfix_expression
//                            | ( "+" | "-" |  "!" | "~" | "*" | "&") cast_expression
//                            | ( "++" | "--" )? unary_expression
//                            | "sizeof" unary_expression
//                            | "sizeof"   "(" type_name ")"
//                            | "_Alignof" unary_expression
//                            | "_Alignof" "(" type_name ")"
static Node *unary_expression(void) {
    Node *node;
    Token *token = cur_token();
    if (consume('+')) {
        node = cast_expression();
        check_arg(node, &token->info, CHK_NOT(CHK_INTEGER), "単項+");
    } else if (consume('-')) {
        node = cast_expression();
        check_arg(node, &token->info, CHK_NOT(CHK_INTEGER), "単項-");
        if (node->type==ND_NUM) {
            node->val = -(node->val);
        } else {
            node = new_node(ND_NEG, NULL, node, node->tp, token);
        }
    } else if (consume('!')) {
        node = cast_expression();
        check_arg(node, &token->info, CHK_NOT(CHK_INTEGER|CHK_PTR), "!");
        node = new_node('!', NULL, node, new_type(INT, 0), token);
    } else if (consume('~')) {
        node = cast_expression();
        check_arg(node, &token->info, CHK_NOT(CHK_INTEGER), "~");
        node = new_node(ND_BNOT, NULL, node, node->tp, token);
    } else if (consume(TK_INC)) {
        node = unary_expression();
        check_arg(node, &token->info, CHK_STRUCT|CHK_UNION|CHK_ARRAY|CHK_CONST, "++");
        node = new_node(ND_INC_PRE, NULL, node, node->tp, token);
        copy_node_name(node, node->rhs);
    } else if (consume(TK_DEC)) {
        node = unary_expression();
        check_arg(node, &token->info, CHK_STRUCT|CHK_UNION|CHK_ARRAY|CHK_CONST, "--");
        node = new_node(ND_DEC_PRE, NULL, node, node->tp, token);
        copy_node_name(node, node->rhs);
    } else if (consume('*')) {
        node = cast_expression();
        check_arg(node, &token->info, CHK_NOT(CHK_ARRAY|CHK_PTR), "*");
        if (!type_is_ptr(node->tp))
            error_at(&node_info(node), "'*'は非ポインタ型(%s)を参照しています", get_node_type_str(node));
        node = new_node(ND_INDIRECT, NULL, node, node->tp->ptr_of, token);
        if (display_name(node->rhs)) {
            char *buf = malloc(strlen(display_name(node->rhs))+4);
            sprintf(buf, "*(%s)", display_name(node->rhs));
            node->disp_name = buf;
        }
    } else if (consume('&')) {
        node = cast_expression();
        node = new_node(ND_ADDRESS, NULL, node, new_type_ptr(node->tp), token);
        if (display_name(node->rhs)) {
            char *buf = malloc(strlen(display_name(node->rhs))+4);
            sprintf(buf, "&(%s)", display_name(node->rhs));
            node->disp_name = buf;
        }
    } else if (consume(TK_SIZEOF) || consume(TK_ALIGNOF)) {
        int is_sizeof = (last_token_type()==TK_SIZEOF);
        Type *tp;
        token = cur_token();
        if (token_is('(')) {
            if (next_token_is_type_spec()) {
                consume('(');
                tp = type_name();
                //dump_type(tp,__func__);
                expect(')');
            } else {
                node = unary_expression();
                tp = node->tp;
            }
        } else {
            node = unary_expression();
            tp = node->tp;
        }
        if (tp->type==ARRAY && tp->array_size<=0) error_at(&token->info, "不完全型のサイズは未定義です");
        node = new_node_num(is_sizeof?size_of(tp):align_of(tp), token);
    } else {
        node = postfix_expression();
    }
    return node;
}

//後置単項演算子（左結合）
//    postfix_expression      = primary_expression 
//                            | primary_expression "[" expression "]"
//                            | primary_expression "(" assignment_expression? ( "," assignment_expression )* ")"
//                            | primary_expression "." identifier
//                            | primary_expression "->" identifier
//                            | primary_expression "++"
//                            | primary_expression "--"
static Node *postfix_expression(void) {
    Node *node = primary_expression();
    for (;;) {
        Token *token = cur_token();
        Type *tp = node->tp;
        if (consume('(')) {  //関数コール
            #define node_is_func(node) ((node)->tp->type==FUNC||((node)->tp->type==PTR &&(node)->tp->ptr_of->type==FUNC))
            if (node->type!=ND_IDENT && !node_is_func(node))
                error_at(&token->info, "%sに対して関数コールできません", get_node_type_str(node));
            node = new_node_func_call(node->name, node->token);
            if (!consume(')')) { 
                node->lhs = expression();
                if (node->lhs->type != ND_LIST) {
                    node->lhs = new_node_list(node->lhs, token);
                }
                expect(')');
            }
            check_funccall(node);
        } else if (node->type==ND_IDENT) {
            error_at(&node_info(node), "'%s'は未定義の変数です", node->name);
        } else if (consume('[')) {
            // a[3] => *(a+3)
            // a[3][2] => *(*(a+3)+2)
            token = cur_token();
            Node *rhs = expression();
            Node *base_node = node;
            long val;
            if (tp->type==ARRAY && node_is_constant(rhs, &val)) {
                tp = node->tp->ptr_of ? node->tp->ptr_of : rhs->tp->ptr_of;
                if (tp==NULL) error_at(&cur_token_info(), "ここでは配列を指定できません");
                node->offset -= size_of(tp)*val;
                node = new_node(ND_INDIRECT, NULL, node, tp, token);
            } else {
                node = new_node('+', node, rhs, tp ,token);
                tp = node->tp->ptr_of ? node->tp->ptr_of : rhs->tp->ptr_of;
                if (tp==NULL) error_at(&cur_token_info(), "ここでは配列を指定できません");
                node = new_node(ND_INDIRECT, NULL, node, tp, token);
            }
            if (display_name(base_node)) {
                char *buf = malloc(strlen(display_name(base_node))+3);
                sprintf(buf, "%s[]", display_name(base_node));
                node->disp_name = buf;
            }
            expect(']');
        } else if (consume('.')) {
            check_arg(node, &token->info, CHK_NOT(CHK_STRUCT|CHK_UNION), ".");
            token = cur_token();
            char *name;
            expect_ident(&name, "struct/unionのメンバ名");
            Node *struct_def = tp->node; assert(struct_def!=NULL);    //STRUCT/UNION
            Node *member_def;
            StorageClass sclass = get_storage_class(node->tp);
            if (map_get(struct_def->map, name, (void**)&member_def)==0)
                error_at(&token->info, "struct/union %sに%sは存在しません", struct_def->name, name);
            node->tp = member_def->tp;
            if (sclass != get_storage_class(node->tp)) {
                node->tp = dup_type(node->tp);
                set_storage_class(node->tp, sclass);
            }
            node->offset -= member_def->offset;
            assert(node->offset-sizeof(node->tp)>=0);
            if (node->disp_name==NULL) node->disp_name = node->name;
            char *buf = malloc(strlen(node->disp_name)+strlen(member_def->name)+2);
            sprintf(buf, "%s.%s", node->disp_name, member_def->name);
            node->disp_name = buf;
        } else if (consume(TK_ARROW)) {
            check_arg(node, &token->info, CHK_NOT(CHK_PTR), "->");
            token = cur_token();
            if (!type_is_struct_or_union(tp->ptr_of)) error_at(&token->info, "ここでメンバ名の指定はできません");
            char *name;
            expect_ident(&name, "struct/unionのメンバ名");
            Node *struct_def = tp->ptr_of->node; assert(struct_def!=NULL);    //STRUCT/UNION
            Node *member_def;
            StorageClass sclass = get_storage_class(node->tp);
            if (map_get(struct_def->map, name, (void**)&member_def)==0) error_at(&token->info, "struct/union %sに%sは存在しません", struct_def->name, name);
            node->tp = new_type_ptr(member_def->tp);
            if (sclass != get_storage_class(node->tp)) {
                node->tp = dup_type(node->tp);
                set_storage_class(node->tp, sclass);
            }
            assert(node->offset-sizeof(node->tp)>=0);
            if (node->disp_name==NULL) node->disp_name = node->name;
            assert(node->disp_name);
            char *buf = malloc(strlen(node->disp_name)+strlen(member_def->name)+3);
            sprintf(buf, "%s->%s", node->disp_name, member_def->name);
            node->disp_name = buf;
            node = new_node(ND_INDIRECT, NULL, node, member_def->tp, token);
            node->name = node->rhs->name;
            node->offset = -member_def->offset;
            if (display_name(node->rhs)) {
                char *buf = malloc(strlen(display_name(node->rhs))+2);
                sprintf(buf, "*%s", display_name(node->rhs));
                node->disp_name = buf;
            }
            //dump_node(node,__func__);
        } else if (consume(TK_INC)) {
            check_arg(node, &token->info, CHK_STRUCT|CHK_UNION|CHK_ARRAY|CHK_CONST, "++");
            node = new_node(ND_INC, node, NULL, node->tp, token);
            copy_node_name(node, node->lhs);
        } else if (consume(TK_DEC)) {
            check_arg(node, &token->info, CHK_STRUCT|CHK_UNION|CHK_ARRAY|CHK_CONST, "--");
            node = new_node(ND_DEC, node, NULL, node->tp, token);
            copy_node_name(node, node->lhs);
        } else {
            break;
        }
    }
    return node;
}

//終端記号：数値、識別子、カッコ
static Node *primary_expression(void) {
    Node *node;
    long val;
    char *name;
    String string, string2;
    Token *token = cur_token();
    if (consume('(')) {
        node = expression();
        expect(')');
    } else if (consume_num(&val)) {
        node = new_node_num(val, token);
    } else if (consume_string(&string)) {
        while (consume_string(&string2)) {
            int new_size = string.size + string2.size - 1;
            string.buf = realloc(string.buf, new_size);
            memcpy(string.buf+string.size-1, string2.buf, string2.size);
            string.size = new_size;
        }
        node = new_node_string(&string, token);
    } else if (consume_ident(&name)) {
        //すでに出現済みであればその参照に決まる(ND_LOCAL_VAR/ND_GLOBAL_VAR/ND_ENUMなど)
        node = new_node_ident(name, token);
    } else {
        error_at(&token->info, "終端記号でないトークンです");
    }

    return node;
}
