
tests: test save
	./save_test_setup.sh
	./save ./savetest save test

test: test.c allist.c dump.c libaltest.a
	gcc -g $^ -o $@ -L. -Wl,-Bstatic -laltest -Wl,-Bdynamic

libalsave.a: save.c
	gcc -Wall -c $^ -o save.o
	ar rccs $@ save.o

libaltest.a: check_test.c
	gcc -Wall -c $^ -o check_test.o
	ar rccs $@ check_test.o

save_main.o: save_main.c
	gcc -Wall -c $^ -o $@

save:  save_main.o libalsave.a
	gcc -o save save_main.o -L. -Wl,-Bstatic -lalsave -Wl,-Bdynamic


clean:
	rm save test
	rm *.o *.a
	rm -rf savetest

.PHONY: tests clean
