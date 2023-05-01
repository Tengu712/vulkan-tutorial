@echo off

if "%1"=="" (
    echo "no file input"
) else (
    gcc .\src\%1*.c .\src\common\debug.c -L. -lglfw3 -lvulkan-1
)
