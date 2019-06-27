#include "9cc.h"

//レジスタ ---------------------------------------------
static char *arg_regs[] = {  //関数の引数で用いるレジスタ
    "rdi", "rsi", "rdx", "rcx", "r8", "r9", NULL
};

static char *regs[][4] = {
//   8bit   16bit  32bit  64bit
    {"al",  "ax",  "eax", "rax" },
    {"dil", "di",  "edi", "rdi" },
    {"sil", "si",  "esi", "rsi" },
    {"dl",  "dx",  "edx", "rdx" },
    {"cl",  "cx",  "ecx", "rcx" },
    {"r8b", "r8w", "r8d", "r8"  },
    {"r9b", "r9w", "r9d", "r9"  },
    {"r10b","r10w","r10d","r10" },
    {"r11b","r11w","r11d","r11" },
    {"bpl", "bp",  "ebp", "rbp" },
    {"spl", "sp",  "esp", "rsp" },
    {NULL,  NULL,  NULL,  NULL },
};

//抽象構文木を下りながらコード生成（スタックマシン）

//ソースコードにコメントを出力する。printfと同じ引数。
static void comment(const char*fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    printf("  # ");
    vprintf(fmt, ap);
}

//乗算をシフトで実現できるときのシフト量
static int shift_size(int val) {
    switch (val) {
        case   1: return 0;
        case   2: return 1;
        case   4: return 2;
        case   8: return 3;
        case  16: return 4;
        case  32: return 5;
        case  64: return 6;
        case 128: return 7;
        case 256: return 8;
    }
    return -1;
}

// レジスタをval倍する。rdxは保存しない。
static void gen_mul_reg(char *reg_name, int val) {
    int shift = shift_size(val);
    if (shift>0) {
        printf("  shl %s, %d\t# %s * %d\n", reg_name, shift, reg_name, val);
    } else if (strcmp(reg_name, "rax")==0) {
        printf("  mov rdx, %d\t# start: %s * %d\n", val, reg_name, val);
        printf("  imul rdx\t# end\n");
    } else {
        printf("  push rax\t# start: %s * %d\n", reg_name, val);
        printf("  mov rax, %s\n", reg_name);
        printf("  mov rdx, %d\n", val);
        printf("  imul rdx\n");
        printf("  mov %s, rax\n", reg_name);
        printf("  pop rax\t# end\n");
    }
}

// レジスタを1/val倍する。rdx,rdiは保存しない。
static void gen_div_reg(char *reg_name, int val) {
    int shift = shift_size(val);
    if (shift>0) {
        printf("  shr %s, %d\t# %s / %d\n", reg_name, shift, reg_name, val);
    } else {
        printf("  mov rdx, 0\n");
        printf("  mov %s, %d\n", reg_name, val);
        printf("  div %s\n", reg_name);
        if (strcmp(reg_name, "rax")!=0) {
            printf("  mov %s, rax\n", reg_name);
        }
    }
}

//型に応じたデータ型名を返す。例：intならDWORD
static char* ptr_name_of_type(const Type *tp) {
    if (tp->type==ARRAY) tp = tp->ptr_of;
    switch (size_of(tp)) {
    case 8: return "QWORD";
    case 4: return "DWORD";
    case 2: return "WORD";
    case 1: return "BYTE";
    default: _ERROR_;
    }
    return NULL;
}

//型に応じたデータ型名（初期値）を返す。
static char* val_name_of_type(const Type *tp) {
    while (tp->type==ARRAY) tp = tp->ptr_of;
    switch (size_of(tp)) {
    case 8: return "quad";
    case 4: return "long";
    case 2: return "value";
    case 1: return "byte";
    default: _ERROR_;
    }
    return NULL;
}

//型に応じたレジスタ名を返す。例：intならrax->eax
static char* reg_name_of_type(const char*reg_name, const Type *tp) {
    int idx;
    while (tp->type==ARRAY) tp = tp->ptr_of;
    switch (size_of(tp)) {
    case 8: idx = 3; break; //32bit
    case 4: idx = 2; break;
    case 2: idx = 1; break;
    case 1: idx = 0; break; //8bit
    default: _ERROR_;
    }
    for (int j=3; j>=0; j--) {
        for (int i=0; regs[i][j]; i++) {
            if (strcmp(reg_name, regs[i][j])==0) return regs[i][idx];
        }
    }
    _ERROR_;
    return NULL;
}

//型に応じたwriteコマンド名を返す。
static char* write_command_of_type(const Type *tp) {
    switch (tp->type) {
    case CHAR:     return "movb";
    case SHORT:    return "mov";
    case INT:      return "mov";
    case LONG:     return "mov";
    case LONGLONG: return "mov";
    case PTR:      return "mov";
    case ARRAY:    return "mov";
    case FUNC:     return "mov";
    default: _ERROR_;
    }
    return NULL;
}

//型に応じたreadコマンド名を返す。
 //movsx: srcで提供されない残りのビットをsrcの符号で埋める
 //movzx: srcで提供されない残りのビットを0で埋める
