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
    {"bpl",  "bp", "ebp", "rbp" },
    {"spl",  "sp", "esp", "rsp" },
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
        case 1: return 0;
        case 2: return 1;
        case 4: return 2;
        case 8: return 3;
        case 16: return 4;
        case 32: return 5;
        case 64: return 6;
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
        printf("  mul rdx\t# end\n");
    } else {
        printf("  push rax\t# start: %s * %d\n", reg_name, val);
        printf("  mov rax, %s\n", reg_name);
        printf("  mov rdx, %d\n", val);
        printf("  mul rdx\n");
        printf("  mov %s, rax\n", reg_name);
        printf("  pop rax\t# end\n");
    }
}

//型に応じたデータ型名を返す。例：intならDWORD
static char* data_name_for_type(const Type *tp) {
    if (tp->type==ARRAY) tp = tp->ptr_of;
    switch (size_of(tp)) {
    case 8: return "QWORD";
    case 4: return "DWORD";
    case 2: return "WORD";
    case 1: return "BYTE";
    default: assert(0);
    }
    return NULL;
}

//型に応じたレジスタ名を返す。例：intならrax->eax
static char* reg_name_of_type(const char*reg_name, const Type *tp) {
    int idx;
    switch (size_of(tp)) {
    case 8: idx = 3; break; //32bit
    case 4: idx = 2; break;
    case 2: idx = 1; break;
    case 1: idx = 0; break; //8bit
    default: assert(0);
    }
    for (int j=3; j>=0; j--) {
        for (int i=0; regs[i][j]; i++) {
            if (strcmp(reg_name, regs[i][j])==0) return regs[i][idx];
        }
    }
    assert(0);
    return NULL;
}

//型に応じたwriteコマンド名を返す。
static char* write_command_of_type(const Type *tp) {
    switch (tp->type) {
    case CHAR:  return "movb";
    case INT:   return "mov";
    case PTR:   return "mov";
    case ARRAY: return "mov";
    default: assert(0);
    }
    return NULL;
}

//型に応じたreadコマンド名を返す。
static char* read_command_of_type(const Type *tp) {
    switch (tp->type) {
    case CHAR:  return "movsx";
    case INT:   return "mov";
    case PTR:   return "mov";
    case ARRAY: return "mov";
    default: assert(0);
    }
    return NULL;
}

//[dst]レジスタが指すアドレスにsrcレジスタの値をwriteする。
//srcレジスタの型に応じてsrcレジスタのサイズを調整する。例：intならrax->eax
static void gen_write_reg(const char*dst, const char*src, const Type *tp, const char* comment) {
    printf("  %s [%s], %s", write_command_of_type(tp), dst, reg_name_of_type(src, tp));
    if (comment) printf("\t# %s\n", comment);
    else         printf("\n");
}

//dstレジスタにsrcレジスタが指すアドレスからreadする。
//dstレジスタの型に応じてdstレジスタのサイズを調整する。例：intならrax->eax
static void gen_read_reg(const char*dst, const char*src, const Type *tp, const char* comment) {
    printf("  %s %s, %s PTR [%s]", read_command_of_type(tp), 
        (tp->type==CHAR)?src:reg_name_of_type(src, tp), data_name_for_type(tp), src);
    if (comment) printf("\t# %s\n", comment);
    else         printf("\n");
}

static int gen(Node*node);

//式を左辺値として評価し、そのアドレスをPUSHする
static void gen_lval(Node*node) {
    if (node->type == ND_LOCAL_VAR) {   //ローカル変数
        Vardef *vardef;
        map_get(cur_funcdef->ident_map, node->name, (void**)&vardef);
        comment("LVALUE:%s\n", node->name);
        printf("  mov rax, rbp\n");
        printf("  sub rax, %d\n", vardef->offset);  //ローカル変数のアドレス
        printf("  push rax\n");
    } else if (node->type == ND_GLOBAL_VAR) {   //グローバル変数
        Vardef *vardef;
        map_get(global_vardef_map, node->name, (void**)&vardef);
        comment("LVALUE:%s\n", node->name);
        printf("  lea rax, %s PTR %s\n", data_name_for_type(vardef->node->tp) ,vardef->name);
        printf("  push rax\n");
    } else if (node->type == ND_INDIRECT) {
        comment("LVALUE:*var\n");
        gen(node->rhs);     //rhsのアドレスを生成する
    } else {
        error_at(node->input, "アドレスを生成できません");
    }
}

