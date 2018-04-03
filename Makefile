all: resource_request_simulator.c 
	gcc -o p3.o resource_request_simulator.c 

q2: part2_skeleton.c
	gcc -o p2.o part2_skeleton.c

debug2: part2_skeleton.c
	gcc -g part2_skeleton.c

debug3: resource_request_simulator.c 
	gcc -DDEBUG -g resource_request_simulator.c 

clean2:
	rm ./p2.o

clean3:
	rm ./p3.o

cleanD:
	rm ./a.out