static char* read_command_of_type(const Type *tp) {
    switch (tp->type) {
    case CHAR:
    case SHORT:
    case INT:
         return tp->is_unsigned ? "movzx" : "movsx";
    case LONG:
    case LONGLONG:
        return "mov";
    case PTR:
    case ARRAY:
    case FUNC:
        return "mov";
    default: _ERROR_;
    }
    return NULL;
}

//[dst]レジスタが指すアドレスにsrcレジスタの値をwriteする。
//型(tp)に応じてsrcレジスタのサイズを調整する。例：intならrax->eax
static void gen_write_reg(const char*dst, const char*src, const Type *tp, const char* comment) {
    printf("  %s [%s], %s", write_command_of_type(tp), dst, reg_name_of_type(src, tp));
    if (comment) printf("\t# %s\n", comment);
    else         printf("\n");
}

//dstレジスタにsrcレジスタが指すアドレスからreadする。
//型(tp)に応じてsrcの修飾子を調整する。例：intならDWORD PTR
static void gen_read_reg(const char*dst, const char*src, const Type *tp, const char* comment) {
    if (tp->type==INT && tp->is_unsigned) {
        printf("  mov %s, DWORD PTR [%s]", reg_name_of_type(dst, tp), src);
    } else if (tp->type==FUNC) {
        printf("  mov %s, OFFSET FLAT:%s", dst, src);
    } else {
        printf("  %s %s, %s PTR [%s]", read_command_of_type(tp), dst, ptr_name_of_type(tp), src);
    }
    if (comment) printf("\t# %s\n", comment);
    else         printf("\n");
}

//array_size個のノードをdata_size単位のバイト列に変換して返す。
//定数でないノードがあればNULLを返す。
char *get_byte_string(Node *node, int array_size, int data_size) {
    int byte_len = array_size * data_size;
    char *byte_string = malloc(byte_len);
    char *p = byte_string;
    long val;
    Node **nodes = (Node**)node->lst->data;
    for (int i=0; i<array_size; i++) {
        if (nodes[i]->type==ND_LIST) {
            _ERROR_;
        } else if (!node_is_const(nodes[i],&val)) return NULL;
        memcpy(p, (char*)(&val), data_size);
        p += data_size;
    }
    return byte_string;
}

//変数のアセンブラ上の名前を返す。
//戻り値は関数内部の静的な領域を指していることがあり、コールする度に上書きされる。
static char *get_asm_var_name(Node *node) {
    static char buf[16];
    char *ret = buf;
    if (node->type==ND_LOCAL_VAR) {
        if (type_is_static(node->tp)) {
            sprintf(buf, "%s.%03d", node->name, node->offset);
        } else {
            sprintf(buf, "rbp-%d", node->offset);
        }
    } else if (node->type==ND_GLOBAL_VAR) {
        ret = node->name;
    } else {
        _NOT_YET_(node);
    }
    return ret;
}

static int gen(Node*node);
static void gen_op2(Node *node);

//ローカル変数の配列の初期化。スタックトップに変数のアドレスが設定されている前提。
static void gen_array_init(Node *node) {
    assert(node->type=='=');
    assert(node->lhs->type==ND_LOCAL_VAR);
    assert(node->lhs->tp->type==ARRAY);
    int array_size  = node->lhs->tp->array_size;  //配列のサイズ
    int data_len;

    if (node->lhs->tp->ptr_of->type==CHAR &&
        node->rhs->type==ND_STRING) {
        //文字列リテラルによる初期化: char a[]="ABC";
        data_len = node->rhs->tp->array_size;
        if (array_size > data_len) array_size = data_len;
        // [rdi++] = [rsi++] を rcx回繰り返す
        printf("  pop rdi\n");
        printf("  lea rsi, BYTE PTR .LC%03ld\n", node->rhs->val);
        printf("  mov rcx, %d\n", array_size);
        printf("  rep movsb\n");
        return;
    }

    assert(node->rhs->type==ND_LIST);
    // char a[]={'A', 'B', 'C', '\0'}
    // int  b[]={1, 2, 4, 8}
    int type_size = size_of(node->lhs->tp->ptr_of);   //左辺の型のサイズ
    data_len = node->rhs->lst->len;
    if (array_size > data_len) array_size = data_len;
    char *data = get_byte_string(node->rhs, array_size, type_size);
    if (data) { //定数のみの初期値
        printf("  pop rdi\n");
        switch (type_size) {
        case 1:
            for (int i=0; i<array_size; i++) {
                printf("  movb BYTE PTR [rdi+%d], %d\n", i*type_size, data[i]);
            }
            break;
        case 2: ;
            short *datas = (short*)data;
            for (int i=0; i<array_size; i++) {
                printf("  mov WORD PTR [rdi+%d], %d\n", i*type_size, datas[i]);
            }
            break;
        case 4: ;
            int *datai = (int*)data;
            for (int i=0; i<array_size; i++) {
                printf("  mov DWORD PTR [rdi+%d], %d\n", i*type_size, datai[i]);
            }
            break;
        case 8: ;
            long *datal = (long*)data;
            for (int i=0; i<array_size; i++) {
                printf("  mov QWORD PTR [rdi+%d], %ld\n", i*type_size, datal[i]);
            }
            break;
        default:
            _ERROR_;
            break;
        }
    } else {    //初期値に定数でない式を含む場合
        Node **nodes = (Node**)node->rhs->lst->data;
        for (int i=0; i<array_size; i++) {
            gen(nodes[i]);
            printf("  pop rax\n");
            printf("  pop rdi\n");
            printf("  %s %s PTR [rdi+%d], %s\n", 
                write_command_of_type(node->lhs->tp->ptr_of),
                ptr_name_of_type(node->lhs->tp->ptr_of), i*type_size,
                reg_name_of_type("rax", node->lhs->tp->ptr_of));
            printf("  push rdi\n");
        }
        printf("  pop rdi\n");
    }
}

