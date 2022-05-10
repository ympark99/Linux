all : ssu_sfinder ssu_find-md5 ssu_find-sha1
ssu_sfinder : ssu_sfinder.o ssu_help.o
	gcc ssu_sfinder.o ssu_help.o -o ssu_sfinder

ssu_find-md5 : ssu_find-md5.o
	gcc -Wall ssu_find-md5.o -o ssu_find-md5 -lcrypto -lssl

ssu_find-sha1 : ssu_find-sha1.o
	gcc -Wall ssu_find-sha1.o -o ssu_find-sha1 -lcrypto -lssl

ssu_sfinder.o : ssu_sfinder.c ssu_sfinder.h ssu_help.h
	gcc -c ssu_sfinder.c

ssu_help.o : ssu_help.c ssu_help.h
	gcc -c ssu_help.c

ssu_find-md5.o : ssu_find-md5.c ssu_find-md5.h
	gcc -c ssu_find-md5.c

ssu_find-sha1.o : ssu_find-sha1.c ssu_find-sha1.h
	gcc -c ssu_find-sha1.c

clean :
	rm *.o
	rm ssu_sfinder
	rm ssu_find-md5
	rm ssu_find-sha1