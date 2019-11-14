# 9cc : 自作Cコンパイラの作成

参考サイト：[低レイヤを知りたい人のためのCコンパイラ作成入門](https://www.sigbus.info/compilerbook/)

# 目標

**セルフホスト**（自作コンパイラでそれ自身のソースコードをコンパイルできるようにすること）

## 基本方針

セルフホストのために必要な構文はすべて実現する。セルフホストのために意図的に使用する構文を制限することはしない。
基本的なエラーチェックは警告レベルも含めて実装する。

## セルフホストに向けたTODOリスト

- **構造体・共用体の実装**
  - 配列の初期値（構造体）
  - 関数の引数・戻り値での値渡し
- 外部のプリプロセッサを通す（または内製する）
- セルフホスト用に最低限のヘッダファイルを用意する（または本物を読み込む）

## 当面実装予定なし

- ビットフィールド
- 可変長配列（Variable Length Array: VLA）
- [指示付きの初期化子 (Designated Initializer)](http://seclan.dll.jp/c99d/c99d07.htm#dt19991025)
- 実数（float, double, long double, _Complex, _Imaginar）
- [複合リテラル (Compound Literal)](http://seclan.dll.jp/c99d/c99d07.htm#dt19991101)

## 独自拡張
- 関数の最後に明示的なreturnが無い場合でも、最後に評価した値を返す。
- トップレベルの空の ; を許可する（ほとんどのコンパイラが独自拡張している）。
- typeof演算子

## その他、制限事項・バグなど

- 現状では厳密なC標準準拠（GCCの-std=c11 -pedantic-errors相当）ではなく、一部のレガシー記述を受け入れる実装となっている。オプションで切り替えるべき。
- 引数の数が6より多い関数は未対応
- 厳密な暗黙の型変換は未考慮（現状に対して修正が必要かどうかも不明）
- 戻り値のない非void関数のチェックが雑。

## [Cの仕様書](http://port70.net/~nsz/c/c11/n1570.html#A)を読むときのメモ

- `xxx-list` は `xxx` をカンマ `,` で並べたもので、9ccのBNFでは `xxx ( , xxx)*` で記載している。
- `xxx` から先頭の `pointer*` をなくしたものが `direct_xxx`。
例： `declarator = pointer* direct_declarator`
- `declarator` と　`abstract-declarator` の違いは前者には `identifier` を含むが、後者は含まないこと。
例： `int *p` と `int *`
