//可変長ベクタ ---------------------------------------
typedef struct {
    void **data;
    int capacity;
    int len;
} Vector;

Vector *new_vector(void);
Vector vec_push(Vector *vec, void *elem);
