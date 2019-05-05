#include "9cc.h"

//レジスタ ---------------------------------------------
static char *arg_regs[] = {  //関数の引数で用いるレジスタ
    "rdi", "rsi", "rdx", "rcx", "r8", "r9", NULL
};

//抽象構文木を下りながらコード生成（スタックマシン）

static void comment(const char*fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    printf("  # ");
    vprintf(fmt, ap);
}

static void gen(Node*node);

//式を左辺値として評価し、そのアドレスをPUSHする
static void gen_lval(Node*node) {
    if (node->type == ND_IDENT) {   //変数
        Vardef *vardef;
        map_get(cur_funcdef->ident_map, node->name, (void**)&vardef);
        comment("LVALUE:%s\n", node->name);
        printf("  mov rax, rbp\n");
        printf("  sub rax, %d\n", vardef->offset);  //ローカル変数のアドレス
        printf("  push rax\n");
    } else if (node->type == ND_INDIRECT) {
        comment("LVALUE:*var\n");
        gen(node->rhs);     //rhsでアドレスを生成する
        if (node->rhs->tp->type != PTR) error("'*'は非ポインタを参照しています: %s\n", node->input);
    } else {
        error("アドレスを生成できません: %s", node->input);
    }
}

static int label_cnt = 0;   //ラベル識別用カウンタ

