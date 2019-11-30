#!/bin/bash
set -u

TESTDIR=./tmpcpp

if [ ! -e $TESTDIR ]; then mkdir $TESTDIR; fi
rm -f $TESTDIR/*

awk \
-e '
function run_test() {
  system("./emcpp " f_in " > " f_out);
  ret = system("diff -c " f_out " " f_expect " > " f_diff);
  if (ret) {
    ng_cnt++;
    print "ERROR:" name;
    system("cat " f_diff);
  } else {
    ok_cnt++;
  }
}
BEGIN {
  dir = "'$TESTDIR'";
  name = "";
  ok_cnt = 0;
  ng_cnt = 0;
}
/^@in/ { 
    if (name != "") run_test();
    name = $2;
    if (name == "") { print "line:" NR " " $1": ファイル名がありません" | "cat 1>&2"; exit };
    base = dir "/" name;
    f_in     = base ".c";
    f_expect = base ".expect";
    f_out    = base ".out";
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
@in if1else ===================
#if 1
  TRUE
#else
  FALSE
#endif
@expect

  TRUE



@in if0else ===================
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

@in if1nest ===================
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


@ =========================
EOF

