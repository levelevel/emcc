#ifndef __BUILTIN_H
#define __BUILTIN_H

#ifdef _emcc
//gccの組み込み予約語を無効にする
#define __const const
#define __restrict restrict
#define __inline inline
#define __builtin_offsetof(type, member) (0)
#define __alignof__(type) (0)
#define __attribute__(a)
#define __asm__(a)
#define __extension__
#define __PRETTY_FUNCTION__ __func__
#endif

//組み込みstdarg

typedef unsigned long __emcc_uint64_t;
typedef unsigned int  __emcc_uint32_t;

typedef struct {
    __emcc_uint64_t i[2];
} __emcc_uint128_t;

// 関数定義：void func(char *fmt, ...) {};
// 関数コール：func(fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
typedef struct __emcc_save_args {
    struct {
        __emcc_uint64_t arg0_rdi;
        __emcc_uint64_t arg1_rsi;
        __emcc_uint64_t arg2_rdx;
        __emcc_uint64_t arg3_rcx;
        __emcc_uint64_t arg4_r8;
        __emcc_uint64_t arg5_r9;
    };
    struct {
        __emcc_uint128_t xmm0;
        __emcc_uint128_t xmm1;
        __emcc_uint128_t xmm2;
        __emcc_uint128_t xmm3;
        __emcc_uint128_t xmm4;
        __emcc_uint128_t xmm5;
        __emcc_uint128_t xmm6;
        __emcc_uint128_t xmm7;
    };
} __emcc_save_args;

//va_listの実体
typedef struct __va_list_tag {
    unsigned int gp_offset;     //先頭引数のオフセット（固定引数が2個であれば8*2）。va_arg()する度に+8する。
    unsigned int fp_offset;     //xmm0のオフセット
    void *overflow_arg_area;    //スタック渡し引数のアドレス（初期値はrbp+16=arg6）。va_arg()する度に+8する。
    void *reg_save_area;        //arg0_rdiのアドレス。gp_offsetを加算することで現在のパラメータにアクセスできる。
} __va_list_tag;

typedef enum {
    BUILTIN_NULL,
    BUILTIN_VA_START,       //組み込み関数(builtin function)
    BUILTIN_VA_ARG,  
    BUILTIN_VA_COPY,
} BuiltinFuncType;

#ifdef _emcc
//va_list/va_start/va_arg/va_end/va_copyを再定義
typedef __va_list_tag __builtin_va_list[1];
#define __builtin_va_start(ap, last)  __emcc_builtin_va_start(ap)
#define __builtin_va_arg(ap, type)    (*(type*)__emcc_builtin_va_arg(ap))
#define __builtin_va_end(ap)          (0)
//#define __builtin_va_copy(d, s)       __emcc_builtin_va_copy(d,s)
#define __builtin_va_copy(d, s)       d[0]=s[0]
#endif

void __emcc_builtin_va_start(__va_list_tag *ap);
void*__emcc_builtin_va_arg(__va_list_tag *ap);
//void __emcc_builtin_va_copy(__va_list_tag *ap_d, __va_list_tag *ap_s);

#endif  //__BUILTIN_H
