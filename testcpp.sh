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
  system("'$CPP' " f_in " > " f_out);
  ret = system("diff -c " f_out " " f_expect " > " f_diff);
  if (ret) {
    ng_cnt++;
    print "ERROR:" name;
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
    name = cnt "_" $2;
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
#if ONE
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
#define AB a+B
#define B b
  AB
@expect


  a+b
@in define_noarg_5 ===================
#define A A B
  A /*- A -*/ A
@expect

  A B /*- A -*/ A B
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
@ =========================
EOF

