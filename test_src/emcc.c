typedef enum {
    ST_ERR=0,
    ST_OK=1,
    ST_WARN=2,
}Status;

typedef struct {
    void **data;
    int capacity;
    int len;
} Vector;

typedef struct {
    Vector *keys;
    Vector *vals;
} Map;

//トークン ------------------------------------------
typedef enum {
    TK_NUM = 256,   //整数トークン
    TK_STRING,      //文字列
    TK_IDENT,       //識別子
    //ここから型
    TK_VOID,        //void
    TK_BOOL,        //_Bool
    TK_CHAR,        //char
    TK_SHORT,       //short
    TK_INT,         //int
    TK_LONG,        //long
//  TK_FLOAT,       //float
//  TK_DOUBLE,      //double
    TK_ENUM,        //enum
    TK_STRUCT,      //struct
    TK_UNION,       //union
    TK_TYPEOF,      //typeof（非標準）
    TK_SIGNED,      //signed
    TK_UNSIGNED,    //unsigned
    TK_VOLATILE,    //type_qualifier
    TK_RESTRICT,    //type_qualifier
    TK_ATOMIC,      //type_qualifier
    TK_CONST,       //type_qualifier
    TK_INLINE,      //function_specifier
    TK_NORETURN,    //function_specifier
    TK_AUTO,        //storage_class
    TK_REGISTER,    //storage_class
    TK_STATIC,      //storage_class
    TK_EXTERN,      //storage_class
    TK_TYPEDEF,     //storage_class
    //ここまで型
    TK_ARROW,       // ->
    TK_INC,         // ++
    TK_DEC,         // --
    TK_EQ,          // ==
    TK_NE,          // !=
    TK_LE,          // <=
    TK_GE,          // >=
    TK_LAND,        // &&
    TK_LOR,         // ||
    TK_SHIFTR,      // >>
    TK_SHIFTL,      // <<
    TK_PLUS_ASSIGN, // +=
    TK_MINUS_ASSIGN,// -=
    TK_MUL_ASSIGN,  // *=
    TK_DIV_ASSIGN,  // /=
    TK_MOD_ASSIGN,  // %=
    TK_SHIFTR_ASSIGN,   //>>=
    TK_SHIFTL_ASSIGN,   //<<=
    TK_AND_ASSIGN,  // &=
    TK_XOR_ASSIGN,  // ^=
    TK_OR_ASSIGN,   // |=
    TK_GOTO,        //goto
    TK_CONTINUE,    //continue
    TK_BREAK,       //break
    TK_RETURN,      //return
    TK_IF,          //if
    TK_ELSE,        //else
    TK_SWITCH,
    TK_CASE,
    TK_DEFAULT,
    TK_WHILE,       //while
    TK_DO,          //do
    TK_FOR,         //for
    TK_SIZEOF,      //sizeof
    TK_ALIGNOF,     //_Alignof (C11)
    TK_SASSERT,     //_Static_assert
    TK_3DOTS,       // ...
    TK_EOF,         //入力の終わり
} TKtype;

//文字列。NULL終端でなくてもよい。
typedef struct String {
    char *buf;
    int size;
} String;

typedef struct {
    TKtype type;    //トークンの型
    long val;       //typeがTK_TOKENの場合、値
    char is_U;      //123U
    char is_L;      //123L
    union {
    char *ident;    //typeがTK_IDENTの場合、その文字列
    String string;  //typeがTK_STRINGの場合、その文字列
    };
    char *input;    //トークン文字列（エラーメッセージ用）
} Token;

