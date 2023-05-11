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
  img[alt~="center"] {
    display: block;
    margin: 1em auto;
  }
---

<!-- _class: top -->
# Simplest Vulkan Tutorial

## 天狗(Tengu712)

---

<!-- _class: section -->
# はじめに

---

# コンセプト

**網羅率を代償に、正しさを持って、簡単に速習すること。**

他のチュートリアルでは軽視されがちな「理論」の部分に重点を置く。
読みやすく、わかりやすく、試しやすい、を意識している。

当スライドを見てもプログラムは組めない。
当スライドを見て、プログラムを俯瞰できるようにしてほしい。

---

# 想定の対象者層

次の程度のリテラシーは欲しい：

* C言語が読める
* 行列の積が分かる
* コンピュータアーキテクチャが少し分かる

---

# サンプルコード

本スライドには断片的にしか掲載しない。適宜以下のリンクを参照してほしい。
https://github.com/Tengu712/Vulkan-Tutorial

尚、**独特**なコーディング規則について、以下のよう：

* 列数に上限なし
* ifの分岐後命令が一つなら中括弧なし
* ifの分岐後命令が一つかつbreak、continue、returnなら改行
* 構造体の実体は初期化子で初期化
* 初期化子内は余程短くない限り改行
* 必要以上に関数・モジュール分割しない

---

# 参考文献

どのくらい参考したかはともかく、ぼくがVulkanを勉強する上で参考にした公式文献を除く文献：

* すらりん『Vulkan Programming Vol.1』
* Fadis『3DグラフィクスAPI Vulkanを出来るだけやさしく解説する本』
* きてらい「やっていくVulkan入門」
* Alexander Overvoorde「VulkanTutorial」
* vblanco20-1「VulkanGuide」

局所的には、各頁に示す。

---

# RenderDoc

グラフィックプログラミングをしていると、
コンパイルエラーもランタイムエラーもないが映らない、
なんてことがしょっちゅうある。

RenderDocを使うと以下を確認できたりするため、利用すべき：

* カラーバッファやデプスバッファ
* 各ステートの設定
* 各シェーダの入力と出力
* デプステストの結果


---

<!-- _class: section -->
# Vulkan概要

---

# Vulkanとは

**グラフィックスAPI**の一種。
OpenGLの後継。従来のAPIより低水準で自由。

![center width:600](https://upload.wikimedia.org/wikipedia/commons/f/fe/Vulkan_logo.svg)

![center width:600](https://upload.wikimedia.org/wikipedia/commons/0/0b/Khronos_Group_logo.svg)

---

# グラフィックスAPIとは

主に**レンダリング**を目的とした、GPUを扱うための**API**。

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

描画対象を処理する工程。多くは大雑把に以下のよう：

1. インプットアセンブラ
2. ヴァーテックスシェーダ
3. ビューポート変換
4. ラスタライゼーション
5. フラグメントシェーダ
6. 合成

---

# Vulkanのレンダリングパイプライン

概ね右の通り。

参考元：
Khronos Group, Vulkan 1.1 Quick Reference

![bg right:65% height:95%](./img/vulkan-rendering-pipeline-diagram.svg)

---

# 今回扱う部分

![bg right:65% height:95%](./img/vulkan-rendering-pipeline-diagram-in-tutorial.svg)

---

# プログラマは何をすればいいか

**Vulkanに詳細な設定を与えて、Vulkanを介してGPUに計算させる。**

難しいアルゴリズムを考える必要は皆無。とにかく仕様と睨めっこ。

---

# イメージ図

