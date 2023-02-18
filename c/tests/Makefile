BUILD=../../build
INCLUDES=$(BUILD)/include

all:	tests alinput_test aloutput_test aldebug test0
	./alinput_test <alinput_test.c
	./aloutput_test
	./aldebug

tests: save_test
	./save_test_setup.sh
	./save_test ./savetest save test

# allist_test move to main Makefile in git project root

save_main.o: save_main.c
	gcc -Wall -c $^ -o $@ -I$(BUILD)/include -I../private

save_test:  save_main.o
	gcc -o $@ $^ -I$(BUILD)/include -L$(BUILD)/lib -Wl,-Bstatic -lalsave -lalcommon -Wl,-Bdynamic

alinput_test: alinput_test.c ../alinput_util.c
	gcc -Wall -g -o $@ $^ -I.. -L$(BUILD)/lib -Wl,-Bstatic -laldev -lalcommon -lalhash  -Wl,-Bdynamic 

aloutput_test: aloutput_test.c 
	gcc -Wall -g -o $@ $^ -I.. -L$(BUILD)/lib -Wl,-Bstatic -laldev -lalcommon -lalhash  -Wl,-Bdynamic 

aldebug: aldebug_test.c
	gcc -Wall -g -o $@ $^ -I$(INCLUDES) -I../private -L$(BUILD)/lib -Wl,-Bstatic -laldev -lalcommon -lalhash  -Wl,-Bdynamic

test0:
	$(BUILD)/albase
	$(BUILD)/json_to_c_stub_sample
	$(BUILD)/c_parser infile=../input_for_c_parser.h outform=aljson_stub
	$(BUILD)/cryptohash '' d
	$(BUILD)/cryptohash $(BUILD)/cryptohash
	$(BUILD)/bitfield outfile=bitfieldcopy
	$(BUILD)/hashtree 'ceci est la premiere pharse' 'la seconde phrase' 'la troisieme phrase' '4' '5' '6'
	$(BUILD)/salsa20



clean:
	rm save_test
	rm *.o
	rm -rf savetest
	rm -f alinput_test
	rm -f bitfieldcopy
	rm -f aloutput_test

.PHONY: test0 tests clean all
