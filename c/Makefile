
tests: test save
	./save_test_setup.sh
	./save ./savetest save test

test: test.c allist.c dump.c
	gcc -g $^ -o $@

libalsave.a: save.c
	gcc -Wall -c $^ -o save.o
	ar rccs $@ save.o

save_main.o: save_main.c
	gcc -Wall -c $^ -o $@

save:  save_main.o libalsave.a
	gcc save_main.o -L . -static -lalsave -o save


.PHONY: tests
