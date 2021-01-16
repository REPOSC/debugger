#!/bin/bash
[[ "$1" == "clean" ]] && rm -rf *.o *.out && exit
[[ "$1" == "c" && "$2" == 32 ]] && gcc -O0 -m32 hello.c -o hello.out && g++ main.cpp -I. -L. -lBeaEngine_s_d_l -o main.out && exit
[[ "$1" == "s" && "$2" == 32 ]] && nasm -f elf32 -o hello.o hello.S && gcc -O0 -m32 hello.o -o hello.out -nostdlib && g++ main.cpp -I. -L. -lBeaEngine_s_d_l -o main.out && exit
[[ "$1" == "c" && "$2" == 64 ]] && gcc -O0 -m64 hello.c -o hello.out && g++ main.cpp -I. -L. -lBeaEngine_s_d_l -o main.out && exit
[[ "$1" == "s" && "$2" == 64 ]] && nasm -f elf64 -o hello.o hello.S && gcc -O0 -m64 hello.o -o hello.out -nostdlib && g++ main.cpp -I. -L. -lBeaEngine_s_d_l -o main.out && exit