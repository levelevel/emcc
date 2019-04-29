//可変長ベクタ ---------------------------------------
typedef struct {
    void **data;
    int capacity;
    int len;
} Vector;

// util.c
Vector *new_vector(void);
void vec_push(Vector *vec, void *elem);
void run_test(void);
