#define _PARSE_C_

#include "9cc.h"

/*  文法：
- A.2.4 External Definitions  http://port70.net/~nsz/c/c11/n1570.html#A.2.4
    translation_unit        = external_declaration*
    external_declaration    = function_definition | declaration
    function_definition     = declaration_specifiers declarator compound_statement
- A.2.2 Declarations          http://port70.net/~nsz/c/c11/n1570.html#A.2.2
    declaration             = declaration_specifiers ( init_declarator ( "," init_declarator )* )? ";"
    declaration_specifiers  = "typeof" "(" identifier ")"
                            | type_specifier          declaration_specifiers*
                            | storage_class_specifier declaration_specifiers*
                            | type_qualifier          declaration_specifiers*
    init_declarator         = declarator ( "=" initializer )?
    storage_class_specifier = "typedef" | "static" | "extern" | "auto" | "register"
    type_specifier          = "void" | "_Bool" | "char" | "short" | "int" | "long" | "signed" | "unsigned"
                            | enum_specifier | typedef_name
    enum_specifier          = "enum" identifier? "{" enumerator ( "," enumerator )* ","? "}"
                            | "enum" identifier
    enumerator              = enumeration_constant ( "=" constant-expression )?
    type_qualifier          = "const"
    declarator              = pointer? direct_declarator
    direct_declarator       = identifier | "(" declarator ")"
                            | direct_declarator "[" assignment_expression? "]"
                            | direct_declarator "(" parameter_type_list? ")"    //関数
    parameter_type_list     = parameter_declaration ( "," parameter_declaration )* ( "," "..." )?
    parameter_declaration   = declaration_specifiers ( declarator | abstract_declarator )?
    initializer             = assignment_expression
                            | "{" init_list "}"
                            | "{" init_list "," "}"
    init_list               = initializer
                            | init_list "," initializer
    //配列のサイズは定数の場合のみサポートする
    array_def               = "[" constant_expression? "]" ( "[" constant_expression "]" )*
    pointer                 = ( "*" type_qualifier* )*
    type_name               = "typeof" "(" identifier ")"
                            | type_specifier abstract_declarator*
                            | type_qualifier abstract_declarator*
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
                            | unary_expression ( "=" | "+=" | "-=" ) assignment_expression
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
                            | "_Alignof" "(" type_name ")"
    postfix_expression      = primary_expression 
                            | primary_expression "[" expression "]"
                            | primary_expression "(" assignment_expression? ( "," assignment_expression )* ")"
                            | primary_expression "++"
                            | primary_expression "--"
    primary_expression      = num
                            | string
                            | identifier
                            |  "(" expression ")"
*/
static Node *external_declaration(void);
static Node *function_definition(Type *tp, char *name);
static Node *declaration(Type *tp, char *name);
static Node *init_declarator(Type *decl_spec, Type *tp, char *name);
static Node *declarator(Type *decl_spec, Type *tp, char *name);
static Node *direct_declarator(Type *tp, char *name);
static Node *parameter_type_list(void);
static Node *parameter_declaration(void);
static Node *initializer(void);
static Node *init_list(void);
static Type *array_def(Type *tp);
static Type *pointer(Type *tp);
static Type *type_name(void);
static Type *abstract_declarator(Type *tp);
static Type *direct_abstract_declarator(Type *tp);
static Type *declaration_specifiers(void);
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

