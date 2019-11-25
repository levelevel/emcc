#define XXX
#if 0x001
    Active
    #ifdef XXX
        ACTIVE
    #else
        NOT ACTIVE
    #endif
# else
    NOT Active
    #if 1
        NOT Sctive
    #endif
#endif
#if 0
    xxx
#elif 1
    Active
#endif
//コメント1
/* コメント2 */
int main(){
    	int a;
    return a+1;
}