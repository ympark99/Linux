all : ssu_sfinder
ssu_sfinder : ssu_sfinder.o ssu_help.o ssu_find.o
	gcc -Wall ssu_sfinder.o ssu_help.o ssu_find.o -o ssu_sfinder -lcrypto -lssl

ssu_sfinder.o : ssu_sfinder.c ssu_sfinder.h ssu_help.h
	gcc -c ssu_sfinder.c

ssu_help.o : ssu_help.c ssu_help.h
	gcc -c ssu_help.c

ssu_find.o : ssu_find.c ssu_find.h
	gcc -c ssu_find.c

clean :
	rm *.o
	rm ssu_sfinder