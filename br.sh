#!/bin/sh
rm my_robots.o
rm my_robots.exe
gcc my_robots.c -Wall -O -g -c -o my_robots.o
gcc -O -g -o my_robots.exe my_robots.o -lncurses