//グローバル変数の配列の初期化
static void gen_array_init_global(Node *node) {
    assert(node->type=='=');
    assert(node->lhs->type==ND_GLOBAL_VAR || type_is_static(node->lhs->tp));
    assert(node->lhs->tp->type==ARRAY);
    int array_size  = node->lhs->tp->array_size;  //配列のサイズ
    int data_len;

    if (node->lhs->tp->ptr_of->type==CHAR &&
        node->rhs->type==ND_STRING) {
        //文字列リテラルによる初期化: char a[]="ABC";
        data_len = node->rhs->tp->array_size;
        char *str = (char*)vec_get(string_vec, node->rhs->val);
        if (array_size < data_len) {
            str[array_size] = 0;
            printf("  .ascii \"%s\"\n", str);
        } else {
            printf("  .string \"%s\"\n", str);
        }
        if (array_size > data_len) printf("  .zero %d\n", array_size-data_len);
        return;
    }

    assert(node->rhs->type==ND_LIST);
    // char a[]={'A', 'B', 'C', '\0'}
    // int  b[]={1, 2, 4, 8}
    int type_size = size_of(node->lhs->tp->ptr_of);   //左辺の型のサイズ
    Node **nodes = (Node**)node->rhs->lst->data;
    data_len = node->rhs->lst->len;
    long val;
    Node *var = NULL;
    char *val_name = val_name_of_type(node->lhs->tp);
    for (int i=0; i<array_size && i<data_len; i++) {
        var = NULL;
        //nodes[i]がアドレス+定数の形式になっているかどうかを調べる。
        //varにND_ADDRESS(&var)のノード、valに定数を返す
        if (!node_is_const_or_address(nodes[i], &val, &var))
            error_at(nodes[i]->input, "初期値が定数式ではありません");
        if (var && val) {
            printf("  .%s %s%+ld\n", val_name, var->rhs->name, val*size_of(var->rhs->tp));
        } else if (var) {
            printf("  .%s %s\n", val_name, var->rhs->name);
        } else {
            printf("  .%s %ld\n", val_name, val);
        }
    }
    if (array_size > data_len) printf("  .zero %d\n", (array_size-data_len)*type_size);
}

//式を左辺値として評価し、そのアドレスをPUSHする
static void gen_lval(Node*node) {
    if (node->type == ND_LOCAL_VAR) {   //ローカル変数
        comment("LVALUE:%s (LOCAL:%s)\n", node->name, get_type_str(node->tp));
        printf("  lea rax, [%s]\n", get_asm_var_name(node));
        printf("  push rax\n");
    } else if (node->type == ND_GLOBAL_VAR) {   //グローバル変数
        comment("LVALUE:%s (GLOBAL:%s)\n", node->name, get_type_str(node->tp));
        printf("  lea rax, %s\n", node->name);
        printf("  push rax\n");
    } else if (node->type == ND_INDIRECT) {
        comment("LVALUE:*var\n");
        gen(node->rhs);     //rhsのアドレスを生成する
    } else if (node->type == ND_CAST) {
        comment("LVALUE (%s)var\n", get_type_str(node->tp));
        gen_lval(node->rhs);
    } else {
        error_at(node->input, "アドレスを生成できません");
    }
}

//変数の内容を指定したレジスタにreadする
//スタックは変化しない
static void gen_read_var(Node *node, const char *reg) {
    char cbuf[256];
    sprintf(cbuf, "%s(%s)", node->name, get_type_str(node->tp));
    if (node->type==ND_LOCAL_VAR || node->type==ND_GLOBAL_VAR) {
        gen_read_reg(reg, get_asm_var_name(node), node->tp, cbuf);
    } else {
        gen_lval(node);
        printf("  pop rax\n");  //rhsのアドレス=戻り値
        gen_read_reg(reg, "rax", node->tp, cbuf);
    }
}