//    external_declaration    = function_definition | declaration
//    function_definition     = declaration_specifiers declarator compound_statement
//    declaration             = declaration_specifiers init_declarator ( "," init_declarator )* ";"
static Node *external_declaration(void) {
    Node *node;
    Type *tp;
    char *name;
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
        tp = declaration_specifiers();
        if (consume(';')) {
            node = new_node(ND_TYPE_DECL, NULL, NULL, tp, input_str());
            return node;
        }
        tp = pointer(tp);
        if (token_is('(')) {    //int * ()
            node = declaration(tp, NULL);
        } else if (!consume_ident(&name)) {
            error_at(input_str(), "型名の後に識別名がありません");
        } else if (token_is('(')) {
            node = function_definition(tp, name);
        } else {
            node = declaration(tp, name);
        }
    } else if (consume(';')) {
        //仕様書に記載はない？が、空の ; を読み飛ばす。
    } else {
        error_at(input_str(), "関数・変数の定義がありません");
    }
    return node;
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
        node->rhs = compound_statement(1);
        map_put(funcdef_map, node->name, cur_funcdef);
        stack_pop(symbol_stack);
        stack_pop(tagname_stack);
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
//    init_declarator         = declarator ( "=" initializer )?
//    declarator              = pointer? direct_declarator
//declaration_specifiers, pointer, identifierまで先読み済み
static Node *declaration(Type *tp, char *name) {
    Node *node;
    Type *decl_spec;

    if (tp==NULL) {
        //declaration_specifiers, pointer, identifierを先読みしていなければdecl_specを読む
        decl_spec = declaration_specifiers();
        if (consume(';')) {
            node = new_node(ND_TYPE_DECL, NULL, NULL, decl_spec, input_str());
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

    node = init_declarator(decl_spec, tp, name);

    if (consume(',')) {
        Node *last_node;
        node = new_node_list(node, node->input);
        Vector *lists = node->lst;
        vec_push(lists, last_node=init_declarator(decl_spec, NULL, NULL));
        while (consume(',')) {
            vec_push(lists, last_node=init_declarator(decl_spec, NULL, NULL));
        }
        node->tp = last_node->tp;
    }
    expect(';');
    return node;
}

//    init_declarator         = declarator ( "=" initializer )?
//    declarator              = pointer? direct_declarator
//declaration_specifiers, pointer, identifierまで先読みの可能性あり
static Node *init_declarator(Type *decl_spec, Type *tp, char *name) {
    Node *node, *rhs=NULL;
    char *input = input_str();

    node = declarator(decl_spec, tp, name);
    tp   = node->tp;
    name = node->name;

    if (type_is_typedef(tp)) {
        node->type = ND_TYPEDEF;
        regist_var_def(node);
        return node;
    }

    //初期値: rhsに初期値を設定する
    if (consume('=')) {
        if (type_is_extern(tp)) error_at(input, "extern変数は初期化できません");
        //node->rhsに変数=初期値の形のノードを設定する。->そのまま初期値設定のコード生成に用いる
        rhs = initializer();
        long val;
        if (tp->type==ARRAY) {  //左辺が配列
            if (rhs->tp->type==ARRAY) {
                //文字列リテラルで初期化できるのはcharの配列だけ
                if (rhs->type==ND_STRING && tp->ptr_of->type!=CHAR)
                    error_at(rhs->input, "%sを文字列リテラルで初期化できません", get_type_str(tp));
                if (tp->array_size<0) {
                    tp->array_size = rhs->tp->array_size;
                }
            } else if (rhs->type==ND_LIST) {
                if (tp->array_size<0) {
                    tp->array_size = rhs->lst->len;
                }
            } else {
                error_at(rhs->input, "配列の初期値が配列形式になっていません");
            }
        }
        if (node_is_const(rhs, &val)) {
            rhs = new_node_num(val, rhs->input);
        }
    }

    if (rhs) node->rhs = new_node('=', NULL, rhs, tp, input);
    //初期値により配列のサイズが決まるケースがあるので、regist_var_def()は初期値の確定後に行う必要あり
    regist_var_def(node);   //ND_UNDEF -> ND_(LOCAL|GLOBAL)_VAR_DEFに確定する。ND_FUNC_DECLはそのまま
    if (rhs) node->rhs->lhs = new_node_ident(name, input);  //=の左辺

    //初期値のないサイズ未定義のARRAYはエラー
    //externの場合はOK
    if (node->tp->type==ARRAY && node->tp->array_size<0 && !type_is_extern(node->tp) &&
        (node->rhs==NULL || node->rhs->type!='='))
        error_at(input_str(), "配列のサイズが未定義です");

    //グローバルスカラー変数の初期値は定数または固定アドレスでなければならない
    if ((node->type==ND_GLOBAL_VAR_DEF || node_is_local_static_var(node)) && 
        node->tp->type!=ARRAY && node->rhs &&
        node->rhs->rhs->type!=ND_STRING) {  //文字列リテラルはここではチェックしない
        long val=0;
        Node *var=NULL;
        if (!node_is_const_or_address(node->rhs->rhs, &val, &var)) {
            error_at(node->rhs->rhs->input, "%s変数の初期値が定数ではありません", 
                     node->type==ND_GLOBAL_VAR_DEF?"グローバル":"静的ローカル");
        }
        if (var) {
            if (val) {
                node->rhs->rhs = new_node('+', var, new_node_num(val, input), var->tp, input);
            } else {
                node->rhs->rhs = var;
            }
        }
    }

    if (node->type==ND_LOCAL_VAR_DEF && type_is_static(node->tp)) {
        char *name = node->name;
        //ローカルstatic変数は初期値を外してreturnする。初期化はvarderのリストから実施される。
        node = new_node(ND_LOCAL_VAR_DEF, NULL, NULL, node->tp, node->input);
        node->name = name;
    }
    return node;
}

//    declarator              = pointer* direct_declarator
//    direct_declarator       = identifier | "(" declarator ")"
//                            = direct_declarator "[" assignment_expression? "]"
//                            | direct_abstract_declarator? "(" parameter_type_list? ")"  //関数宣言
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
static Node *direct_declarator(Type *tp, char *name) {
    Node *node=NULL, *lhs=NULL;
    Type *nest_tp=NULL;
    char *input = input_str();
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
            node->lhs = parameter_type_list();
            Type *p = nest_tp;
            for (;;) {
                if (p->type==PTR) {
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
            node->tp = tp = nest_tp;
        } else {
            if (cur_funcdef && type_is_static(tp)) {
                error_at(input, "ブロック内のstatic関数");
            }
            node = new_node_func(name, tp, input);
            node->lhs = parameter_type_list();
            if (org_funcdef) cur_funcdef = org_funcdef;
        }
        tp->node = node;
        expect(')');
        if (node->type==ND_FUNC_DECL) regist_func(node, 1);
        return node;
    }
    
    if (token_is('[')) tp = array_def(tp);
    node = new_node(ND_UNDEF, lhs, NULL, tp, NULL);
    node->name = name;
    node->input = input;
    return node;
}

//    parameter_type_list     = parameter_declaration ( "," parameter_declaration )* ( "," "..." )?
//    parameter_declaration   = declaration_specifiers ( declarator | abstract_declarator )?
static Node *parameter_type_list(void) {
    Node *node = new_node_list(NULL, input_str());
    Node *last_node;
    char *vararg_ptr = NULL;
    if (!token_is_type_spec()) return node; //空のリスト

    vec_push(node->lst, last_node=parameter_declaration());
    while (consume(',')) {
        char *input = input_str();
        if (token_is('}')){
            break;
        } else if (consume(TK_3DOTS)) { //...（可変長引数）
            last_node = new_node(ND_VARARGS, NULL, NULL, new_type(VARARGS, 0), input);
            vararg_ptr = input;
        } else {
            last_node = parameter_declaration();
            last_node->input = input;
        }
        vec_push(node->lst, last_node);
    }
    if (vararg_ptr && last_node->type!=ND_VARARGS)
        error_at(vararg_ptr, "...の位置が不正です");
    return node;
}
static Node *parameter_declaration(void) {
    Node *node;
    char *name, *input = input_str();
    Type *tp = declaration_specifiers();
    tp = pointer(tp);
    if (consume_ident(&name)) {
        node = declarator(tp, tp, name);
        regist_var_def(node);
    } else {
        tp = abstract_declarator(tp);
        node = new_node_var_def(NULL, tp, input);
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
    Node *node = new_node_list(NULL, input_str());
    Node *last_node;

    vec_push(node->lst, last_node=initializer());
    while (consume(',') && !token_is('}')) {
        vec_push(node->lst, last_node=initializer());
    }
    node->tp = last_node->tp;

    return node;
}

//    配列のサイズは定数の場合のみサポートする
//    array_def               = "[" constant_expression? "]" ( "[" constant_expression "]" )*
static Type *array_def(Type *tp) {
    Node *node;
    Type *ret_tp = tp;
    char *input;
    // int *a[10][2][3]
    if (consume('[')) {
        if (consume(']')) { //char *argv[];
            tp = new_type_array(tp, -1);    //最初だけ省略できる（初期化が必要）
        } else {
            input = input_str();
            node = constant_expression();
            if (node->val==0) error_at(input, "配列のサイズが0です");
            tp = new_type_array(tp, node->val);
            expect(']'); 
        }
        ret_tp = tp;
        // ret_tp=tp=ARRAY[10] -> PTR -> INT 
    }

    while (consume('[')) {
        input = input_str();
        node = constant_expression();
        if (node->val==0) error_at(input, "配列のサイズが0です");
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
    int is_const = 0;
    if (tp->type==CONST) {
        is_const = 1;
        tp = tp->ptr_of;
    }
    while (consume('*')) {
        tp = new_type_ptr(tp);
        if (is_const) tp->is_const = 1;
        is_const = 0;
        while (consume(TK_CONST)) is_const = 1;
    }
    if (is_const) {
        Type *tmp = tp;
        while (tmp->ptr_of) tmp = tmp->ptr_of;
        tmp->is_const = 1;       
    }

    return tp;
}

//    type_name               = "typeof" "(" identifier ")"
//                            | type_specifier abstract_declarator*
//                            | type_qualifier abstract_declarator*
//    abstract_declarator     = pointer? direct_abstract_declarator
//    direct_abstract_declarator = "(" abstract_declarator ")"
//                            | direct_abstract_declarator? "[" assignment_expression "]"
//                            | direct_abstract_declarator? "(" parameter_type_list? ")"  //関数
static Type *type_name(void) {
    Type *tp = declaration_specifiers();
    if (tp->sclass!=SC_UNDEF) error_at(input_str(), "storage classは指定できません");
    tp = abstract_declarator(tp);
    return tp;
}
static Type *abstract_declarator(Type *tp) {
    tp = pointer(tp);
    tp = direct_abstract_declarator(tp);
    return tp;
}
static Type *direct_abstract_declarator(Type *tp) {
    if (consume('(')){
        tp = abstract_declarator(tp);
        expect(')');
    }
    if (token_is('[')) tp = array_def(tp);
    return tp;
}

//    declaration_specifiers  = "typeof" "(" identifier ")"
//                            | type_specifier         declaration_specifiers*
//                            | storage_class_specifier declaration_specifiers*
//                            | type_qualifier         declaration_specifiers*
static Type *declaration_specifiers(void) {
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
    char *us_input;
    TPType type = 0;
    StorageClass sclass = SC_UNDEF;
    int is_unsigned = -1;   //-1:未指定、0:signed、1:unsigned
    int pre_const  = 0;     //型の前にconstがある：const int
    int post_const = 0;     //型の後にconstがある：int const

    while (1) {
        char *input = input_str();
        if (consume(TK_VOID)) {
            if (type) error_at(input, "型指定が不正です\n");
            type = VOID;
        } else if (consume(TK_BOOL)) {
            if (type) error_at(input, "型指定が不正です\n");
            type = BOOL;
        } else if (consume(TK_CHAR)) {
            if (type) error_at(input, "型指定が不正です\n");
            type = CHAR;
        } else if (consume(TK_SHORT)) {
            if (type && type!=INT) error_at(input, "型指定が不正です\n");
            type = SHORT;
        } else if (consume(TK_INT)) {
            if (type && type!=SHORT && type!=LONG && type!=LONGLONG) error_at(input, "型指定が不正です\n");
            type = INT;
        } else if (consume(TK_LONG)) {
            if (type==LONG) type = LONGLONG;
            else if (type && type!=INT) error_at(input, "型指定が不正です\n");
            else type = LONG;
        } else if (consume(TK_ENUM)) {
            if (type) error_at(input, "型指定が不正です\n");
            type = ENUM;
            node = enum_specifier();
        } else if (consume_typedef(&node)) {
            if (type) error_at(input, "型指定が不正です\n");
            type = node->tp->type;
        } else if (consume(TK_SIGNED)) {
            if (is_unsigned>=0) error_at(input, "型指定が不正です\n");
            is_unsigned = 0;
            us_input = input;
        } else if (consume(TK_UNSIGNED)) {
            if (is_unsigned>=0) error_at(input, "型指定が不正です\n");
            is_unsigned = 1;
            us_input = input;
        } else if (consume(TK_AUTO)) {
            if (sclass) error_at(input, "strage classが重複しています\n");
            sclass = SC_AUTO;
        } else if (consume(TK_REGISTER)) {
            if (sclass) error_at(input, "strage classが重複しています\n");
            sclass = SC_REGISTER;
        } else if (consume(TK_STATIC)) {
            if (sclass) error_at(input, "strage classが重複しています\n");
            sclass = SC_STATIC;
        } else if (consume(TK_EXTERN)) {
            if (sclass) error_at(input, "strage classが重複しています\n");
            sclass = SC_EXTERN;
        } else if (consume(TK_TYPEDEF)) {
            if (sclass) error_at(input, "strage classが重複しています\n");
            sclass = SC_TYPEDEF;
        } else if (consume(TK_CONST)) {
            if (type == 0) pre_const = 1;
            else           post_const = 1;
        } else {
            if (!type && is_unsigned>=0) type = INT;
            if (!type) error_at(input, "型名がありません\n");
            break;
        }
        if (type==VOID && is_unsigned>=0)
            error_at(input, "void型の指定が不正です\n");
    }

    Type *top_tp;
    if (node) { //enum,typedef_name
        if (is_unsigned>=0) error_at(us_input, "enum/typedef/struct/union名に対してsigned/unsignedの指定はできません\n");
        top_tp = tp = node->tp;
        while (tp->ptr_of) tp = tp->ptr_of; //PTRとARRAYを飛ばす
    } else if (type==BOOL) {
        if (is_unsigned>=0) error_at(us_input, "_Boolに対してsigned/unsignedの指定はできません\n");
        is_unsigned = 1;
        top_tp = tp = new_type(type, is_unsigned);
    } else {
        if (is_unsigned<0) is_unsigned = 0;
        top_tp = tp = new_type(type, is_unsigned);
    }
    tp->sclass = sclass;
    if (pre_const) tp->is_const = 1;
    if (post_const) {   //先頭にCONSTを挿入
        Type *tmp = new_type(CONST, 0);
        tmp->ptr_of = top_tp;
        top_tp = tmp;
    }
    return top_tp;
}

//    enum_specifier          = "enum" identifier? "{" enumerator ( "," enumerator )* ","? "}"
//                            | "enum" identifier
//    enumerator              = enumeration_constant ( "=" constant-expression )?
static Node *enum_specifier(void) {
    Node *node = new_node(ND_ENUM_DEF, NULL, NULL, new_type(ENUM, 0), input_str());
    char *name = NULL;
    if (consume_ident(&name)) {
        node->name = name;
    } else {
        //node->name = "(anonymouse)";
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
    } else if (name==NULL) {
        expect('{');
    }
    node->tp->node = node;
    if (name) regist_tagname(node);
    return node;
}
static Node *enumerator(Node *enum_def, int default_val) {
    Node *node = new_node(ND_ENUM, enum_def, NULL, new_type(INT, 0), input_str());
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
    Node *node;
    char *input = input_str();

    // labeled_statement
    if (token_is(TK_IDENT) && next_token_is(':')) {
        char *name;
        expect_ident(&name, "ラベル名");
        consume(':');
        node = statement();
        node = new_node(ND_LABEL, NULL, node, node->tp, input);
        node->name = name;
        regist_label(node);
        return node;
    } else if (consume(TK_CASE)) {
        node = constant_expression();
        expect(':');
        node = new_node(ND_CASE, node, statement(), node->tp, input);
        node->val = node->lhs->val;
        regist_case(node);
        return node;
    } else if (consume(TK_DEFAULT)) {
        expect(':');
        node = statement();
        node = new_node(ND_DEFAULT, NULL, node, node->tp, input);
        regist_case(node);
        return node;
    } else if (token_is('{')) {
        return compound_statement(0);    //{ ブロック }

    //selection_statement
    } else if (consume(TK_IF)) {        //if(A)B else C
        Node *node_A, *node_B;
        expect('(');
        node_A = expression();
        expect(')');
        input = input_str();
        node_B = statement();
        node = new_node(ND_UNDEF, node_A, node_B, NULL, input); //lhs
        input = input_str();
        if (consume(TK_ELSE)) {
            node = new_node(ND_IF, node, statement(), NULL, input);
        } else {
            node = new_node(ND_IF, node, NULL, NULL, input);
        }
        return node;
    } else if (consume(TK_SWITCH)) {    //switch (A) B
        expect('(');
        node = expression();
        expect(')');
        node = new_node(ND_SWITCH, node, NULL, NULL, input);
        node->map = new_map();
        Node *org_cur_switch = cur_switch;
        cur_switch = node;
        node->rhs = statement();
        cur_switch = org_cur_switch;
        return node;

    //iteration_statement
    } else if (consume(TK_WHILE)) { //while(A)B         lhs=A, rhs=B
        expect('(');
        node = expression();
        expect(')');
        node = new_node(ND_WHILE, node, statement(), NULL, input);
        return node;
    } else if (consume(TK_DO)) {    //do A while(B);    lhs=A, rhs=B
        node = statement();
        if (!consume(TK_WHILE)) error_at(input_str(), "doに対応するwhileがありません");
        expect('(');
        node = new_node(ND_DO, node, expression(), NULL, input);
        expect(')');
    } else if (consume(TK_FOR)) {   //for(A;B;C)D       lhs->lhs=A, lhs->rhs=B, rhs->lhs=C, rhs->rhs=D
        Node *node1, *node2;
        stack_push(symbol_stack, new_map());
        stack_push(tagname_stack, new_map());
        expect('(');
        if (consume(';')) {
            node1 = new_node_empty(input_str());
        } else if (token_is_type_spec()) {
            node1 = declaration(NULL, NULL);
        } else {
            node1 = expression();         //A
            expect(';');
        }
        if (consume(';')) {
            node2 = new_node_empty(input_str());
        } else {
            node2 = expression();         //B
            expect(';');
        }
        node = new_node(ND_UNDEF, node1, node2, NULL, input);       //A,B
        if (consume(')')) {
            node1 = new_node_empty(input_str());
        } else {
            node1 = expression();         //C
            expect(')');
        }
        node2 = new_node(ND_UNDEF, node1, statement(), NULL, input);     //C,D
        node = new_node(ND_FOR, node, node2, NULL, input);   //(A,B),(C,D)
        stack_pop(symbol_stack);
        stack_pop(tagname_stack);
        return node;

    //jump_statement
    } else if (consume(TK_GOTO)) {      //goto label;      name=label
        char *name;
        node = new_node(ND_GOTO, NULL, NULL, NULL, input);
        expect_ident(&name, "ラベル名");
        node->name = name;
        regist_label(node);
    } else if (consume(TK_CONTINUE)) {  //continue
        node = new_node(ND_CONTINUE, NULL, NULL, NULL, input);
    } else if (consume(TK_BREAK)) {     //break
        node = new_node(ND_BREAK, NULL, NULL, NULL, input);
    } else if (consume(TK_RETURN)) {
        if (token_is(';')) {
            node = new_node(ND_RETURN, NULL, NULL, NULL, input);
        } else {
            node = expression();
            node = new_node(ND_RETURN, NULL, node, node->tp, input);
        }
        check_return(node); //戻り値の妥当性をチェック

    //statement_expression
    } else if (consume(';')) {
        return new_node_empty(input_str());
    } else {
        node = expression();
    }

    expect(';');
    return node;
}

static Node *compound_statement(int is_func_body) {
    Node *node;
    assert (consume('{'));

    node = new_node_block(input_str());
    stack_push(symbol_stack, new_map());
    stack_push(tagname_stack, new_map());

    if (is_func_body) {
        //関数の先頭に static const char __func__[]="function name"; 相当の変数を登録する
        char *input = input_str();
        String string = {cur_funcdef->func_name, strlen(cur_funcdef->func_name)+1};
        Node *string_node = new_node_string(&string, input);
        Type *tp = string_node->tp;
        tp->ptr_of->is_const = 1;
        tp->ptr_of->sclass = SC_STATIC;
        Node *func_name_node = new_node_var_def("__func__", tp, input);
        Node *var_node = new_node_ident(func_name_node->name, input);  //=の左辺
        func_name_node->unused = 1;
        func_name_node->rhs = new_node('=', var_node, string_node, tp, input);
    }

    while (!consume('}')) {
        Node *block_item;
        if (token_is_type_spec()) {
            block_item = declaration(NULL, NULL);
        } else {
            block_item = statement();
        }
        vec_push(node->lst, block_item);
    }
    stack_pop(symbol_stack);
    stack_pop(tagname_stack);
    return node;
}

//expr：単なる式またはそのコンマリスト（左結合）
//リストであればリスト(ND_LIST)を作成する
//    expression        = assignment_expression ( "," assignment_expression )* 
static Node *expression(void) {
    char *input = input_str();
    Node *node = assignment_expression();
    Node *last_node = node;
    if (consume(',')) {
        node = new_node_list(node, input);
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
static Node *constant_expression(void) {
    char *input = input_str();
    Node *node = conditional_expression();
    long val;
    if (!node_is_const(node, &val))
        error_at(node->input, "定数式が必要です");
    node = new_node(ND_NUM, NULL, node, node->tp, input);
    node->val = val;
    return node;
}

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
    }
    char *input = input_str();
    if (consume('=')) {
        if (!is_lvalue || node->tp->type==ARRAY) error_at(node->input, "左辺値ではありません");
        rhs = assignment_expression(); 
        if (!(rhs->type==ND_NUM && rhs->val==0) &&  //右辺が0の場合は無条件にOK
            !type_eq_assign(node->tp, rhs->tp))
            warning_at(input, "=の左右の型(%s:%s)が異なります", 
                get_type_str(node->tp), get_type_str(rhs->tp));
        node = new_node('=', node, rhs, node->tp, input); //ND_ASIGN
    } else if (consume(TK_PLUS_ASSIGN)) { //+=
        if (!is_lvalue || node->tp->type==ARRAY) error_at(node->input, "左辺値ではありません");
        rhs = assignment_expression(); 
        if (node_is_ptr(node) && node_is_ptr(rhs))
            error_at(node->input, "ポインタ同士の加算です");
        node = new_node(ND_PLUS_ASSIGN, node, rhs, node->tp, input);
    } else if (consume(TK_MINUS_ASSIGN)) { //-=
        if (!is_lvalue || node->tp->type==ARRAY) error_at(node->input, "左辺値ではありません");
        rhs = assignment_expression(); 
        if (node_is_ptr(rhs)) 
            error_at(node->input, "ポインタによる減算です");
        node = new_node(ND_MINUS_ASSIGN, node, rhs, node->tp, input);
    }
    return node;
}

//三項演算子（右結合）
//    conditional_expression    = logical_OR_expression "?" expression ":" conditional_expression
static Node *conditional_expression(void) {
    Node *node = logical_OR_expression(), *sub_node, *lhs, *rhs;
    char *input = input_str();
    if (consume('?')) {
        lhs = expression();
        expect(':');
        rhs = conditional_expression();
        sub_node = new_node(ND_UNDEF, lhs, rhs, lhs->tp, input);
        node = new_node(ND_TRI_COND, node, sub_node, lhs->tp, node->input);
    }
    return node;
}

//論理和（左結合）
//    logical_OR_expression  = logical_AND_expression ( "||" logical_AND_expression )*
static Node *logical_OR_expression(void) {
    Node *node = logical_AND_expression();
    for (;;) {
        char *input = input_str();
        if (consume(TK_LOR)) {
            node = new_node(ND_LOR, node, logical_AND_expression(), new_type(INT, 0), input);
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
        char *input = input_str();
        if (consume(TK_LAND)) {
            node = new_node(ND_LAND, node, inclusive_OR_expression(), new_type(INT, 0), input);
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
        char *input = input_str();
        if (consume('|')) {
            rhs = exclusive_OR_expression();
            if (type_is_ptr(node->tp) || type_is_ptr(rhs->tp))
                error_at(input, "ポインタに対するビット演算です");
            node = new_node('|', node, rhs, new_type(INT, 0), input);
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
        char *input = input_str();
        if (consume('^')) {
            rhs = AND_expression();
            if (type_is_ptr(node->tp) || type_is_ptr(rhs->tp))
                error_at(input, "ポインタに対するビット演算です");
            node = new_node('^', node, rhs, new_type(INT, 0), input);
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
        char *input = input_str();
        if (consume('&')) {
            rhs = equality_expression();
            if (type_is_ptr(node->tp) || type_is_ptr(rhs->tp))
                error_at(input, "ポインタに対するビット演算です");
            node = new_node('&', node, rhs, new_type(INT, 0), input);
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
        char *input = input_str();
        if (consume(TK_EQ)) {
            node = new_node(ND_EQ, node, relational_expression(), new_type(INT, 0), input);
        } else if (consume(TK_NE)) {
            node = new_node(ND_NE, node, relational_expression(), new_type(INT, 0), input);
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
        char *input = input_str();
        if (consume('<')) {
            node = new_node('<',   node, shift_expression(), new_type(INT, 0), input);
        } else if (consume(TK_LE)) {
            node = new_node(ND_LE, node, shift_expression(), new_type(INT, 0), input);
        } else if (consume('>')) {
            node = new_node('<',   shift_expression(), node, new_type(INT, 0), input);
        } else if (consume(TK_GE)) {
            node = new_node(ND_LE, shift_expression(), node, new_type(INT, 0), input);
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
        char *input = input_str();
        if (consume(TK_SHIFTL)) {
            rhs = additive_expression();
            if (node_is_ptr(node) || node_is_ptr(rhs))
                error_at(node->input, "ポインタによるシフト演算です");
            node = new_node(ND_SHIFTL, node, rhs, node->tp, input);
        } else if (consume(TK_SHIFTR)) {
            rhs = additive_expression();
            if (node_is_ptr(node) || node_is_ptr(rhs))
                error_at(node->input, "ポインタによるシフト演算です");
            node = new_node(ND_SHIFTR, node, rhs, node->tp, input);
        } else {
            break;
        }
    }
    return node;
}

//加減算（左結合）
//    additive_expression         = multiplicative_expression ( "+" multiplicative_expression | "-" multiplicative_expression )*
static Node *additive_expression(void) {
    Node *node = multiplicative_expression(), *rhs;
    for (;;) {
        char *input = input_str();
        if (consume('+')) {
            rhs = multiplicative_expression();
            if (node_is_ptr(node) && node_is_ptr(rhs))
                error_at(node->input, "ポインタ同士の加算です");
            Type *tp = node_is_ptr(node) ? node->tp : rhs->tp;
            node = new_node('+', node, rhs, tp, input);
        } else if (consume('-')) {
            rhs = multiplicative_expression();
            if (node_is_ptr(node) && node_is_ptr(rhs)) {
                if (!type_eq(node->tp, rhs->tp)) 
                    error_at(node->input, "異なるタイプのポインタによる減算です: %s vs %s",
                        get_type_str(node->tp), get_type_str(rhs->tp));
            } else if (!node_is_ptr(node)  && node_is_ptr(rhs)) {
                error_at(node->input, "ポインタによる減算です");
            }
            node = new_node('-', node, rhs, rhs->tp, input);
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
        char *input = input_str();
        if (consume('*')) {
            node = new_node('*', node, cast_expression(), node->tp, input);
        } else if (consume('/')) {
            node = new_node('/', node, cast_expression(), node->tp, input);
        } else if (consume('%')) {
            node = new_node('%', node, cast_expression(), node->tp, input);
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
    char *input = input_str();
    if (token_is('(') && next_token_is_type_spec()) {
        expect('(');
        tp = type_name();
        expect(')');
        node = new_node(ND_CAST, NULL, cast_expression(), tp, input);
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
//                            | "_Alignof" "(" type_name ")"
static Node *unary_expression(void) {
    Node *node;
    char *input = input_str();
    if (consume('+')) {
        node = cast_expression();
    } else if (consume('-')) {
        node = cast_expression();
        node = new_node('-', new_node_num(0, input), node, node->tp, input);
    } else if (consume('!')) {
        node = new_node('!', NULL, cast_expression(), new_type(INT, 0), input);
    } else if (consume('~')) {
        node = cast_expression();
        if (type_is_ptr(node->tp))
            error_at(input, "ポインタに対するビット演算です");
        node = new_node(ND_BNOT, NULL, node, node->tp, input);
    } else if (consume(TK_INC)) {
        node = unary_expression();
        node = new_node(ND_INC_PRE, NULL, node, node->tp, input);
    } else if (consume(TK_DEC)) {
        node = unary_expression();
        node = new_node(ND_DEC_PRE, NULL, node, node->tp, input);
    } else if (consume('*')) {
        node = cast_expression();
        if (!type_is_ptr(node->tp))
            error_at(node->input, "'*'は非ポインタ型(%s)を参照しています", get_type_str(node->tp));
        node = new_node(ND_INDIRECT, NULL, node, node->tp->ptr_of, input);
    } else if (consume('&')) {
        node = cast_expression();
        node = new_node(ND_ADDRESS, NULL, node, new_type_ptr(node->tp), input);
    } else if (consume(TK_SIZEOF)) {
        Type *tp;
        char *input = input_str();
        if (token_is('(')) {
            if (next_token_is_type_spec()) {
                consume('(');
                tp = type_name();
                expect(')');
            } else {
                node = unary_expression();
                tp = node->tp;
            }
        } else {
            node = unary_expression();
            tp = node->tp;
        }
        if (tp->type==ARRAY && tp->array_size<=0) error_at(input, "不完全型のサイズは未定義です");
        node = new_node_num(size_of(tp), input);
    } else if (consume(TK_ALIGNOF)) {
        Type *tp;
        expect('(');
        tp = type_name();
        expect(')');
        node = new_node_num(align_of(tp), input);
    } else {
        node = postfix_expression();
    }
    return node;
}

//後置単項演算子（左結合）
//    postfix_expression      = primary_expression 
//                            | primary_expression "[" expression "]"
//                            | primary_expression "(" assignment_expression? ( "," assignment_expression )* ")"
//                            | primary_expression "++"
//                            | primary_expression "--"
static Node *postfix_expression(void) {
    Node *node = primary_expression();
    for (;;) {
        char *input = input_str();
        Type *tp = node->tp;
        if (consume('(')) {  //関数コール
            #define node_is_func(node) ((node)->tp->type==FUNC||((node)->tp->type==PTR &&(node)->tp->ptr_of->type==FUNC))
            if (node->type!=ND_IDENT && !node_is_func(node))
                error_at(input, "%sに対して関数コールできません", get_type_str(node->tp));
            node = new_node_func_call(node->name, node->input);
            if (!consume(')')) { 
                node->lhs = expression();
                if (node->lhs->type != ND_LIST) {
                    node->lhs = new_node_list(node->lhs, input);
                }
                expect(')');
            }
            check_funccall(node);
        } else if (node->type==ND_IDENT) {
            error_at(node->input, "'%s'は未定義の変数です", node->name);
        } else if (consume(TK_INC)) {
            node = new_node(ND_INC, node, NULL, node->tp, input);
        } else if (consume(TK_DEC)) {
            node = new_node(ND_DEC, node, NULL, node->tp, input);
        } else if (consume('[')) {
            // a[3] => *(a+3)
            // a[3][2] => *(*(a+3)+2)
            input = input_str();
            Node *rhs = expression();
            node = new_node('+', node, rhs, tp ,input);
            tp = node->tp->ptr_of ? node->tp->ptr_of : rhs->tp->ptr_of;
            if (tp==NULL) error_at(input_str(), "ここでは配列を指定できません");
            node = new_node(ND_INDIRECT, NULL, node, tp, input);
            expect(']');
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
    char *input = input_str();
    if (consume('(')) {
        node = expression();
        expect(')');
    } else if (consume_num(&val)) {
        node = new_node_num(val, input);
    } else if (consume_string(&string)) {
        while (consume_string(&string2)) {
            int new_size = string.size + string2.size - 1;
            string.buf = realloc(string.buf, new_size);
            memcpy(string.buf+string.size-1, string2.buf, string2.size);
            string.size = new_size;
        }
        node = new_node_string(&string, input);
    } else if (consume_ident(&name)) {
        //すでに出現済みであればその参照に決まる(ND_LOCAL_VAR/ND_GLOBAL_VAR/ND_ENUMなど)
        node = new_node_ident(name, input);
    } else {
        error_at(input, "終端記号でないトークンです");
    }

    return node;
}
