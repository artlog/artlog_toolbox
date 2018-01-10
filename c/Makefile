BUILD=../build

c_parser: c_parser.c c_tokenizer.c aljson_parser.c aljson_import_internal.c alstrings.c al_options.c
	gcc -o $@ $^ -L$(BUILD)/lib -Wl,-Bstatic -lalcommon -laldev -lalhash -Wl,-Bdynamic

albase: albase.c albase_test.c 
	gcc -o $@ $^

json_to_c_stub: json_to_c_stub.c albase.c
	gcc -o $@ $^ -L$(BUILD)/lib -Wl,-Bstatic -laljson -lalcommon -laldev -lalhash  -lalstack -Wl,-Bdynamic

tests: c_parser albase json_to_c_stub
	./c_parser infile=./input_for_c_parser.c outform=aljson_stub
	./albase
	./json_to_c_stub


clean:
	rm c_parser albase json_to_c_stub

.PHONY: clean tests
