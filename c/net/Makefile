BUILD=../../build

CC=gcc
LDLIBS= -lssl -lcrypto -lpthread
CFLAGS = -std=gnu99 -Wall -Werror -g3
export CPATH := .:..
BINARY=clientserver

$(BUILD)/$(BINARY): resolve.o clientserver_main.o loopbackserv.o alconn.o altls_openssl_wrapper.o dumper.o  alinput_sock.o
	$(CC) -g -o $@ $^ $(CFLAGS) $(LDLIBS) -L$(BUILD)/lib -Wl,-Bstatic -laljson -lalcommon -laldev -lalhash  -lalstack  -Wl,-Bdynamic

.PHONY:	clean test

clean:
	rm -rf $(BINARY)
	rm -rf *.o


test: clean connect
	@echo "*** Please 'nc localhost 4096' to complete ***"  
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:mylib ./$(BINARIES) free.fr