//スタックトップをpopして変数にwriteする
//結果は指定したregに残る
static void gen_write_var(Node *node, char *reg) {
    char cbuf[256];
    sprintf(cbuf, "%s(%s)", node->name, get_type_str(node->tp));
    if (node->type==ND_LOCAL_VAR) {
        printf("  pop %s\n", reg);  //writeする値
        gen_write_reg(get_asm_var_name(node), reg, node->tp, cbuf);
    } else {
        gen_lval(node);
        printf("  pop rcx\n");  //変数のアドレス
        printf("  pop %s\n", reg);  //writeする値
        gen_write_reg("rcx", reg, node->tp, cbuf);
    }
}

static int label_cnt = 0;   //ラベル識別用カウンタ

//ステートメントを評価
//結果をスタックに積んだ場合は1、そうでない場合は0を返す
static int gen(Node*node) {
    int ret;
    char buf[20];
    char *label;
    assert(node!=NULL);
    switch (node->type) {
    case ND_NUM:            //数値
        if (node->tp->type==LONG) {
            printf("  mov rax, %ld\t# 0x%lx (%s)\n", node->val, node->val, get_type_str(node->tp));
            printf("  push rax\n");
        } else {
            printf("  push %d\t# 0x%lx (%s)\n", (int)node->val, node->val, get_type_str(node->tp));
        }
        break;
    case ND_STRING:         //文字列リテラル
        sprintf(buf, ".LC%03ld", node->val);
        printf("  lea rax, BYTE PTR %s\n", buf);
        printf("  push rax\n");             //文字列リテラルのアドレスを返す
        break;
    case ND_EMPTY:          //空文
    case ND_GLOBAL_VAR_DEF: //グローバル(extern)変数定義
    case ND_FUNC_DEF:       //関数定義
    case ND_FUNC_DECL:      //関数宣言
        return 0;
    case ND_LOCAL_VAR_DEF:  //ローカル変数定義
        if (node->rhs==NULL) return 0;
        return gen(node->rhs);  //代入
    case ND_LOCAL_VAR:
    case ND_GLOBAL_VAR:     //変数参照
        if (node->tp->type==ARRAY) {
            //アドレスをそのまま返す
            comment("%s_VAR:%s(%s)\n", node->type==ND_LOCAL_VAR?"LOCAL":"GLOBAL", node->name, get_type_str(node->tp));
            gen_lval(node);
        } else {
            gen_read_var(node, "rax");
            printf("  push rax\n");
        }
        break;
    case ND_FUNC_CALL:      //関数コール
    {
        int i=0;
        comment("CALL:%s\t%s\n", node->name, get_type_str(node->tp));
        if (node->lhs) {    //引数
            assert(node->lhs->type==ND_LIST);
            Vector *lists = node->lhs->lst;
            Node **nodes = (Node**)lists->data;
            for (i=0; i < lists->len; i++) {
                comment("ARGLIST[%d]\n", i);
                gen(nodes[i]);  //スタックトップに引数がセットされる
            }
        }
        if (node->rhs==NULL ||          //未定義の関数のコール
            node->rhs->tp->type==FUNC) {//定義済み関数のコール
            for (; i; i--) {
                printf("  pop %s\n", arg_regs[i-1]);
            }
            printf("  mov al, 0\n");
            printf("  call %s\n", node->name);
        } else {                        //関数ポインタのコール
            gen(node->rhs);
            printf("  pop rbx\n");
            for (; i; i--) {
                printf("  pop %s\n", arg_regs[i-1]);
            }
            printf("  mov al, 0\n");
            printf("  call rbx\n");
        }
        printf("  push rax\n");
        break;
    }
    case ND_CAST:           //キャスト
        comment("CAST (%s)%s\n", get_type_str(node->tp), get_type_str(node->rhs->tp));
        gen(node->rhs);
        break;
    case ND_LABEL:
        printf(".L%s:\n", node->name);
        return gen(node->rhs);
    case ND_GOTO:
    {
        Node *tmp;
        if (map_get(cur_funcdef->label_map, node->name, (void**)&tmp)!=0) {
            if (tmp->type!=ND_LABEL) error_at(node->input, "ラベルが未定義です");
        }
        printf("  jmp .L%s\n", node->name);
        return 0;
    }
    case ND_RETURN:         //return
        if (node->rhs) {
            comment("RETURN (%s)\n", get_type_str(node->tp));
            gen(node->rhs);
            printf("  pop rax\t# RETURN VALUE\n");
        } else {
            comment("RETURN\n");
        }
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
        return 0;
    case ND_IF:             //if (A) B [else C]
    {
        int cnt = label_cnt++;
        comment("IF(A) B [else C]\n");
        ret = gen(node->lhs->lhs); //A
        assert(ret);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        if (node->rhs) {    //elseあり
            printf("  je .LIfElse%03d\n", cnt);
        } else {
            printf("  je .LIfEnd%03d\n", cnt);
        }
        if (gen(node->lhs->rhs)) //B
            printf("  pop rax\n");
        if (node->rhs) {    //elseあり
            printf("  jmp .LIfEnd%03d\n", cnt);
            printf(".LIfElse%03d:\n", cnt);
            if (gen(node->rhs)) //C
                printf("  pop rax\n");
        }
        printf(".LIfEnd%03d:\n", cnt);
        return 0;
    }
    case ND_WHILE:          //while (A) B
    {
        int cnt = label_cnt++;
        char b_label[20]; sprintf(b_label, ".LWhileEnd%03d",  cnt); stack_push(break_stack, b_label);
        char c_label[20]; sprintf(c_label, ".LWhileBody%03d", cnt); stack_push(continue_stack, c_label);
        comment("WHILE(A)B\n");
        printf(".LWhileBegin%03d:\n", cnt);
        ret = gen(node->lhs); //A
        assert(ret);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .LWhileEnd%03d\n", cnt);
        if (gen(node->rhs)) //B
            printf("  pop rax\n");
        printf("  jmp .LWhileBegin%03d\n", cnt);
        printf(".LWhileEnd%03d:\n", cnt);
        stack_pop(break_stack);
        stack_pop(continue_stack);
        return 0;
    }
    case ND_FOR:            //for (A;B;C) D
    {
        int cnt = label_cnt++;
        char b_label[20]; sprintf(b_label, ".LForEnd%03d",  cnt); stack_push(break_stack, b_label);
        char c_label[20]; sprintf(c_label, ".LForNext%03d", cnt); stack_push(continue_stack, c_label);
        comment("FOR(A;B;C)D\n");
        if (gen(node->lhs->lhs))//A
            printf("  pop rax\n");
        printf(".LForBegin%03d:\n", cnt);
        comment("FOR:B\n");
        if (gen(node->lhs->rhs)) {//B
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  jne .LForBody%03d\n", cnt);
            printf("  push rax\n");
            printf("  jmp .LForEnd%03d\n", cnt);
        }
        printf(".LForBody%03d:\n", cnt);
        comment("FOR:D\n");
        if (gen(node->rhs->rhs))    //D
            printf("  pop rax\n");
        printf(".LForNext%03d:\n", cnt);
        comment("FOR:C\n");
        if (gen(node->rhs->lhs))//C
            printf("  pop rax\n");
        printf("  jmp .LForBegin%03d\n", cnt);
        printf(".LForEnd%03d:\n", cnt);
        stack_pop(break_stack);
        stack_pop(continue_stack);
        return 0;
    }
    case ND_BREAK:          //break
        if (break_stack->len==0) error_at(node->input,"ここではbreakを使用できません");
        label = (char*)stack_top(break_stack);
        printf("  jmp %s\t# break\n", label);
        return 0;
    case ND_CONTINUE:       //continue
        if (continue_stack->len==0) error_at(node->input,"ここではcontinueを使用できません");
        label = (char*)stack_top(continue_stack);
        printf("  jmp %s\t# continue\n", label);
        return 0;
    case ND_BLOCK:          //{ ブロック }
    {
        Vector *blocks = node->lst;
        Node **nodes = (Node**)blocks->data;
        for (int i=0; i < blocks->len; i++) {
            comment("BLOCK[%d]\n", i);
            if (gen(nodes[i])) printf("  pop rax\n");
        }
        return 0;
    }
    case ND_LIST:           //コンマリスト
    {
        Vector *lists = node->lst;
        Node **nodes = (Node**)lists->data;
        for (int i=0; i < lists->len; i++) {
            comment("LIST[%d]\n", i);
            if (gen(nodes[i])) printf("  pop rax\n");
        }
        printf("  push rax\n");
        break;
    }
    case ND_ASSIGN:         //代入('=')
        comment("'='\n");
        if (node->lhs->tp->type==ARRAY) {   //ローカル変数の配列の初期値
            //ここに到達するのはローカル変数の配列の初期化のはず
            gen_lval(node->lhs);
            gen_array_init(node);
            return 0;
        } else {
            gen(node->rhs);
            gen_write_var(node->lhs, "rax");
            printf("  push rax\n");
        }
        break;
    case ND_PLUS_ASSIGN:    //+=
    case ND_MINUS_ASSIGN:   //-=
        comment("'A+=B'\n");
        gen_lval(node->lhs);
        printf("  mov rdi, QWORD PTR [rsp]\n");  //lhsのアドレス
        gen_read_reg("rax", "rdi", node->lhs->tp, NULL);    //lhsの値
        printf("  push rax\n"); //lhsの値
        gen(node->rhs);
        printf("  pop rax\n");  //rhsの値
        printf("  pop rdi\n");  //lhsの値
        if (node_is_ptr(node)) {
            gen_mul_reg("rax", size_of(node->tp->ptr_of));
        }
        if (node->type == ND_PLUS_ASSIGN) {
            printf("  add rax, rdi\n");
        } else {
            printf("  sub rdi, rax\n");
            printf("  mov rax, rdi\n");
        }
        printf("  pop rdi\n");  //lhsのアドレス
        gen_write_reg("rdi", "rax", node->lhs->tp, NULL);
        printf("  push rax\n"); //戻り値
        break;
    case ND_INDIRECT:       //*a（間接参照）
        comment("'*A'\n");
        gen(node->rhs);
        if (node->tp->type==ARRAY) {
            //rhsのアドレスをそのまま返す
        } else {
            printf("  pop rax\n");  //rhsの値（アドレス）
            gen_read_reg("rax", "rax", node->tp, NULL);
            printf("  push rax\n");
        }
        break;
    case ND_ADDRESS:        //&a（アドレス演算子）
        comment("'&A'\n");
        gen_lval(node->rhs);
        break;
    case ND_INC_PRE:        //++a
        comment("'++A'\n");
        gen_lval(node->rhs);
        printf("  pop rdi\n");  //rhsのアドレス
        gen_read_reg("rax", "rdi", node->rhs->tp, NULL);
        if (node_is_ptr(node)) {
            printf("  add rax, %ld\n", size_of(node->tp->ptr_of));
        } else {
            printf("  inc rax\n");  //戻り値を設定する前にINC
        }
        gen_write_reg("rdi", "rax", node->rhs->tp, NULL);
        printf("  push rax\n"); //戻り値
        break;
    case ND_DEC_PRE:        //--a
        comment("'--A'\n");
        gen_lval(node->rhs);
        printf("  pop rdi\n");  //rhsのアドレス
        gen_read_reg("rax", "rdi", node->rhs->tp, NULL);
        if (node_is_ptr(node)) {
            printf("  sub rax, %ld\n", size_of(node->tp->ptr_of));
        } else {
            printf("  dec rax\n");  //戻り値を設定する前にDEC
        }
        gen_write_reg("rdi", "rax", node->rhs->tp, NULL);
        printf("  push rax\n"); //戻り値
        break;
    case ND_INC:            //a++
        comment("'A++'\n");
        gen_lval(node->lhs);
        printf("  pop rdi\n");  //lhsのアドレス
        gen_read_reg("rax", "rdi", node->lhs->tp, NULL);
        printf("  push rax\n"); //INCする前に戻り値を設定
        if (node_is_ptr(node)) {
            printf("  add rax, %ld\n", size_of(node->tp->ptr_of));
        } else {
            printf("  inc rax\n");
        }
        gen_write_reg("rdi", "rax", node->lhs->tp, NULL);
        break;
    case ND_DEC:            //a--
        comment("'A--'\n");
        gen_lval(node->lhs);
        printf("  pop rdi\n");  //lhsのアドレス
        gen_read_reg("rax", "rdi", node->lhs->tp, NULL);
        printf("  push rax\n"); //DECする前に戻り値を設定
        if (node_is_ptr(node)) {
            printf("  sub rax, %ld\n", size_of(node->tp->ptr_of));
        } else {
            printf("  dec rax\n");
        }
        gen_write_reg("rdi", "rax", node->lhs->tp, NULL);
        break;
    case ND_NOT:            //否定('!')
        comment("'!'\n");
        gen(node->rhs);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        printf("  push rax\n");
        break;
    case ND_BNOT:           //ビット反転('~')
        comment("'~'\n");
        gen(node->rhs);
        printf("  pop rax\n");
        printf("  not rax\n");
        printf("  push rax\n");
        break;
    case ND_TRI_COND:       //A ? B * C（三項演算）
    {
        int cnt = label_cnt++;
        comment("A ? B : C\n");
        gen(node->lhs);         //A
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .LTriFalse%03d\n", cnt);
        gen(node->rhs->lhs);    //B
        printf("  jmp .LTriEnd%03d\n", cnt);
        printf(".LTriFalse%03d:\n", cnt);
        gen(node->rhs->rhs);    //C
        printf(".LTriEnd%03d:\n", cnt);
        break;
    }
    default:                //2項演算子
        gen_op2(node);
    }
    return 1;   //結果をスタックに積んでいる
}

