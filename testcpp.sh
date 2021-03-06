#!/bin/bash
set -u

function Usage() {
  echo "Usage: testcpp.sh [emcpp|emcpp2]"
  exit 1
}

TESTDIR=./tmp_emcpp
CPP=./emcpp

while [ $# -gt 0 ]; do
  case $1 in
  emcpp*)
    TESTDIR=./tmp_$1
    CPP=./$1 ;;
  *) Usage; exit 1 ;;
  esac
  shift
done

if [ ! -e $CPP ]; then
  echo "ERROR: $CPP not found"
  exit 1
fi

if [ ! -e $TESTDIR ]; then mkdir $TESTDIR; fi
rm -f $TESTDIR/*

awk \
-e '
function run_test() {
  #system("cpp " f_in " | grep -v ^# > " f_outgcc);
  system("'$CPP' " f_in " > " f_out " 2>&1");
  ret = system("diff -c " f_expect " " f_out " > " f_diff);
  if (ret) {
    ng_cnt++;
    print "ERROR:" name;
    system("cat " f_in);
    system("cat " f_diff);
  } else {
    ok_cnt++;
    system("rm " f_diff);
  }
}
BEGIN {
  dir = "'$TESTDIR'";
  name = "";
  cnt = 0;
  ok_cnt = 0;
  ng_cnt = 0;
}
/^@in/ { 
    if (name != "") run_test();
    cnt++;
    #name = cnt "_" $2;
    name = $2
    if (name == "") { print "line:" NR " " $1": ファイル名がありません" | "cat 1>&2"; exit };
    base = dir "/" name;
    f_in     = base ".c";
    f_expect = base ".expect";
    f_out    = base ".out";
    f_outgcc = base ".out.gcc";
    f_diff   = base ".diff";
    fname = f_in;
    #print fname;
    next;
  }
/^@expect/ {
    fname = f_expect;
    next;
  }
/^@/ {next;}
  {
    print >> fname 
  }
END {
    if (name != "") run_test();
    print "OK:" ok_cnt;
    print "NG:" ng_cnt;
  }' << EOF
@in comment1 ===================
1;  //comment1
a;  /*comment2*/
@expect
1;  //comment1
a;  /*comment2*/
@in if1_else ===================
#if 1   //comment
  TRUE
#else   //comment
  FALSE
#endif
@expect

  TRUE



@in if0_else ===================
#if 0
  FALSE
#else
  TRUE
#endif
@expect



  TRUE

@in elif1 ===================
#if 0
  FALSE
#elif 1
  TRUE
#else
  FALSE
#endif
@expect



  TRUE



@in elif0 ===================
#if 0
  FALSE
#elif 0
  FALSE
#else
  TRUE
#endif
@expect





  TRUE

@in if1_nest ===================
#if 1
  TRUE
  #if 0
    FALSE
  #else
    TRUE
  #endif
#else
  FALSE
  #if 1
    FALSE
  #else
    FALSE
  #endif
#endif
@expect

  TRUE



    TRUE









@in define_emcpp ===================
#ifdef _emcpp //組み込みマクロ
  TRUE
#endif
@expect

  TRUE

@in ifdef1 ===================
#define AAA
#ifdef AAA
  TRUE
#else
  FALSE
#endif
@expect


  TRUE



@in ifdef0 ===================
#ifdef AAA
  FALSE
#else
  TRUE
#endif
@expect



  TRUE

@in ifndef1 ===================
#ifndef AAA
  TRUE
#else
  FALSE
#endif
@expect

  TRUE



@in ifndef0 ===================
#define AAA
#ifndef AAA
  FALSE
#else
  TRUE
#endif
@expect




  TRUE

@in undef1 ===================
#define AAA
#undef  AAA
#ifdef AAA
  FALSE
#else
  TRUE
#endif
#undef BBB
@expect





  TRUE


@in define_noarg_1 ===================
#if ONE //未定義の識別子は0
  FALSE
#else
  TRUE
#endif
@expect



  TRUE

@in define_noarg_2 ===================
#define ONE 1 //comment
  ONE
@expect

  1
@in define_noarg_3 ===================
#define ABC (A B  C) //comment
  { ABC, ABC }
@expect

  { (A B  C), (A B  C) }
@in define_noarg_4 ===================
#define ABC a+BC  //多重展開
#define BC b+C
#define C c
  ABC
