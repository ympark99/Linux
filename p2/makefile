all : ssu_sdup ssu_find-md5 ssu_help
ssu_sdup : main.o ssu_sdup.o
	gcc main.o ssu_sdup.o -o ssu_sdup

ssu_find-md5 : ssu_find-md5.o
	gcc -Wall ssu_find-md5.o -o ssu_find-md5 -lcrypto -lssl

ssu_help : ssu_help.o
	gcc ssu_help.o -o ssu_help

main.o : main.c ssu_sdup.h
	gcc -c main.c

ssu_sdup.o : ssu_sdup.c ssu_sdup.h
	gcc -c ssu_sdup.c

ssu_help.o : ssu_help.c ssu_help.h
	gcc -c ssu_help.c

ssu_find-md5.o : ssu_find-md5.c ssu_find-md5.h
	gcc -c ssu_find-md5.c

clean :
	rm *.o
	rm ssu_sdup
	rm ssu_find-md5
	rm ssu_help