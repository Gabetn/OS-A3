all: faulty_request_simulator.c 
	gcc -o p3b.o faulty_request_simulator.c -lpthread

q3a: resource_request_simulator.c 
	gcc -o p3a.o resource_request_simulator.c -lpthread

q2: 260679520_part2.c
	gcc -o p2.o 260679520_part2.c

debug2: 260679520_part2.c
	gcc -g 260679520_part2.c

debug3a: resource_request_simulator.c 
	gcc -DDEBUG -g resource_request_simulator.c -lpthread

debug3b: faulty_request_simulator.c 
	gcc -DDEBUG -g faulty_request_simulator.c -lpthread

clean2:
	rm ./p2.o

clean3:
	rm ./p3.o

cleanD:
	rm ./a.out
