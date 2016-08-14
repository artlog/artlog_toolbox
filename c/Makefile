BUILD=../build

tests: test save
	./save_test_setup.sh
	./save ./savetest save test

test: test.c allist.c dump.c
	gcc -g $^ -o $@ -L$(BUILD)/lib -Wl,-Bstatic -laltest -Wl,-Bdynamic

save_main.o: save_main.c
	gcc -Wall -c $^ -o $@

save:  save_main.o
	gcc -o $@ $^ -L$(BUILD) -Wl,-Bstatic -lalsave -Wl,-Bdynamic

clean:
	rm save test
	rm *.o
	rm -rf savetest

.PHONY: tests clean
