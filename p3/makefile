all : ssu_sfinder
ssu_sfinder : ssu_sfinder.o ssu_help.o ssu_find.o ssu_list.o
	gcc -Wall ssu_sfinder.o ssu_help.o ssu_find.o ssu_list.o -o ssu_sfinder -lcrypto -lssl -g

ssu_sfinder.o : ssu_sfinder.c ssu_sfinder.h
	gcc -c ssu_sfinder.c

ssu_help.o : ssu_help.c ssu_find.h
	gcc -c ssu_help.c

ssu_list.o : ssu_list.c ssu_find.h
	gcc -c ssu_list.c	

ssu_find.o : ssu_find.c ssu_find.h
	gcc -c ssu_find.c

clean :
	rm *.o
	rm ssu_sfinder