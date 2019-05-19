BUILD=../../build

tests: save_test
	./save_test_setup.sh
	./save_test ./savetest save test

# allist_test move to main Makefile in git project root

save_main.o: save_main.c
	gcc -Wall -c $^ -o $@ -I$(BUILD)/include -I../private

save_test:  save_main.o
	gcc -o $@ $^ -I$(BUILD)/include -L$(BUILD)/lib -Wl,-Bstatic -lalsave -lalcommon -Wl,-Bdynamic

clean:
	rm save_test
	rm *.o
	rm -rf savetest

.PHONY: tests clean
