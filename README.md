# Simplest Vulkan Tutorial in Japanese

## Outline

速く・かつ正しくVulkanを学習するための日本語チュートリアル。

少なくとも、このリポジトリで管理されているすべてファイルは、CC0 パブリックドメインで配布しているため、自由に利用して構わない。

## Build

以下のツールが必要：

* git
* gcc
* glslc
* make

以下のサードパーティー製ライブラリが必要：

* [glfw3](https://www.glfw.org/)
* [vulkan](https://vulkan.lunarg.com/)

以下のようにこのリポジトリをクローン：

```
$ git clone https://github.com/Tengu712/Vulkan-Tutorial.git
$ cd Vulkan-Tutorial
```

ビルド結果を格納するディレクトリを作成：

```
Vulkan-Tutorial$ mkdir build
```

makeコマンドでサンプルプログラムをビルドして実行：

```
Vulkan-Tutorial$ make 00
Vulkan-Tutorial$ cd build
Vulkan-Tutorial/build$ ./a.out
```

ただし、OSがWindowsである場合は、glfw3.dllをbuildディレクトリ内に配置しておくこと。
また、生成される実行ファイル名は`a.exe`である。