//２項演算子の処理
static void gen_op2(Node *node) {
    int cnt;
    Node *lhs = node->lhs;
    Node *rhs = node->rhs;
    //lhsとrhsを処理して結果をPUSHする
    assert(lhs!=NULL);
    assert(rhs!=NULL);
    gen(lhs);
    gen(rhs);

    //それらをPOPして、演算する
    printf("  pop rdi\n");  //rhs
    printf("  pop rax\n");  //lhs

    int size1 = size_of(lhs->tp);
    int size2 = size_of(rhs->tp);
    char *reg1, *reg2;
    if (size1>4 || size2>4) {
        reg1 = "rax";
        reg2 = "rdi"; 
    } else {
        reg1 = "eax";
        reg2 = "edi"; 
    }

    switch(node->type) {
    case ND_LOR:    //"||"
        cnt = label_cnt++;
        comment("'||'\n");
        printf("  cmp rax, 0\n");   //lhs
        printf("  jne .LTrue%03d\n", cnt);
        printf("  cmp rdi, 0\n");   //rhs
        printf("  jne .LTrue%03d\n", cnt);
        printf("  mov rax, 0\n");   //False
        printf("  jmp .LEnd%03d\n", cnt);
        printf(".LTrue%03d:\n", cnt);
        printf("  mov rax, 1\n");   //True
        printf(".LEnd%03d:\n", cnt);
        break;
    case ND_LAND:   //"&&"
        cnt = label_cnt++;
        comment("'&&'\n");
        printf("  cmp rax, 0\n");   //lhs
        printf("  je .LFalse%03d\n", cnt);
        printf("  cmp rdi, 0\n");   //rhs
        printf("  je .LFalse%03d\n", cnt);
        printf("  mov rax, 1\n");   //True
        printf("  jmp .LEnd%03d\n", cnt);
        printf(".LFalse%03d:\n", cnt);
        printf("  mov rax, 0\n");   //False
        printf(".LEnd%03d:\n", cnt);
        break;
    case ND_OR:     //'|'
        printf("  or rax, rdi\t # |\n");
        break;
    case ND_XOR:    //'^'
        printf("  xor rax, rdi\t # ^\n");
        break;
    case ND_AND:    //'&'
        printf("  and rax, rdi\t # &\n");
        break;
    case ND_EQ:     //"=="
        comment("'=='\n");
        printf("  cmp %s, %s\n", reg1, reg2);
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_NE:     //"!="
        comment("'!='\n");
        printf("  cmp %s, %s\n", reg1, reg2);
        printf("  setne al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_LT:     //'<': ND_GT('>')もここで対応（構文木作成時に左右入れ替えてある）
        comment("'<' or '>'\n");
        printf("  cmp %s, %s\n", reg1, reg2);
        if (lhs->tp->is_unsigned || rhs->tp->is_unsigned) {
            printf("  setb al\n");
        } else {
            printf("  setl al\n");
        }
        printf("  movzb rax, al\n");
        break;
    case ND_LE:     //"<=": ND_GE(">=")もここで対応（構文木作成時に左右入れ替えてある）
        comment("'<=' or '>='\n");
        printf("  cmp %s, %s\n", reg1, reg2);
        if (lhs->tp->is_unsigned || rhs->tp->is_unsigned) {
            printf("  setbe al\n");
        } else {
            printf("  setle al\n");
        }
        printf("  movzb rax, al\n");
        break;
    case ND_SHIFTL:
        comment("'<<'\n");
        printf("  mov rcx, rdi\n");
        printf("  sal rax, cl\n");
        break;
    case ND_SHIFTR:
        comment("'<<'\n");
        printf("  mov rcx, rdi\n");
        printf("  sar rax, cl\n");
        break;
    case ND_PLUS:   //'+': rax(lhs)+rdi(rhs)
        comment("'+' %s %s\n", get_type_str(lhs->tp), get_type_str(rhs->tp));
        if (node_is_ptr(lhs)) {
            gen_mul_reg("rdi", size_of(lhs->tp->ptr_of));
        } else if (node_is_ptr(rhs)) {
            gen_mul_reg("rax", size_of(rhs->tp->ptr_of));
        }
        printf("  add rax, rdi\n");
        break;
    case ND_MINUS:  //'-': rax(lhs)-rdi(rhs)
        comment("'-' %s %s\n", get_type_str(lhs->tp), get_type_str(rhs->tp));
        if (node_is_ptr(lhs) && node_is_ptr(rhs)) {
            printf("  sub rax, rdi\n");
            gen_div_reg("rax", size_of(lhs->tp->ptr_of));
        } else {
            if (node_is_ptr(lhs)) {
                gen_mul_reg("rdi", size_of(lhs->tp->ptr_of));
            }
            printf("  sub rax, rdi\n");
        }
        break;
    case ND_MUL:    //'*': rax*rdi -> rdx:rax
        comment("'*'\n");
        printf("  imul rdi\n");
        break;
    case ND_DIV:    //'/': rdx:rax(lhs) / rdi(rhs) -> rax（商）, rdx（余り）
        comment("'/'\n");
        printf("  mov rdx, 0\n");
        printf("  div rdi\n");
        break;
    case ND_MOD:    //'%': rdx:rax / rdi -> rax（商）, rdx（余り）
        comment("'%%'\n");
        printf("  cqo\n");
        printf("  idiv rdi\n");
        printf("  mov rax, rdx\n");
        break;
    default:
        _NOT_YET_(node);
    }

    printf("  push rax\n");
}

static long get_single_val(Node *node) {
    long val;
    if (node->type==ND_NUM) {
        val = node->val;
    } else if (node->type==ND_LIST) {
        val = get_single_val((Node*)node->lst->data[0]);
    } else {
        error_at(node->input, "スカラー定数ではありません");
    }
    return val;
}

//グローバルシンボルのコードを生成
static void gen_global_var(Node *node) {
    if (node->type==ND_FUNC_DEF || node->type==ND_FUNC_DECL) {
        if (!type_is_static(node->tp))
            printf(".global %s\n", node->name);
        return;
    }

    assert(node->type == ND_GLOBAL_VAR_DEF || type_is_static(node->tp));
    if (type_is_extern(node->tp)) return;
    int size = size_of(node->tp);
    int align_size = align_of(node->tp);

    if (!type_is_static(node->tp))
        printf(".global %s\n", node->name);
    if (align_size > 1) printf(".align %d\n", align_size);
    if (node->type == ND_LOCAL_VAR_DEF && type_is_static(node->tp)) {
        printf("%s.%03d:\t# %s\n", node->name, node->offset, get_type_str(node->tp));
    } else {
        printf("%s:\t# %s\n", node->name, get_type_str(node->tp));
    }

    if (node->rhs) {    //初期値あり、rhsは'='のノード
        Node *rhs = node->rhs->rhs; //'='の右辺
        switch (node->tp->type) {
        case CHAR:
            printf("  .byte %ld\n", get_single_val(rhs));
            break;
        case SHORT:
            printf("  .value %ld\n", get_single_val(rhs));
            break;
        case INT:
            printf("  .long %ld\n", get_single_val(rhs));
            break;
        case LONG:
        case LONGLONG:
            printf("  .quad %ld\n", get_single_val(rhs));
            break;
        case PTR:
            if (rhs->type==ND_ADDRESS) {
                printf("  .quad %s\n", get_asm_var_name(rhs->rhs));
            } else if (rhs->type==ND_NUM || rhs->type==ND_LIST) {
                printf("  .quad %ld\n", get_single_val(rhs));
            } else if (rhs->type=='+') {
                printf("  .quad %s%+ld\n", rhs->lhs->rhs->name, size_of(node->tp->ptr_of)*rhs->rhs->val);
            } else {
                _NOT_YET_(rhs);
            }
            break;
        case ARRAY:
            gen_array_init_global(node->rhs);
            return;
        default:
            _NOT_YET_(node);
        }
    } else {
        printf("  .zero %d\n", size);
    }

}

//ローカル変数用のスタックサイズを計算する
//RSPの16バイトアライメントを維持する
static int calc_stack_offset(int stack_size) {
    int size = (stack_size + 15)/16 * 16;
    return size;
}

void gen_program(void) {
    int size;

    // アセンブリのヘッダ部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".section .data\n");

    //グローバル変数
    size = global_symbol_map->vals->len;
    Node **nodes = (Node**)global_symbol_map->vals->data;
    for (int i=0; i<size; i++) {
        gen_global_var(nodes[i]);
    }

    //ローカルstatic変数
    size = static_var_vec->len;
    nodes = (Node**)static_var_vec->data;
    for (int i=0; i < size; i++) {
        gen_global_var(nodes[i]);
    }

    //文字列リテラル
    size = string_vec->len;
    char **strs = (char**)string_vec->data;
    for (int i=0; i<size; i++) {
        printf(".LC%03d:\n", i);
        printf("   .string \"%s\"\n", strs[i]);
    }

    // 関数ごとに、抽象構文木を下りながらコード生成
    printf(".section .text\n");
    Funcdef **funcdef = (Funcdef**)funcdef_map->vals->data;
    for (int i=0; i < funcdef_map->keys->len; i++) {
        assert(funcdef[i]->node->type==ND_FUNC_DEF);
        cur_funcdef = funcdef[i];
        printf("%s:\t#%s %s(%s)\n", funcdef[i]->name, 
            get_type_str(funcdef[i]->tp), funcdef[i]->name,
            get_func_args_str(funcdef[i]->node->lhs));

        // プロローグ
        // ローカル変数用の領域を確保する
        printf("  push rbp\n");
        printf("  mov rbp, rsp\n");
        printf("  sub rsp, %d\n", calc_stack_offset(funcdef[i]->var_stack_size));

        // 引数をスタックのローカル変数領域にコピー
        int size = funcdef[i]->node->lhs->lst->len;
        Node **arg_nodes = (Node**)funcdef[i]->node->lhs->lst->data;
        if (size && arg_nodes[0]->tp->type!=VOID) {
            char buf[128];
            assert(size<=6);
            printf("  mov rax, rbp\n");
            for (int j=0; j < size; j++) {
                Node *arg = arg_nodes[j];
                if (arg->type==ND_VARARGS) {  //...（可変引数）
                    ; 
                } else {
                    printf("  sub rax, %ld\n", size_of(arg->tp));
                    sprintf(buf, "arg:%s %s", get_type_str(arg->tp) ,arg->name);
                    gen_write_reg("rax", arg_regs[j], arg->tp, buf);
                }
            }
        }

        // 関数本体のコード生成
        gen(funcdef[i]->node->rhs);

        // エピローグ
        // 最後の式の結果がRAXに残っているのでそれが返り値になる
        printf("  # Epilogue\n");
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
    }

}
