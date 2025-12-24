all: part1 part2

part1: part1.c
	gcc -Wall -g part1.c -o part1 -lalgds

part2: part2.c
	gcc -march=native -O3 -Wall -g part2.c -o part2 -lalgds

1: part1
	cat input | ./part1

2: part2
	cat input | ./part2