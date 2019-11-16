void *func1(void){char*p; return p;}
char *func2(void){void*p; return p;}
void func3(void*p);
void func4(char*p);
int main() {
    char*cp;
    void*vp;
    func3(cp);
    func4(vp);
    cp=vp;
    vp=cp;
    return 1;
}
