.PHONY: 00 01 02 03 04 05 clean

out=./build/a.out
opt=-lglfw -lvulkan -lm
cln=rm -rf ./build/a.out ./build/*.spv

ifeq ($(OS),Windows_NT)
    out=./build/a.exe
    opt=-L./build/ -lglfw3 -lvulkan-1
    cln=del .\build\a.exe .\build\*.spv
else ifeq ($(shell type lsb_release > /dev/null 2>&1 && lsb_release -i -s),Ubuntu)
    opt=-lglfw3 -lvulkan -lm
endif

ifneq ($(RELEASE),)
    opt+=-D RELEASE
endif

00:
	gcc -o $(out) ./src/00-window/main.c ./src/common/debug.c $(opt)
01:
	gcc -o $(out) ./src/01-minimum/main.c ./src/common/debug.c $(opt)
02:
	gcc -o $(out) ./src/02-device/main.c ./src/common/debug.c $(opt)
03:
	gcc -o $(out) ./src/03-command/main.c ./src/common/debug.c $(opt)
04:
	gcc -o $(out) ./src/04-clear-screen/main.c ./src/common/debug.c $(opt)
05:
	glslc -o ./build/shader.vert.spv ./src/05-triangle/shader.vert
	glslc -o ./build/shader.frag.spv ./src/05-triangle/shader.frag
	gcc -o $(out) ./src/05-triangle/main.c ./src/common/debug.c ./src/common/read_bin.c ./src/common/buffer.c $(opt)
06:
	glslc -o ./build/shader.vert.spv ./src/06-affine-transform/shader.vert
	glslc -o ./build/shader.frag.spv ./src/06-affine-transform/shader.frag
	gcc -o $(out) ./src/06-affine-transform/main.c ./src/common/debug.c ./src/common/read_bin.c ./src/common/buffer.c $(opt)
07:
	glslc -o ./build/shader.vert.spv ./src/07-camera/shader.vert
	glslc -o ./build/shader.frag.spv ./src/07-camera/shader.frag
	gcc -o $(out) ./src/07-camera/main.c ./src/common/debug.c ./src/common/read_bin.c ./src/common/buffer.c $(opt)
08:
	glslc -o ./build/shader.vert.spv ./src/08-image/shader.vert
	glslc -o ./build/shader.frag.spv ./src/08-image/shader.frag
	gcc -o $(out) ./src/08-image/main.c ./src/common/debug.c ./src/common/read_bin.c ./src/common/buffer.c ./src/common/image.c $(opt)
09:
	glslc -o ./build/shader.vert.spv ./src/09-cube/shader.vert
	glslc -o ./build/shader.frag.spv ./src/09-cube/shader.frag
	gcc -o $(out) ./src/09-cube/main.c ./src/common/debug.c ./src/common/read_bin.c ./src/common/buffer.c ./src/common/image.c $(opt)
clean:
	$(cln)
