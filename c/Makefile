BUILD=../build

c_parser: c_parser.c c_tokenizer.c aljson_parser.c aljson_import_internal.c alstrings.c al_options.c
	gcc -o $@ $^ -L$(BUILD)/lib -Wl,-Bstatic -lalcommon -laldev -lalhash -Wl,-Bdynamic

albase: albase.c albase_test.c  alstrings.c aldebug.c
	gcc -o $@ $^

json_to_c_stub_sample: json_to_c_stub.c json_to_c_stub_sample.c albase.c
	gcc -g -o $@ $^ -L$(BUILD)/lib -Wl,-Bstatic -laljson -lalcommon -laldev -lalhash  -lalstack -Wl,-Bdynamic

cryptohash: alcryptohash.c alcryptohash_test.c alstrings.c aldebug.c altodo.c alinput.c
	gcc -g -O0 -o $@ $^

tests: c_parser albase json_to_c_stub_sample cryptohash
	./albase
	./json_to_c_stub_sample
	./c_parser infile=./input_for_c_parser.h outform=aljson_stub
	./cryptohash


clean:
	rm c_parser albase json_to_c_stub_sample cryptohash

.PHONY: clean tests
