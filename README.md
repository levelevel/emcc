# 9cc : 自作Cコンパイラの作成

参考サイト：[低レイヤを知りたい人のためのCコンパイラ作成入門](https://www.sigbus.info/compilerbook/)

# 目標

**セルフホスト**（自作コンパイラでそれ自身のソースコードをコンパイルできるようにすること）

## 基本方針

普通にCコンパイラを実装するために必要な構文はすべて実現する（そうしないとセルフホストできない）。セルフホストのために意図的に使用する構文を制限することはしない。ただし実装しなくても動作するものはあえて実装しなくてもよいものとする（エラーチェックなど）。

## セルフホストに向けたTODOリスト

- 引数の数が6より多い関数
- **構造体・共用体の実装**
  - ビットフィールド
  - 配列の初期値（構造体）
- プリプロセッサを通す
- セルフホスト用に最低限のヘッダファイルを用意する

## 当面実装予定なし

- より厳密なC標準準拠（GCCの-std=c11 -pedantic-errors相当）
- 可変長配列（Variable Length Array: VLA）
- [指示付きの初期化子 (Designated Initializer)](http://seclan.dll.jp/c99d/c99d07.htm#dt19991025)
- 実数（float, double, long double, _Complex, _Imaginar）
- プリプロセッサ

## 独自拡張
- 関数の最後に明示的なreturnが無い場合でも、最後に評価した値を返す。
- トップレベルの空の ; を許可する（ほとんどのコンパイラが独自拡張している）。
- typeof演算子

## その他、制限事項・バグなど

- [複合リテラル (Compound Literal)](http://seclan.dll.jp/c99d/c99d07.htm#dt19991101)
- 暗黙の型変換
- 戻り値のない非void関数のチェックが雑。

## [Cの仕様書](http://port70.net/~nsz/c/c11/n1570.html#A)を読むときのメモ

- `xxx-list` は `xxx` をカンマ `,` で並べたもので、9ccのBNFでは `xxx ( , xxx)*` で記載している。
- `xxx` から先頭の `pointer*` をなくしたものが `direct_xxx`。
例： `declarator = pointer* direct_declarator`
- `declarator` と　`abstract-declarator` の違いは前者には `identifier` を含むが、後者は含まないこと。
例： `int *p` と `int *`