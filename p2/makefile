ssu_sindex : main.o ssu_sindex.o option.o
	gcc main.o ssu_sindex.o option.o -o ssu_sindex

main.o : main.c ssu_sindex.h
	gcc -c main.c

ssu_sindex.o : ssu_sindex.c ssu_sindex.h option.h
	gcc -c ssu_sindex.c

option.o : option.c option.h
	gcc -c option.c

clean :
	rm *.o
	rm ssu_sindex