@expect



  a+b+c
@in define_noarg_5 ===================
#define A A B
  A /*- A -*/ A
@expect

  A B /*- A -*/ A B
@in define_noarg_5 ===================
#define AB \
  AAAAA;\
  BBBBB
123\
456;
AB;
@expect

123456;
AAAAA;  BBBBB;
@in define_arg0_1 ===================
#define A() a()
#define B() b
#define C (c)
  A() B() C() /*- A -*/
  A () B () C ()
  A B C
@expect



  a() b (c)() /*- A -*/
  a() b (c) ()
  A B (c)
@in define_arg1_1 ===================
/* 1
 * 2
 * 3 */
#define A(a) a
#define B( aa , bb , cc ) BBB(aa + bb-cc)
 A(arg);
 B(1,2,3);
 B ( "ab,c" , ('b' ) , (1 + (2,3)) );
// B (99,,)
@expect
/* 1
 * 2
 * 3 */


 arg;
 BBB(1 + 2-3);
 BBB("ab,c" + ('b' )-(1 + (2,3)) );
// B (99,,)
@in define_arg1_2 ===================
#define SUM(a,b,c) a + b + c
SUM(1,,3);  //実引数は省略可能
SUM( ,2, );
SUM( ,  , );
@expect

1 + b + 3;  //実引数は省略可能
a + 2 + c;
a + b + c;
@in define_arg1_3 ===================
#define ADD(a,b) ((a)+(b))  //多重展開
#define MUL(a,b) (a)*(b)
MUL(ADD(1,2)+10,ADD(3,4)/ADD(5,6));
@expect


(((1)+(2))+10)*(((3)+(4))/((5)+(6)));
@in define_arg1_4 ===================
#define x(a,b) x(a+1,b+1) + 4 //再帰的定義は1回だけ展開
x(20,10)
@expect

x(20+1,10+1) + 4
@in define_arg1_5 ===================
#define SQR(s)  ((s) * (s))
#define PRNT(a,b)   \
  printf("value 1 = %d¥n", a);\
  printf("value 2 = %d¥n", b)
PRNT(SQR(x),y);
@expect


printf("value 1 = %d¥n", ((x) * (x)));  printf("value 2 = %d¥n", y);
@in define_arg2_1 ===================
#define catval(a,b) a  ##  b
catval(1,L)+catval( 2 , UL );
@expect

1L+2UL;
@in define_arg2_2 ===================
#define ArgArg(x, y)          x##y    //##の演算は引数の展開より先
#define ArgArgArg(x, y, z)    x ## y ## z
#define ArgText(x)            x##TEXT
#define TextArg(x)            TEXT##x
#define TextText              TEXT##text
#define Jitter                1
#define bug                   2
#define Jitterbug             3
#define bookworld             BookWorld
ArgArg(lady, bug)
ArgArgArg(big, lady, bug)
ArgArg(big lady, bug book)
ArgArgArg(big lady, bug book, world wide)
ArgText(con)
TextArg(book)
TextText
ArgArg(Jitter, bug)
@expect









ladybug
bigladybug
big ladybug book
big ladybug BookWorld wide
conTEXT
TEXTbook
TEXTtext
3
@in define_arg3_1 ===================
#define STR(x)        #x
#define ONE           1
STR(  abc  )
STR(a b  c)
STR(\n\001)
STR("\n\001")
STR(\n  "\n"  '\n')
STR(ONE)
@expect


"abc"
"a b c"
"\n\001"
"\"\\\\n\\\\001\""
"\n \"\\\\n\" '\\n'"
"ONE"
@in define_arg3_2 ===================
#define STR2(x,y)     strcalc(#x,PLUS,#y)
#define PLUS          "+"
STR2(a,b)
STR2( a b c , X  Y  Z )
@expect


strcalc("a","+","b")
strcalc("a b c","+","X Y Z")
@in define_arg3_3 ===================
#define STR2(x)     # z
STR2(abc)
@expect
emcc:Error: ./tmp_emcpp/define_arg3_3.c:2: #define STR2(x)     # z
                                                               ^ #の次がマクロのパラメータではありません

@in define_arg3_4 ===================
#define STR(x)        #x
#define XSTR(x)       STR(x)
#define ONE           1
XSTR(a)
XSTR(ONE)
XSTR("hello")
@expect



"a"
"1"
"\"hello\""
@ =========================
EOF

