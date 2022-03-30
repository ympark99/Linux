ssu_sdup : main.o ssu_sdup.o ssu_help.o ssu_find-md5.o
	gcc -Wall main.o ssu_sdup.o ssu_help.o ssu_find-md5.o -o ssu_sdup -lcrypto -lssl

main.o : main.c ssu_sdup.h
	gcc -c main.c

ssu_sdup.o : ssu_sdup.c ssu_sdup.h ssu_help.h
	gcc -c ssu_sdup.c

ssu_help.o : ssu_help.c ssu_help.h
	gcc -c ssu_help.c

ssu_find-md5.o : ssu_find-md5.c ssu_find-md5.h
	gcc -c ssu_find-md5.c

clean :
	rm *.o
	rm ssu_sdup