BUILD=../build

tests: test save
	./save_test_setup.sh
	./save ./savetest save test

test: test.c allist.c dump.c
	gcc -g $^ -o $@ -L$(BUILD)/lib -Wl,-Bstatic -laltest -Wl,-Bdynamic

save_main.o: save_main.c
	gcc -Wall -c $^ -o $@

save:  save_main.o
	gcc -o $@ $^ -L$(BUILD)/lib -Wl,-Bstatic -lalsave -Wl,-Bdynamic

clean:
	rm save test
	rm *.o
	rm -rf savetest

c_parser: c_parser.c c_tokenizer.c json_parser.c json_import_internal.c
	gcc -o $@ $^ -L$(BUILD)/lib -Wl,-Bstatic -lalcommon -laldev -Wl,-Bdynamic

.PHONY: tests clean
