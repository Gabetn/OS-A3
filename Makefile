all: part2_skeleton.c
	gcc -o p2.o part2_skeleton.c

debug: part2_skeleton.c
	gcc -g part2_skeleton.c

clean:
	rm ./p2.o

cleanD:
	rm ./a.out
