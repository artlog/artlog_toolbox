BUILD=../../build

tests: allist_test save_test
	./save_test_setup.sh
	./save_test ./savetest save test

allist_test: allist_test.c
	gcc -g $^ -o $@ -I$(BUILD)/include -L$(BUILD)/lib -Wl,-Bstatic -lallist -laltest -Wl,-Bdynamic

save_main.o: save_main.c
	gcc -Wall -c $^ -o $@ -I$(BUILD)/include

save_test:  save_main.o
	gcc -o $@ $^ -I$(BUILD)/include -L$(BUILD)/lib -Wl,-Bstatic -lalsave -Wl,-Bdynamic

clean:
	rm save_test allist_test
	rm *.o
	rm -rf savetest

.PHONY: tests clean
