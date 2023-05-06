.PHONY: 00 01 02 03 04 clean

out=./build/a.out
opt=-lglfw3 -lvulkan -lm
cln=rm -rf ./build/a.out ./build/*.spv

ifeq ($(OS),Windows_NT)
	out=./build/a.exe
	opt=-L./build/ -lglfw3 -lvulkan-1
	cln=del .\build\a.exe .\build\*.spv
endif

00:
	gcc -o $(out) ./src/00-window/main.c ./src/common/debug.c $(opt)
01:
	gcc -o $(out) ./src/01-minimum/main.c ./src/common/debug.c $(opt)
02:
	gcc -o $(out) ./src/02-device/main.c ./src/common/debug.c $(opt)
03:
	gcc -o $(out) ./src/03-clear-screen/main.c ./src/common/debug.c $(opt)
04:
	glslc -o ./build/shader.vert.spv ./src/04-triangle/shader.vert
	glslc -o ./build/shader.frag.spv ./src/04-triangle/shader.frag
	gcc -o $(out) ./src/04-triangle/main.c ./src/common/debug.c ./src/common/read_bin.c ./src/common/buffer.c $(opt)
clean:
	$(cln)
