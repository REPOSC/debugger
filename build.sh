#!/bin/bash
[[ "$1" == "clean" ]] && rm -rf *.o *.out && exit
nasm -f elf64 -o hello.o hello.S && gcc hello.o -o hello.out -nostdlib && g++ main.cpp -I. -L. -lBeaEngine_s_d_l -o main.out