typedef enum {
    ND_UNDEF = 0,
    ND_NOT   = '!',
    ND_MOD   = '%',
    ND_AND   = '&',
    ND_MUL   = '*',
    ND_PLUS  = '+',
    ND_MINUS = '-',
    ND_DIV   = '/',
    ND_LT    = '<',
    ND_ASSIGN= '=',
    ND_GT    = '>',
    ND_XOR   = '^',
    ND_OR    = '|',
    ND_BNOT  = '~',
    ND_NUM,         //整数のノードの型
    ND_STRING,      //文字列リテラル    name=文字列, val=string_vecのindex
    ND_TYPE_DECL,   //型の宣言　例：int; enum ABC {A,B,C}; enum ABC; struct ST {...}; struct ST; 
    ND_IDENT,       //IDENT:中間的なタイプであり、最終的にND_LOCAL_VARなどに置き換わる
    ND_ENUM_DEF,    //enum定義          name=enum名/NULL, lst=node(ND_ENUM)/NULL
    ND_ENUM,        //enum要素          name=要素名, val=値, lhs=node(ND_ENUN_DEF)
    ND_TYPEDEF,     //typedef           name=typedef名, tp->sclass=SC_TYPEDEF
    ND_STRUCT_DEF,  //struct            name=struct名/NULL, lst=node(ND_MEMBER_DEF)
    ND_UNION_DEF,   //union             name=union名/NULL, lst=node(ND_MEMBER_DEF)
    ND_LOCAL_VAR,   //ローカル変数の参照    name=変数名、offset=RBPからのオフセット(AUTO)/global_index(STATIC)
    ND_GLOBAL_VAR,  //グローバル変数の参照  name=変数名、offset=0
    ND_CAST,        //キャスト
    ND_INC,         // a++
    ND_DEC,         // a--
    ND_INC_PRE,     // ++a
    ND_DEC_PRE,     // --a
    ND_NEG,         // -a
    ND_INDIRECT,    // * / ->（間接参照）
    ND_ADDRESS,     // &（アドレス演算子）
    ND_EQ,          // ==
    ND_NE,          // !=
    ND_LE,          // <=, >=
    ND_LAND,        // &&
    ND_LOR,         // ||
    ND_SHIFTR,      // >>
    ND_SHIFTL,      // <<
    ND_TRI_COND,    // A ? B : C（三項演算子）
    ND_PLUS_ASSIGN, // +=
    ND_MINUS_ASSIGN,// -=
    ND_MUL_ASSIGN,  // *=
    ND_DIV_ASSIGN,  // /=
    ND_MOD_ASSIGN,  // %=
    ND_SHIFTR_ASSIGN,   // >>=
    ND_SHIFTL_ASSIGN,   // <<=
    ND_AND_ASSIGN,  // &=
    ND_XOR_ASSIGN,  // ^=
    ND_OR_ASSIGN,   // |=
    ND_LOCAL_VAR_DEF,   //int A=B;      name=A, rhs=Node（"A=B"の形式の初期化式、初期値がない場合はNULL）
                        //              offset=RBPからのオフセット(AUTO)/index=global_index(STATIC)
    ND_GLOBAL_VAR_DEF,  //int A=B;      同上、offset=0
    ND_MEMBER_DEF,  // struct {int A;}; name=A
    ND_IF,          // if(A)B else C    lhs->lhs=A, lhs->rhs=B, rhs=C
    ND_SWITCH,      // switch(A)B       lhs=A, rhs=B, lst=node(ND_CASE,ND_DEFAULT)
    ND_LABEL,       // label:B          name=label, rhs=B
    ND_CASE,        // case A:B;        val=A(constant), lhs=A, rhs=B, name="case:%ld"
    ND_DEFAULT,     // default:A        rhs=A, name="default"
    ND_WHILE,       // while(A)B        lhs=A, rhs=B
    ND_DO,          // do A while(B);   lhs=A, rhs=B
    ND_FOR,         // for(A;B;C)D      lhs->lhs=A, lhs->rhs=B, rhs->lhs=C, rhs->rhs=D
    ND_GOTO,        // goto label;      name=label
    ND_CONTINUE,
    ND_BREAK,
    ND_RETURN,      // rhs=expression
    ND_BLOCK,       //{ }               lst=ノード(declaration/statement)
    ND_LIST,        //コンマリスト(a,b,c)
    ND_INIT_LIST,   //初期値リスト{a,b,c}
    ND_FUNC_CALL,   //関数コール        name=関数名, lhs=引数リスト(ND_LIST)/NULL, 
                    //                 rhs=ND_FUNC_DEF|DECL/ND_LOCAL|GLOBAL_VAR_DEF(FUNC)
    ND_FUNC_DEF,    //関数定義          lhs=引数リスト(ND_LIST), rhs=ブロック(ND_BLOCK：関数本体)
    ND_FUNC_DECL,   //関数宣言          lhs=引数リスト(ND_LIST)
    ND_VARARGS,     //...
    ND_EMPTY,       //空のノード
} NDtype;

