all : ssu_sfinder
ssu_sfinder : ssu_sfinder.o ssu_find.o ssu_list.o ssu_trash.o ssu_restore.o
	gcc -Wall ssu_sfinder.o ssu_find.o ssu_list.o ssu_trash.o ssu_restore.o -o ssu_sfinder -lcrypto -lssl

ssu_sfinder.o : ssu_sfinder.c ssu_find.h
	gcc -c ssu_sfinder.c

ssu_list.o : ssu_list.c ssu_find.h
	gcc -c ssu_list.c	

ssu_trash.o : ssu_trash.c ssu_find.h
	gcc -c ssu_trash.c	

ssu_restore.o : ssu_restore.c ssu_find.h
	gcc -c ssu_restore.c	

ssu_find.o : ssu_find.c ssu_find.h
	gcc -c ssu_find.c

clean :
	rm *.o
	rm ssu_sfinder