//ステートメントを評価
static void gen(Node*node) {
    assert(node!=NULL);
    if (node->type == ND_NUM) {             //数値
        printf("  push %d\t# NUM\n", node->val);
    } else if  (node->type == ND_EMPTY) {   //空
    //  printf("  push 0\t# EMPTY\n");
        printf("  push rax\t# EMPTY\n");
    } else if (node->type == ND_VAR_DEF) {  //ローカル変数定義
        printf("  push 0\t# VAR_DEF\n");
    } else if (node->type == ND_IDENT) {    //変数参照
        comment("IDENT:%s\n", node->name);
        gen_lval(node);
        printf("  pop rax\n");
        printf("  mov rax, [rax]\t# IDENT:%s\n", node->name);
        printf("  push rax\n");
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
        printf("  jne .IfThen%03d\n", cnt);
        printf("  push rax\n");
        if (node->rhs) {    //elseあり
            printf("  jmp .IfElse%03d\n", cnt);
        } else {
            printf("  jmp .IfEnd%03d\n", cnt);
        }
        printf(".IfThen%03d:\n", cnt);
        gen(node->lhs->rhs); //B
        if (node->rhs) {    //elseあり
            printf("  jmp .IfEnd%03d\n", cnt);
            printf(".IfElse%03d:\n", cnt);
            gen(node->rhs); //C
        }
        printf(".IfEnd%03d:\n", cnt);
    } else if (node->type == ND_WHILE) {    //while (A) B
        int cnt = label_cnt++;
        comment("WHILE(A)B\n");
        printf(".WhileBegin%03d:\n", cnt);
        gen(node->lhs); //A
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  jne .WhileBody%03d\n", cnt);
        printf("  push rax\n");
        printf("  jmp .WhileEnd%03d\n", cnt);
        printf(".WhileBody%03d:\n", cnt);
        gen(node->rhs); //B
        printf("  jmp .WhileBegin%03d\n", cnt);
        printf(".WhileEnd%03d:\n", cnt);
    } else if (node->type == ND_FOR) {      //for (A;B;C) D
        int cnt = label_cnt++;
        comment("FOR(A;B;C)D\n");
        if (node->lhs->lhs->type != ND_EMPTY) {
            gen(node->lhs->lhs);//A
            printf("  pop rax\n");
        }
        printf(".ForBegin%03d:\n", cnt);
        if (node->lhs->rhs->type != ND_EMPTY) {
            comment("FOR:B\n");
            gen(node->lhs->rhs);//B
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  jne .ForBody%03d\n", cnt);
            printf("  push rax\n");
            printf("  jmp .ForEnd%03d\n", cnt);
        }
        printf(".ForBody%03d:\n", cnt);
        comment("FOR:D\n");
        gen(node->rhs->rhs);    //D
        printf("  pop rax\n");
        if (node->rhs->lhs->type != ND_EMPTY) {
            comment("FOR:C\n");
            gen(node->rhs->lhs);//C
            printf("  pop rax\n");
        }
        printf("  jmp .ForBegin%03d\n", cnt);
        printf(".ForEnd%03d:\n", cnt);
    } else if (node->type == ND_BLOCK) {    //{ ブロック }
        Vector *blocks = node->lst;
        Node **nodes = (Node**)blocks->data;
        for (int i=0; i < blocks->len; i++) {
            comment("BLOCK[%d]\n", i);
            gen(nodes[i]);
            printf("  pop rax\n");
        }
        printf("  push rax\n");
    } else if (node->type == ND_LIST) {         //コンマリスト
        Vector *lists = node->lst;
        Node **nodes = (Node**)lists->data;
        node->tp = nodes[lists->len-1]->tp;
        for (int i=0; i < lists->len; i++) {
            comment("LIST[%d]\n", i);
            gen(nodes[i]);
            printf("  pop rax\n");
        }
        printf("  push rax\n");
    } else if (node->type == '=') {         //代入
        node->tp = node->rhs->tp;
        comment("'='\n");
        gen_lval(node->lhs);    //スタックトップにアドレス設定
        printf("  push 0\t#for RSP alignment+\n");
        gen(node->rhs);
        printf("  pop rax\n");  //rhsの値
        printf("  pop rdi\t#for RSP alignment-\n");
        printf("  pop rdi\n");  //lhsのアドレス
        printf("  mov [rdi], rax\n");
        printf("  push rax\n");
    } else if (node->type == ND_INDIRECT) { //*a（間接参照）
        comment("'*A'\n");
        gen(node->rhs);
        if (node->rhs->tp->type != PTR) error("'*'は非ポインタを参照しています: %s\n", node->input);
        node->tp = node->rhs->tp->ptr_of;
        printf("  pop rax\n");  //rhsの値（アドレス）
        printf("  mov rax, [rax]\n");//戻り値
        printf("  push rax\n");
    } else if (node->type == ND_ADDRESS) { //&a（アドレス演算子）
        node->tp = new_type(node->tp);
        comment("'&A'\n");
        gen_lval(node->rhs);
        printf("  pop rax\n");  //rhsのアドレス=戻り値
        printf("  push rax\n");
    } else if (node->type == ND_INC_PRE) {  //++a
        node->tp = node->rhs->tp;
        comment("'++A'\n");
        gen_lval(node->rhs);
        printf("  pop rdi\n");  //rhsのアドレス
        printf("  mov rax, [rdi]\n");
        printf("  inc rax\n");  //戻り値を設定する前にINC
        printf("  mov [rdi], rax\n");
        printf("  push rax\n"); //戻り値
    } else if (node->type == ND_DEC_PRE) {  //--a
        node->tp = node->rhs->tp;
        comment("'--A'\n");
        gen_lval(node->rhs);
        printf("  pop rdi\n");  //rhsのアドレス
        printf("  mov rax, [rdi]\n");
        printf("  dec rax\n");  //戻り値を設定する前にDEC
        printf("  mov [rdi], rax\n");
        printf("  push rax\n"); //戻り値
    } else if (node->type == ND_INC) {      //a++
        node->tp = node->lhs->tp;
        comment("'A++'\n");
        gen_lval(node->lhs);
        printf("  pop rdi\n");  //lhsのアドレス
        printf("  mov rax, [rdi]\n");
        printf("  push rax\n"); //戻り値
        printf("  inc rax\n");  //戻り値を設定した後でINC
        printf("  mov [rdi], rax\n");
    } else if (node->type == ND_DEC) {      //a--
        node->tp = node->lhs->tp;
        comment("'A--'\n");
        gen_lval(node->lhs);
        printf("  pop rdi\n");  //lhsのアドレス
        printf("  mov rax, [rdi]\n");
        printf("  push rax\n"); //戻り値
        printf("  dec rax\n");  //戻り値を設定した後でDEC
        printf("  mov [rdi], rax\n");
    } else if (node->type == '!') {         //否定
        node->tp = new_type_int();
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

        switch(node->type) {
        case ND_LAND: //"&&"
        {
            int cnt = label_cnt++;
            node->tp = new_type_int();
            comment("'&&'\n");
            printf("  cmp rax, 0\n");   //lhs
            printf("  je .False%03d\n", cnt);
            printf("  cmp rdi, 0\n");   //rhs
            printf("  je .False%03d\n", cnt);
            printf("  mov rax, 1\n");   //True
            printf("  jmp .End%03d\n", cnt);
            printf(".False%03d:\n", cnt);
            printf("  mov rax, 0\n");   //False
            printf(".End%03d:\n", cnt);
            break;
        }
        case ND_LOR: //"||"
        {
            int cnt = label_cnt++;
            node->tp = new_type_int();
            comment("'||'\n");
            printf("  cmp rax, 0\n");   //lhs
            printf("  jne .True%03d\n", cnt);
            printf("  cmp rdi, 0\n");   //rhs
            printf("  jne .True%03d\n", cnt);
            printf("  mov rax, 0\n");   //False
            printf("  jmp .End%03d\n", cnt);
            printf(".True%03d:\n", cnt);
            printf("  mov rax, 1\n");   //True
            printf(".End%03d:\n", cnt);
            break;
        }
        case ND_EQ: //"=="
            node->tp = new_type_int();
            comment("'=='\n");
            printf("  cmp rax, rdi\n");
            printf("  sete al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_NE: //"!="
            node->tp = new_type_int();
            comment("'!='\n");
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            printf("  movzb rax, al\n");
            break;
        case '<':   //'>'もここで対応（構文木作成時に左右入れ替えてある）
            node->tp = new_type_int();
            comment("'<' or '>'\n");
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_LE: //"<="、">="もここで対応（構文木作成時に左右入れ替えてある）
            node->tp = new_type_int();
            comment("'<=' or '>='\n");
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            printf("  movzb rax, al\n");
            break;
        case '+':   //rax(lhs)+rdi(rhs)
            node->tp = node->lhs->tp;
            comment("'+'\n");
            printf("  add rax, rdi\n");
            break;
        case '-':   //rax(lhs)-rdi(rhs)
            node->tp = node->lhs->tp;
            comment("'-'\n");
            printf("  sub rax, rdi\n");
            break;
        case '*':   //rax*rdi -> rdx:rax
            node->tp = node->lhs->tp;
            comment("'*'\n");
            printf("  mul rdi\n");
            break;
        case '/':   //rdx:rax(lhs) / rdi(rhs) -> rax（商）, rdx（余り）
            node->tp = node->lhs->tp;
            comment("'/'\n");
            printf("  mov rdx, 0\n");
            printf("  div rdi\n");
            break;
        case '%':   //rdx:rax / rdi -> rax（商）, rdx（余り）
            node->tp = node->lhs->tp;
            comment("'%%'\n");
            printf("  mov rdx, 0\n");
            printf("  div rdi\n");
            printf("  mov rax, rdx\n");
            break;
        default:
            error("不正なトークンです: '%d'\n", node->type);
        }

        printf("  push rax\n");
    }
}

//RSPの16バイトアライメントを維持する
static int calc_stack_offset() {
    int size = (cur_funcdef->ident_map->keys->len+1)/2 *16;
    return size;
}

void print_functions(void) {
    int size;

    // アセンブリのヘッダ部分を出力
    printf(".intel_syntax noprefix\n");
    size = func_map->keys->len;
    char **names = (char**)func_map->keys->data;
    for (int i=0; i<size; i++) {
        printf(".global %s\n", names[i]);
    }

    // 関数ごとに、抽象構文木を下りながらコード生成
    size = funcdef_map->vals->len;
    Funcdef **funcdef = (Funcdef**)funcdef_map->vals->data;
    for (int i=0; i < funcdef_map->keys->len; i++) {
        assert(funcdef[i]->node->type==ND_FUNC_DEF);
        cur_funcdef = funcdef[i];
        printf("%s:\n", funcdef[i]->name);

        // プロローグ
        // ローカル変数用の領域を確保する
        printf("  push rbp\n");
        printf("  mov rbp, rsp\n");
        printf("  sub rsp, %d\n", calc_stack_offset());

        // 引数をスタックのローカル変数領域にコピー
        int size = funcdef[i]->node->lhs->lst->len;
        Node **ident_nodes = (Node**)funcdef[i]->node->lhs->lst->data;
        if (size) {
            assert(arg_regs[size-1]);
            printf("  mov rax, rbp\n");
            for (int j=0; j < size; j++) {
                printf("  sub rax, 8\n");
                printf("  mov [rax], %s\t#arg:%s\n", arg_regs[j], ident_nodes[j]->name);
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
