# affine transform

## Outline

![](https://skdassoc.com/img/outer/Vulkan-Tutorial-05-affine-transform.png)

複数のモデルを描画する。

頂点シェーダへプッシュコンスタントを渡し、頂点シェーダ内で頂点座標変換(ビュー変換と射影変換を除く)を行う。

## Additions

* プッシュコンスタント
* 頂点座標変換

## Method

描画コマンド前にプッシュコンスタントをバインドするコマンドを積む。

頂点シェーダ内でプッシュコンスタントをもとに座標変換行列を生成し、ローカル座標に乗算して座標変換する。
