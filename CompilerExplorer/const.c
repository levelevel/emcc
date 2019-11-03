char*func2(void) {
    static const char str[]="abc";
    return str; //warning: return discards 'const' qualifier from pointer target type [-Wdiscarded-qualifiers]
}
void func3(char*str){}
void func3(const char*str); //error: conflicting types for 'func3'
char *func4(void){}
const char *func4(void);    //error: conflicting types for 'func4'
void*func(void){
    static char str[]="abc";
    const int ci;
    ci = 1;     //error: assignment of read-only variable 'ci'

    const char * p1=str;
    *p1 = 'A';  //error: assignment of read-only location '*p1'
    (*p1)++;    //error: increment of read-only location '*p1'
    p1 = 0;
    func3(p1);  //warning: passing argument 1 of 'func3' discards 'const' qualifier from pointer target type [-Wdiscarded-qualifiers]

    char const* p2=str;
    *p2 = 'A';  //error: assignment of read-only location '*p2'
    p2 = 0;
    (*p2)++;    //error: increment of read-only location '*p2'
    func3(p2);  //warning: passing argument 1 of 'func3' discards 'const' qualifier from pointer target type [-Wdiscarded-qualifiers]

    const char const* p12=str;
    *p12 = 'A'; //error: assignment of read-only location '*p12'
    p12 = 0;
    (*p12)++;   //error: increment of read-only location '*p12'
    func3(p12);

    char * const p3=str;
    *p3 = 'A';
    (*p3)++;
    p3 = 0;     //error: assignment of read-only variable 'p3'
    func3(p3);

    const char * const p13=str;
    *p13 = 'A'; //error: assignment of read-only location '*p13'
    (*p13)++;   //error: increment of read-only location '*p13'
    p13 = 0;    //error: assignment of read-only variable 'p13'
    func3(p13);
}
