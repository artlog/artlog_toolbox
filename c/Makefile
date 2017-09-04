BUILD=../build

c_parser: c_parser.c c_tokenizer.c json_parser.c json_import_internal.c alstrings.c al_options.c
	gcc -o $@ $^ -L$(BUILD)/lib -Wl,-Bstatic -lalcommon -laldev -lalhash -Wl,-Bdynamic

clean:
	rm c_parser

.PHONY: clean
