---
marp: true
paginate: true
size: 16:9
style: |
  section {
    justify-content: normal;
  }
  section.top h1 {
    font-size: 2em;
    margin: 0px;
  }
  section.top h2 {
    margin: 0px;
  }
  section.top {
    text-align: center;
    justify-content: center;
  }
  section.section h1 {
    position: absolute;
    top: 450px;
    font-size: 2em;
  }
  strong {
    color: red;
  }
---

<!-- _class: top -->
# Vulkan Tutorial Better Than ChatGPT

## 天狗(Tengu712)

---

<!-- _class: section -->
# おことわり

---

# 本スライドのコンセプト

**網羅率を代償に、正しさを持って速習すること。**

雑に新しいものを学ぶならChatGPTを用いるのが早い。
また、Vulkanは文献が乏しいながら、皆無というわけではない。

しかし、ChatGPTは勿論嘘を吐くし、
解説書は specific なプログラムを組む場合がある(一つの物体しか扱わないとか)。

本スライドでは、それらに比べ、正しく・一般的な理解を促進したい。

---

# 想定の対象

次の程度のリテラシーが大前提となっています：
* C言語が分かる
* 行列の積が分かる
* コンピュータアーキテクチャがある程度分かる
* 「DirectX」とか「シェーダ」くらいは聞いたことがある

---

# サンプルコード

本スライドには断片的にしか掲載しません。
適宜以下のリンクを参照してください。
https://github.com/Tengu712/Vulkan-Tutorial

尚、**独特**なコーディング規則について、以下のよう：

* 列数に上限なし
* ifの分岐後命令が一つなら中括弧なし
* ifの分岐後命令が一つかつbreak、continue、returnなら改行
* 構造体の実体は初期化子で初期化
* 初期化子内は余程短くない限り改行

---

# 参考文献

どのくらい参考したかはともかく、ぼくがVulkanを勉強する上で参考にした文献：

* Fadis(FADIS PRESS)『3DグラフィクスAPI Vulkanを出来るだけやさしく解説する本』
* すらりん(すらりんラボ)『Vulkan Programming Vol.1』

局所的には、各頁に示す。

---

<!-- _class: section -->
# Vulkan概要

---

# Vulkanとは

**グラフィックスAPI**の一種。
OpenGLの後継。従来のAPIより低水準で自由。

---

# グラフィックスAPIとは

主に**レンダリング**を目的としたGPUを扱うための**API**。

「なぜAPIを介すのか？」
GPUのアーキテクチャは非公開であることが多く、アセンブリを書くのが現実的でないから。

「なぜGPUを使うのか？」
現状の並列計算力を比較してCPUよりGPUの方がレンダリング処理に強いから

---

# レンダリングとは

画面に図形を描画すること。手法は色々考えられる。

主要グラフィックスAPIでは、一つの対象に対してパイプライン処理を行う。
**レンダリングパイプライン**と言う。

---

# レンダリングパイプライン

グラフィックスAPIによる、GPUの扱い方。
従って、利用可不可はGPUのアーキテクチャにも依存する。

多くは大雑把に以下のよう：

1. インプットアセンブラ
2. ヴァーテックスシェーダ
3. ビューポート変換
4. ラスタライゼーション
5. フラグメントシェーダ
6. 合成

---

# Vulkanのレンダリングパイプライン

参考元：
https://www.khronos.org/files/vulkan11-reference-guide.pdf

![bg right:65% height:95%](./img/vulkan-rendering-pipeline-diagram.svg)

---

# 今回扱う部分

![bg right:65% height:95%](./img/vulkan-rendering-pipeline-diagram-in-tutorial.svg)

---

# プログラマは何をすればいいか

Vulkanを**設定**して、Vulkanを介してGPUを扱う。

難しいアルゴリズムを考える必要は皆無。とにかく仕様と睨めっこ。

---

<!-- _class: section -->
# 空のアプリ

---

# ウィンドウ作成

解説の都合上GLFWを用いる。
普通、WindowsならWin32API、LinuxならXlibを用いる。

GLFWとは、端的に、OpenGLを扱いやすくするためのライブラリ。
Windows/Linux/macOSでウィンドウを作れる。

GLFW：
https://www.glfw.org/

---

