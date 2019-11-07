struct ST{
    int a;
    union {
        int ua;
        long ul;
        char*up;
        struct {
            int x,y,z;
        };
    };
}st={0};
struct ST func2(void);
void func(void) {
    int a = st.a;
    int b = st.ua;
    int l = st.ul;
    int x = st.x;
    int y = st.y;
    int z = st.z;
    st = func2();
    st = st;
}