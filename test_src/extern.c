
//グローバル変数
char  g_extern_c = 1;
short g_extern_s = 2;
int   g_extern_i = 3;
long  g_extern_l = 4;

static char  g_static_c=0;
static short g_static_s=0;
static int   g_static_i=0;
static long  g_static_l=0;

char g_extern_ac6[] = "ABCDE";
int  g_extern_ai4[] = {0, 10, 20, 30};
long g_extern_al4[] = {1,2,3,4};

char *g_extern_pc = g_extern_ac6;
int  *g_extern_pi = g_extern_ai4;
int  *g_extern_pl = &g_extern_al4;