typedef enum {
    VOID = 1,
    BOOL,           //ここからtype_is_integer
    CHAR,
    SHORT,
    INT,
    LONG,
    LONGLONG,
    ENUM,           //ここまでtype_is_integer
    FLOAT,
    DOUBLE,
    LONGDOUBLE,
    STRUCT,
    UNION,
    PTR,
    ARRAY,
    VARARGS,        //...
    FUNC,           //関数
    NEST,           //ネストした型宣言処理の一時的なデータ構造でのみ使用する。他のメンバーは未使用。
} TPType;

typedef enum {
    SC_UNDEF,
    SC_TYPEDEF,     //便宜的にここに入っている
    SC_AUTO,
    SC_REGISTER,
    SC_STATIC,
    SC_EXTERN,
} StorageClass;

typedef struct Type Type;
typedef struct Node Node;
typedef Vector Stack;

struct Type {
    TPType          type;
    char            is_unsigned;    //unsigned型
    char            is_const;
    StorageClass    tmp_sclass;
    Type            *ptr_of;        //typeがPTR,ARRAY,FUNCの場合次のType
    Node            *node;          //typeがFUNCの場合のND_FUNC_DEFのノード
                                    //typeがENUMの場合のND_ENUM_DEFのノード
    long            array_size;     //typeがARRAYの場合の配列サイズ。未定義の場合は-1
};

struct Node {
    NDtype type;    //nodeの型：演算子、ND_INDENTなど
    char unused;    //無効（重複した宣言など：コード生成時には無視する）
    int offset;     //auto変数：ベースアドレスからのoffset：(ベースアドレス-offset)が実際のアドレスになる
                    //typeがND_MEMBER_DEFの場合の先頭アドレスからのoffset。UNIONの場合は常に0
    int index;      //static変数：識別用index（global_index）
                    //typeがND_STRINGの場合のstring_vecのインデックス
    long val;       //typeがND_NUMの場合の値
                    //typeがND_(STRUCT/UNION/LOCAL_VAR|GLOBAL_VAR)_DEFの場合のサイズ(sizeof)
                    //typeがND_MEMBER_DEFの場合のパディングを含めたサイズ
    Node *lhs;
    Node *rhs;
    Vector *lst;    //typeがND_BLOCKの場合のstatementのリスト
                    //typeがND_LISTの場合のassignmentのリスト
                    //typeがND_STRUCT/UNION_DEFの場合のメンバのリスト
    Map *map;       //typeがND_SWITCHの場合のND_CASEのマップ: key=node->val, val=node(ND_CASE)
                    //typeがND_STRUCT/UNION_DEFの場合のND_MEMBER_DEFのマップ: key=node->name, val=node
    union {
    char *name;     //typeがND_LOCAL|GLOBAL_VAR[_DEF]の場合の変数名
                    //typeがND_FUNC_CALL|DEF|DECLの場合の関数名
    String string;  //typeがND_STRINGの場合の文字列（NULL終端でなくてもよい）
    };
    char *disp_name;//nameの代わりの表示名(構造体のメンバ名アクセス:st.a)
    Type *tp;       //型情報
    char *input;    //トークン文字列（エラーメッセージ用）。Token.inputと同じ。
};

typedef struct {
    char    *func_name;     //関数名
    Node    *node;          //ND_FUNC_DEFのnode
    Type    *tp;            //関数の型(常にFUNC)
    Map     *symbol_map;    //通常の識別子：key=name, val=Node(ND_LOCAL_VAR_DEFなど)
    Map     *tagname_map;   //タグ名：key=name, val=Node(ND_ENUN_DEFなど)
    Map     *label_map;     //ラベル：key=name, val=Node(ND_LABEL)
    int     var_stack_size; //ローカル変数のために必要となるスタックサイズ（offset）
} Funcdef;

typedef enum {
    ERC_CONTINUE,
    ERC_EXIT,
    ERC_LONGJMP,
    ERC_ABORT,
} ErCtrl;
