#!/bin/bash
[[ "$1" == "clean" ]] && rm -rf *.o *.out *.txt && exit
[[ "$1" == 32 ]] && gcc -O0 -nostdlib -m32 hello.c -o hello.out && g++ main.cpp -I. -L. -lBeaEngine_s_d_l -o main.out && exit
[[ "$1" == 64 ]] && gcc -O0 -nostdlib -m64 hello.c -o hello.out && g++ main.cpp -I. -L. -lBeaEngine_s_d_l -o main.out && exit
#do nothing and exit