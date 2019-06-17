# 9cc
低レイヤを知りたい人のためのCコンパイラ作成入門

https://www.sigbus.info/compilerbook/

# 目標

**セルフホスト**

## セルフホストに向けたTODOリスト

前提条件として、現在コード中に用いている記述は変更しない。コンパイルを通すためにあえて使用する構文を制限することもしない。ただし実装しなくても動作するもの（constなど）はあえて実装しなくてもよいものとする。

- 関数のポインタ
- ポインタ同士の減算（差分）
- 関数の引数の型チェック
- 関数の戻り値の型チェック
- 多次元配列の初期化
- typedef
- enum
- 型キャスト
- \による文字のエスケープ
- case文
- goto（多重ループから抜けるために使っているけど、コードを書き直してもよい）
- **構造体の実装**
  - .
  - ->
  - メンバのアラインメント
  - 初期値
  - 配列の初期値（構造体）
- プリプロセッサを通す
- セルフホスト用に最低限のヘッダファイルを用意する

## 実装予定なし

- ローカル配列のサイズが定数でないケース。
- [指示付きの初期化子 (Designated Initializer)](http://seclan.dll.jp/c99d/c99d07.htm#dt19991025)
- 実数

## その他、制限事項・バグなど

- トップレベルに空の ; があるとエラーになる（構文規則的になぜOKなのか不明）。
- ブロックコメントの終了判定が雑
- 文字列リテラルの途中に明示的な'\0'があると以降が無視される。
- ローカル変数の初期値として文字列リテラルを指定すると.dataにコピーが作られ、実行時にそこからコピーする動作になっている。

## [Cの仕様書](http://port70.net/~nsz/c/c11/n1570.html#A)を読むときのメモ

- `xxx-list` は `xxx` をカンマ `,` で並べたもので、9ccのBNFでは `xxx ( , xxx)*` で記載している。
- `xxx` から先頭の `pointer*` をなくしたものが `direct_xxx`。
例： `declarator = pointer* direct_declarator`
- `declarator` と　`abstract-declarator` の違いは前者には `identifier` を含むが、後者は含まないこと。
例： `int *p` と `int *`