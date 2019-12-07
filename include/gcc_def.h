//gccの組み込み予約語を無効にする
#define __const const
#define __restrict restrict
#define __inline inline
#define __builtin_va_list void *
#define __builtin_va_start(a, b) (0)
#define __builtin_va_end(a) (0)
#define __builtin_va_arg(ar, t) (0)
#define __builtin_va_copy(d, s) (0)
#define __builtin_offsetof(type, member) (0)
#define __alignof__(type) (0)
#define __attribute__(a)
#define __asm__(a)
#define __extension__
