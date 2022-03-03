ssu_sindex : main.o ssu_sindex.o
	gcc main.o ssu_sindex.o -o ssu_sindex

main.o : main.c ssu_sindex.h
	gcc -c main.c

ssu_sindex.o : ssu_sindex.c ssu_sindex.h
	gcc -c ssu_sindex.c

clean :
	rm *.o
	rm ssu_sindex