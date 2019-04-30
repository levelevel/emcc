#include <stdarg.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

//抽象構文木を下りながらコード生成（スタックマシン）

//式を左辺値として評価し、そのアドレスをPUSHする
static void gen_lval(Node*node) {
    if (node->type != ND_IDENT) {   //変数
        error("代入の左辺値が変数ではありません");
    }
    int offset = (long)map_get(ident_map, node->name);
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", offset);
    printf("  push rax\n");
}

static void comment(const char*fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
}

static int label_cnt = 0;   //ラベル識別用カウンタ

//ステートメントを評価
void gen(Node*node) {
    if (node->type == ND_NUM) {             //数値
        comment("  # NUM:%d\n", node->val);
        printf("  push %d\n", node->val);
    } else if (node->type == ND_IDENT) {    //変数
        comment("  # IDENT:%s\n", node->name);
        gen_lval(node);
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
    } else if (node->type == ND_RETURN) {   //return
        comment("  # RETURN\n");
        gen(node->lhs);
        printf("  pop rax\n");
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
    } else if (node->type == ND_IF) {       //if (A) B
        int cnt = label_cnt++;
        comment("  # IF(A)B\n");
        gen(node->lhs); //A
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .Lend%03d\n", cnt);
        comment("  # IF:B\n");
        gen(node->rhs); //B
        printf(".Lend%03d:\n", cnt);
    } else if (node->type == ND_WHILE) {    //while (A) B
        int cnt = label_cnt++;
        comment("  # WHILE(A)B\n");
        printf(".Lbegin%03d:\n", cnt);
        gen(node->lhs); //A
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .Lend%03d\n", cnt);
        comment("  # WHILE:B\n");
        gen(node->rhs); //B
        printf("  jmp .Lbegin%03d\n", cnt);
        printf(".Lend%03d:\n", cnt);
    } else if (node->type == ND_FOR) {      //for (A;B;C) D
        int cnt = label_cnt++;
        comment("  # FOR(A;B;C)D\n");
        if (node->lhs->lhs) {
            gen(node->lhs->lhs);//A
            printf("  pop rax\n");
        }
        printf(".Lbegin%03d:\n", cnt);
        if (node->lhs->rhs) {
            comment("  # FOR:B\n");
            gen(node->lhs->rhs);//B
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .Lend%03d\n", cnt);
        }
        comment("  # FOR:D\n");
        gen(node->rhs->rhs);    //D
        printf("  pop rax\n");
        if (node->rhs->lhs) {
            comment("  # FOR:C\n");
            gen(node->rhs->lhs);//C
            printf("  pop rax\n");
        }
        printf("  jmp .Lbegin%03d\n", cnt);
        printf(".Lend%03d:\n", cnt);
    } else if (node->type == '=') {         //代入
        comment("  # '='\n");
        gen_lval(node->lhs);
        gen(node->rhs);
        printf("  pop rdi\n");  //rhsの値
        printf("  pop rax\n");  //lhsのアドレス
        printf("  mov [rax], rdi\n");
        printf("  push rdi\n");
    } else {                                //演算子
        //lhsとrhsを処理して結果をPUSHする
        gen(node->lhs);
        gen(node->rhs);

        //それらをPOPして、演算する
        printf("  pop rdi\n");  //rhs
        printf("  pop rax\n");  //lhs

        switch(node->type) {
        case ND_EQ: //"=="
            comment("  # '=='\n");
            printf("  cmp rax, rdi\n");
            printf("  sete al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_NE: //"!="
            comment("  # '!='\n");
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            printf("  movzb rax, al\n");
            break;
        case '<':   //'>'もここで対応（構文木作成時に左右入れ替えてある）
            comment("  # '<' or '>'\n");
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_LE: //"<="、">="もここで対応（構文木作成時に左右入れ替えてある）
            comment("  # '<=' or '>='\n");
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            printf("  movzb rax, al\n");
            break;
        case '+':
            comment("  # '+'\n");
            printf("  add rax, rdi\n");
            break;
        case '-':   //rax(lhs)-rdi(rhs)
            comment("  # '-'\n");
            printf("  sub rax, rdi\n");
            break;
        case '*':   //rax*rdi -> rdx:rax
            comment("  # '*'\n");
            printf("  mul rdi\n");
            break;
        case '/':   //rdx:rax(lhs) / rdi(rhs) -> rax（商）, rdx（余り）
            comment("  # '/'\n");
            printf("  mov rdx, 0\n");
            printf("  div rdi\n");
            break;
        case '%':   //rdx:rax / rdi -> rax（商）, rdx（余り）
            comment("  # '%%'\n");
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