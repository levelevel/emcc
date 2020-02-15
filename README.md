# emcc : 自作Cコンパイラの作成

参考サイト：[低レイヤを知りたい人のためのCコンパイラ作成入門](https://www.sigbus.info/compilerbook/)

# 目標

**セルフホスト**（自作コンパイラでそれ自身のソースコードをコンパイルできるようにすること）

## 基本方針

- セルフホストのために必要な構文はすべて実現する。
- セルフホストのために意図的に使用する構文を制限することはしない。
- 基本的なエラーチェックは警告レベルも含めて実装する。

## セルフホストに向けたTODOリスト

- 2020/2/4達成！
- emcpp: #include、可変長引数マクロ、標準定義マクロ、プラグマ

## セルフホストに必須ではないが実装したいもの

- プリプロセッサ（emcpp） 
- プリプロセッサ、コンパイラ、アセンブラ、リンカの統合
- 関数の引数・戻り値での構造体の値渡し
- ビットフィールド
- [デバッグ情報](https://ja.osdn.net/projects/drdeamon64/wiki/DWARF%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%E3%83%95%E3%82%A9%E3%83%BC%E3%83%9E%E3%83%83%E3%83%88)

## 当面実装予定なし

- [複合リテラル (Compound Literal)](http://seclan.dll.jp/c99d/c99d07.htm#dt19991101)
- [指示付きの初期化子 (Designated Initializer)](http://seclan.dll.jp/c99d/c99d07.htm#dt19991025)
- 実数（float, double, long double, _Complex, _Imaginar）
- 可変長配列（Variable Length Array: VLA）

## 独自拡張
- 関数の最後に明示的なreturnが無い場合でも、最後に評価した値を返す。
- トップレベルの空の ; を許可する（ほとんどのコンパイラが独自拡張している）。
- typeof演算子

## その他、制限事項・バグなど

- 現状では厳密なC標準準拠（GCCの-std=c11 -pedantic-errors相当）ではなく、一部のレガシー記述を受け入れる実装となっている。オプションで切り替えるべき。
- 戻り値のない非void関数のチェックが雑。

## [Cの仕様書](http://port70.net/~nsz/c/c11/n1570.html#A)を読むときのメモ

- `xxx-list` は `xxx` をカンマ `,` で並べたもので、9ccのBNFでは `xxx ( , xxx)*` で記載している。
- `xxx` から先頭の `pointer*` をなくしたものが `direct_xxx`。
例： `declarator = pointer* direct_declarator`
- `declarator` と　`abstract-declarator` の違いは前者には `identifier` を含むが、後者は含まないこと。
例： `int *p` と `int *`
