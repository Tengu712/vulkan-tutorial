@echo off

set OPTION=

if "%1"=="" (
    echo "no file input"
) else if "%1"=="00" (
    gcc .\src\00-window\main.c .\src\common\debug.c -L. -lglfw3 -lvulkan-1
) else if "%1"=="01" (
    gcc .\src\01-minimum\main.c .\src\common\debug.c -L. -lglfw3 -lvulkan-1
) else if "%1"=="02" (
    gcc .\src\02-device\main.c .\src\common\debug.c -L. -lglfw3 -lvulkan-1
) else if "%1"=="03" (
    gcc .\src\03-clear-screen\main.c .\src\common\debug.c -L. -lglfw3 -lvulkan-1
) else if "%1"=="04" (
    glslc -o .\shader.vert.spv .\src\04-triangle\shader.vert
    glslc -o .\shader.frag.spv .\src\04-triangle\shader.frag
    gcc .\src\04-triangle\main.c .\src\common\debug.c .\src\common\read_bin.c -L. -lglfw3 -lvulkan-1
)
