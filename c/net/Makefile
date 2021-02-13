CC=gcc
LDLIBS= -lssl -lcrypto -lpthread
CFLAGS = -std=gnu99 -Wall -Werror -g3
export CPATH := .:..

connect: resolve.o connect.o loopbackserv.o alurl.o al_http.o alconn.o altls_openssl_wrapper.o dumper.o ../aldebug.o

.PHONY:	clean test

clean:
	rm -rf connect
	rm -rf *.o


test: clean connect
	@echo "*** Please 'nc localhost 4096' to complete ***"  
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:mylib ./connect free.fr