static int label_cnt = 0;   //ラベル識別用カウンタ

//ステートメントを評価
//結果をスタックに積んだ場合は1、そうでない場合は0を返す
static int gen(Node*node) {
    assert(node!=NULL);
    if (node->type == ND_NUM) {             //数値
        printf("  push %d\t# NUM\n", node->val);
    } else if (node->type == ND_STRING) {   //文字列リテラル
        char buf[20];
        sprintf(buf, ".LC%03d", node->val);
        printf("  lea rax, BYTE PTR %s\n", buf);
        printf("  push rax\n");
    } else if  (node->type == ND_EMPTY) {   //空
        return 0;
    } else if (node->type == ND_VAR_DEF) {  //ローカル変数定義
        if (node->rhs==NULL) return 0;
        return gen(node->rhs); //代入
    } else if (node->type == ND_LOCAL_VAR ||
               node->type == ND_GLOBAL_VAR) {//変数参照
        comment("%s_VAR:%s(%s)\n", node->type==ND_LOCAL_VAR?"LOCAL":"GLOBAL", node->name, get_type_str(node->tp));
        gen_lval(node);
        if (node->tp->type==ARRAY) {
            printf("  push rax\n");
            printf("  pop rax\n");
        } else {
            printf("  pop rax\n");  //rhsのアドレス=戻り値
            gen_read_reg("rax", "rax", node->tp, NULL);
            printf("  push rax\n");
        }
    } else if (node->type == ND_FUNC_CALL) {//関数コール
        comment("CALL:%s\n", node->name);
        if (node->lhs) {
            int i;
            assert(node->lhs->type==ND_LIST);
            Vector *lists = node->lhs->lst;
            Node **nodes = (Node**)lists->data;
            for (i=0; i < lists->len; i++) {
                comment("ARGLIST[%d]\n", i);
                gen(nodes[i]);  //スタックトップに引数がセットされる
            }
            for (; i; i--) {
                printf("  pop %s\n", arg_regs[i-1]);
            }
        }
        printf("  mov al, 0\n");
        printf("  call %s\n", node->name);
        printf("  push rax\n");
    } else if (node->type == ND_RETURN) {   //return
        comment("RETURN\n");
        gen(node->lhs);
        printf("  pop rax\t# RETURN VALUE\n");
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
    } else if (node->type == ND_IF) {       //if (A) B [else C]
        int cnt = label_cnt++;
        comment("IF(A) B [else C]\n");
        gen(node->lhs->lhs); //A
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  jne .LIfThen%03d\n", cnt);
        printf("  push rax\n");
        if (node->rhs) {    //elseあり
            printf("  jmp .LIfElse%03d\n", cnt);
        } else {
            printf("  jmp .LIfEnd%03d\n", cnt);
        }
        printf(".LIfThen%03d:\n", cnt);
        gen(node->lhs->rhs); //B
        if (node->rhs) {    //elseあり
            printf("  jmp .LIfEnd%03d\n", cnt);
            printf(".LIfElse%03d:\n", cnt);
            gen(node->rhs); //C
        }
        printf(".LIfEnd%03d:\n", cnt);
    } else if (node->type == ND_WHILE) {    //while (A) B
        int cnt = label_cnt++;
        comment("WHILE(A)B\n");
        printf(".LWhileBegin%03d:\n", cnt);
        gen(node->lhs); //A
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  jne .LWhileBody%03d\n", cnt);
        printf("  push rax\n");
        printf("  jmp .LWhileEnd%03d\n", cnt);
        printf(".LWhileBody%03d:\n", cnt);
        gen(node->rhs); //B
        printf("  jmp .LWhileBegin%03d\n", cnt);
        printf(".LWhileEnd%03d:\n", cnt);
    } else if (node->type == ND_FOR) {      //for (A;B;C) D
        int cnt = label_cnt++;
        comment("FOR(A;B;C)D\n");
        if (node->lhs->lhs->type != ND_EMPTY) {
            gen(node->lhs->lhs);//A
            printf("  pop rax\n");
        }
        printf(".LForBegin%03d:\n", cnt);
        if (node->lhs->rhs->type != ND_EMPTY) {
            comment("FOR:B\n");
            gen(node->lhs->rhs);//B
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  jne .LForBody%03d\n", cnt);
            printf("  push rax\n");
            printf("  jmp .LForEnd%03d\n", cnt);
        }
        printf(".LForBody%03d:\n", cnt);
        comment("FOR:D\n");
        gen(node->rhs->rhs);    //D
        printf("  pop rax\n");
        if (node->rhs->lhs->type != ND_EMPTY) {
            comment("FOR:C\n");
            gen(node->rhs->lhs);//C
            printf("  pop rax\n");
        }
        printf("  jmp .LForBegin%03d\n", cnt);
        printf(".LForEnd%03d:\n", cnt);
    } else if (node->type == ND_BLOCK) {    //{ ブロック }
        Vector *blocks = node->lst;
        Node **nodes = (Node**)blocks->data;
        for (int i=0; i < blocks->len; i++) {
            comment("BLOCK[%d]\n", i);
            if (gen(nodes[i])) printf("  pop rax\n");
        }
        printf("  push rax\n");
    } else if (node->type == ND_LIST) {     //コンマリスト
        Vector *lists = node->lst;
        Node **nodes = (Node**)lists->data;
        for (int i=0; i < lists->len; i++) {
            comment("LIST[%d]\n", i);
            gen(nodes[i]);
            printf("  pop rax\n");
        }
        printf("  push rax\n");
    } else if (node->type == '=') {         //代入
        comment("'='\n");
        gen_lval(node->lhs);    //スタックトップにアドレス設定
        if (node->lhs->tp->type==ARRAY) {   //ローカル変数の初期値
            //文字配列の初期化
            assert(node->lhs->type==ND_LOCAL_VAR);
            assert(node->rhs->tp->ptr_of->type==CHAR);
            int size  = node->lhs->tp->array_size;
            int rsize = node->rhs->tp->array_size;
            if (size > rsize) size = rsize;
            printf("  pop rdi\n");
            printf("  lea rsi, BYTE PTR .LC%03d\n", node->rhs->val);
            printf("  mov rcx, %d\n", size);
            printf("  rep movsb\n");
            return 0;
        } else {
            printf("  push 0\t#for RSP alignment+\n");
            gen(node->rhs);
            printf("  pop rax\n");  //rhsの値
            printf("  pop rdi\t#for RSP alignment-\n");
            printf("  pop rdi\n");  //lhsのアドレス
        //  printf("  mov [rdi], rax\n");
            gen_write_reg("rdi", "rax", node->tp, NULL);
            printf("  push rax\n");
        }
    } else if (node->type == ND_INDIRECT) { //*a（間接参照）
        comment("'*A'\n");
        gen(node->rhs);
        printf("  pop rax\n");  //rhsの値（アドレス）
    //  printf("  mov rax, [rax]\n");//戻り値
        gen_read_reg("rax", "rax", node->tp, NULL);
        printf("  push rax\n");
    } else if (node->type == ND_ADDRESS) { //&a（アドレス演算子）
        comment("'&A'\n");
        gen_lval(node->rhs);
        printf("  pop rax\n");  //rhsのアドレス=戻り値
        printf("  push rax\n");
    } else if (node->type == ND_INC_PRE) {  //++a
        comment("'++A'\n");
        gen_lval(node->rhs);
        printf("  pop rdi\n");  //rhsのアドレス
        printf("  mov rax, [rdi]\n");
        if (node_is_ptr(node)) {
            printf("  add rax, %d\n", size_of(node->tp->ptr_of));
        } else {
            printf("  inc rax\n");  //戻り値を設定する前にINC
        }
        printf("  mov [rdi], rax\n");
        printf("  push rax\n"); //戻り値
    } else if (node->type == ND_DEC_PRE) {  //--a
        comment("'--A'\n");
        gen_lval(node->rhs);
        printf("  pop rdi\n");  //rhsのアドレス
        printf("  mov rax, [rdi]\n");
        if (node_is_ptr(node)) {
            printf("  sub rax, %d\n", size_of(node->tp->ptr_of));
        } else {
            printf("  dec rax\n");  //戻り値を設定する前にDEC
        }
        printf("  mov [rdi], rax\n");
        printf("  push rax\n"); //戻り値
    } else if (node->type == ND_INC) {      //a++
        comment("'A++'\n");
        gen_lval(node->lhs);
        printf("  pop rdi\n");  //lhsのアドレス
        printf("  mov rax, [rdi]\n");
        printf("  push rax\n"); //INCする前に戻り値を設定
        if (node_is_ptr(node)) {
            printf("  add rax, %d\n", size_of(node->tp->ptr_of));
        } else {
            printf("  inc rax\n");
        }
        printf("  mov [rdi], rax\n");
    } else if (node->type == ND_DEC) {      //a--
        comment("'A--'\n");
        gen_lval(node->lhs);
        printf("  pop rdi\n");  //lhsのアドレス
        printf("  mov rax, [rdi]\n");
        printf("  push rax\n"); //DECする前に戻り値を設定
        if (node_is_ptr(node)) {
            printf("  sub rax, %d\n", size_of(node->tp->ptr_of));
        } else {
            printf("  dec rax\n");
        }
        printf("  mov [rdi], rax\n");
    } else if (node->type == '!') {         //否定
        comment("'!'\n");
        gen(node->rhs);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        printf("  push rax\n");
    } else {                                //2項演算子
        //lhsとrhsを処理して結果をPUSHする
        assert(node->lhs!=NULL);
        assert(node->rhs!=NULL);
        gen(node->lhs);
        gen(node->rhs);

        //それらをPOPして、演算する
        printf("  pop rdi\n");  //rhs
        printf("  pop rax\n");  //lhs

        switch((int)node->type) {
        case ND_LAND: //"&&"
        {
            int cnt = label_cnt++;
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
        }
        case ND_LOR: //"||"
        {
            int cnt = label_cnt++;
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
        }
        case ND_EQ: //"=="
            comment("'=='\n");
            printf("  cmp rax, rdi\n");
            printf("  sete al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_NE: //"!="
            comment("'!='\n");
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            printf("  movzb rax, al\n");
            break;
        case '<':   //'>'もここで対応（構文木作成時に左右入れ替えてある）
            comment("'<' or '>'\n");
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_LE: //"<="、">="もここで対応（構文木作成時に左右入れ替えてある）
            comment("'<=' or '>='\n");
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            printf("  movzb rax, al\n");
            break;
        case '+':   //rax(lhs)+rdi(rhs)
            comment("'+' %s %s\n", get_type_str(node->lhs->tp), get_type_str(node->rhs->tp));
            if (node_is_ptr(node->lhs)) {
                gen_mul_reg("rdi", size_of(node->lhs->tp->ptr_of));
            } else if (node_is_ptr(node->rhs)) {
                gen_mul_reg("rax", size_of(node->rhs->tp->ptr_of));
            }
            printf("  add rax, rdi\n");
            break;
        case '-':   //rax(lhs)-rdi(rhs)
            comment("'-' %s %s\n", get_type_str(node->lhs->tp), get_type_str(node->rhs->tp));
            assert(!node_is_ptr(node->rhs));
            if (node_is_ptr(node->lhs)) {
                gen_mul_reg("rdi", size_of(node->lhs->tp->ptr_of));
            }
            printf("  sub rax, rdi\n");
            break;
        case '*':   //rax*rdi -> rdx:rax
            comment("'*'\n");
            printf("  mul rdi\n");
            break;
        case '/':   //rdx:rax(lhs) / rdi(rhs) -> rax（商）, rdx（余り）
            comment("'/'\n");
            printf("  mov rdx, 0\n");
            printf("  div rdi\n");
            break;
        case '%':   //rdx:rax / rdi -> rax（商）, rdx（余り）
            comment("'%%'\n");
            printf("  mov rdx, 0\n");
            printf("  div rdi\n");
            printf("  mov rax, rdx\n");
            break;
        default:
            error_at(node->input, "不正なトークンです");
        }

        printf("  push rax\n");
    }
    return 1;   //結果をスタックに積んでいる
}

//グローバル変数のコードを生成
static void gen_global_var(Node *node) {
    int size = size_of(node->tp);
    printf("%s:\t# %s\n", node->name, get_type_str(node->tp));
    if (node->rhs) {    //初期値あり、rhsは'='のノード
        Node *rhs = node->rhs->rhs; //'='の右辺
        switch (node->tp->type) {
        case CHAR:
            assert(rhs->type==ND_NUM);
            printf("  .byte %d\n", rhs->val);
            break;
        case INT:
            assert(rhs->type==ND_NUM);
            printf("  .long %d\n", rhs->val);
            break;
        case PTR:
            if (rhs->type==ND_ADDRESS) {
                printf("  .quad %s\n", rhs->rhs->name);
            } else {
                printf("  .quad %d\n", rhs->val);
            }
            break;
        case ARRAY:
            assert(node->tp->ptr_of->type==CHAR);
            int rsize = rhs->tp->array_size;
            char *str = (char*)vec_get(string_vec, rhs->val);
            if (size < rsize) {
                str[size] = 0;
                printf("  .ascii \"%s\"\n", str);
            } else {
                printf("  .string \"%s\"\n", str);
            }
            if (size > rsize) printf("  .zero %d\n", size-rsize);
            break;
        default:
            assert(0);
        }
    } else {
        printf("  .zero %d\n", size);
    }

}

//ローカル変数用のスタックサイズを計算する
//RSPの16バイトアライメントを維持する
static int calc_stack_offset() {
    int size = (var_stack_size + 15)/16 * 16;
    return size;
}

void print_functions(void) {
    int size;

    // アセンブリのヘッダ部分を出力
    printf(".intel_syntax noprefix\n");
    size = func_map->keys->len;

    //グローバルシンボル（関数）
    char **strs = (char**)func_map->keys->data;
    for (int i=0; i<size; i++) {
        printf(".global %s\n", strs[i]);
    }

    //グローバルシンボル（変数）
    size = global_vardef_map->keys->len;
    strs = (char**)global_vardef_map->keys->data;
    for (int i=0; i<size; i++) {
        printf(".global %s\n", strs[i]);
    }

    printf(".section .data\n");
    //文字列リテラル
    size = string_vec->len;
    strs = (char**)string_vec->data;
    for (int i=0; i<size; i++) {
        printf(".LC%03d:\n", i);
        printf("   .string \"%s\"\n", strs[i]);
    }

    //グローバル変数
    size = global_vardef_map->vals->len;
    Vardef **vardefs = (Vardef**)global_vardef_map->vals->data;
    for (int i=0; i<size; i++) {
        gen_global_var(vardefs[i]->node);
    }

    // 関数ごとに、抽象構文木を下りながらコード生成
    printf(".section .text\n");
    size = funcdef_map->vals->len;
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
        printf("  sub rsp, %d\n", calc_stack_offset());

        // 引数をスタックのローカル変数領域にコピー
        int size = funcdef[i]->node->lhs->lst->len;
        Node **ident_nodes = (Node**)funcdef[i]->node->lhs->lst->data;
        if (size) {
            assert(size<=6);
            printf("  mov rax, rbp\n");
            for (int j=0; j < size; j++) {
                char buf[128];
                printf("  sub rax, %d\n", size_of(ident_nodes[j]->tp));
                sprintf(buf, "arg:%s %s", get_type_str(ident_nodes[j]->tp) ,ident_nodes[j]->name);
                gen_write_reg("rax", arg_regs[j], ident_nodes[j]->tp, buf);
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
