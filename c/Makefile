BUILD=../build

c_parser: c_parser.c c_tokenizer.c aljson_parser.c aljson_import_internal.c alstrings.c al_options.c aljson.c alstack.c
	gcc -o $@ $^ -L$(BUILD)/lib -Wl,-Bstatic -lalcommon -laldev -lalhash -Wl,-Bdynamic

albase: albase.c albase_test.c  alstrings.c aldebug.c
	gcc -o $@ $^

json_to_c_stub_sample: json_to_c_stub.c json_to_c_stub_sample.c albase.c
	gcc -g -o $@ $^ -L$(BUILD)/lib -Wl,-Bstatic -laljson -lalcommon -laldev -lalhash  -lalstack -Wl,-Bdynamic

cryptohash: alcryptohash_test.c alcryptohash.c alstrings.c aldebug.c altodo.c alinput.c albase64.c albitfieldreader.c alcryptohash_tool.c aloutput.c
	gcc -g -O0 -o $@ $^

salsa20: alsalsa20.c alsalsa20_test.c alstrings.c aldebug.c altodo.c 
	gcc -g -O0 -o $@ $^

bitfield: albitfield_test.c al_options.c
	gcc -g -o $@ $^ -L$(BUILD)/lib -Wl,-Bstatic -laljson -lalcommon -laldev -lalhash  -lalstack -Wl,-Bdynamic

alpadfile: alpadfile.c al_options.c
	gcc -g -o $@ $^ -L$(BUILD)/lib -Wl,-Bstatic -laljson -lalcommon -laldev -lalhash  -lalstack -Wl,-Bdynamic

hashtree: alhashtree.c alhashtree_test.c alcryptohash.c
	gcc -g -o $@ $^ -L$(BUILD)/lib -Wl,-Bstatic -lalcommon -laldev -lalhash -Wl,-Bdynamic

tests: c_parser albase json_to_c_stub_sample cryptohash bitfield
	./albase
	./json_to_c_stub_sample
	./c_parser infile=./input_for_c_parser.h outform=aljson_stub
	./cryptohash '' d
	./cryptohash ./cryptohash
	./bitfield outfile=bitfieldcopy
	./hashtree 'ceci est la premiere pharse' 'la seconde phrase' 'la troisieme phrase' '4' '5' '6'
	./salsa20

aljson_ld: aljson_ld.c aljson_ld_test.c
	gcc -g -o $@ $^ -L$(BUILD)/lib -Wl,-Bstatic -laljson -lalcommon -laldev -lalhash  -lalstack -Wl,-Bdynamic

clean:
	rm c_parser albase json_to_c_stub_sample cryptohash bitfield

.PHONY: clean tests
