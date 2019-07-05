BUILD=../build

BINARIES=c_parser albase json_to_c_stub_sample cryptohash bitfield aljson_ld alpadfile hashtree salsa20 alimp
BUILT_BINARIES=$(addprefix $(BUILD)/,$(BINARIES))

all: $(BUILT_BINARIES)

$(BUILD)/c_parser: c_parser.c c_tokenizer.c aljson_parser.c aljson_import_internal.c alstrings.c al_options.c aljson.c alstack.c altoken.c aljson_dump.c aljson_output.c
	gcc -o $@ $^ -L$(BUILD)/lib -Wl,-Bstatic -lalcommon -laldev -lalhash -Wl,-Bdynamic -lm

$(BUILD)/albase: albase.c albase_test.c  alstrings.c aldebug.c aloutput.c
	gcc -o $@ $^

$(BUILD)/json_to_c_stub_sample: aljson_encoder.c json_to_c_stub.c json_to_c_stub_sample.c albase.c
	gcc -g -o $@ $^ -L$(BUILD)/lib -Wl,-Bstatic -laljson -lalcommon -laldev -lalhash  -lalstack -Wl,-Bdynamic -lm

$(BUILD)/cryptohash: alcryptohash_test.c alcryptohash.c alstrings.c aldebug.c altodo.c alinput.c albase64.c albitfieldreader.c alcryptohash_tool.c aloutput.c
	gcc -g -O0 -o $@ $^

$(BUILD)/salsa20: alsalsa20.c alsalsa20_test.c alstrings.c aldebug.c altodo.c  aloutput.c
	gcc -g -O0 -o $@ $^

$(BUILD)/bitfield: albitfield_test.c al_options.c
	gcc -g -o $@ $^ -L$(BUILD)/lib -Wl,-Bstatic -laljson -lalcommon -laldev -lalhash  -lalstack -Wl,-Bdynamic

$(BUILD)/alpadfile: alpadfile.c
	gcc -g -o $@ $^ -L$(BUILD)/lib -Wl,-Bstatic -laljson -lalcommon -laldev -lalhash  -lalstack -Wl,-Bdynamic

$(BUILD)/hashtree: alhashtree.c alhashtree_test.c alcryptohash.c
	gcc -g -o $@ $^ -L$(BUILD)/lib -Wl,-Bstatic -lalcommon -laldev -lalhash -Wl,-Bdynamic


$(BUILD)/aljson_ld: aljson_ld.c aljson_ld_test.c
	gcc -g -o $@ $^ -L$(BUILD)/lib -Wl,-Bstatic -laljson -laldev -lalhash  -lalstack -lalcommon -Wl,-Bdynamic

$(BUILD)/alimp: 	alimp.c
	gcc -g -o $@ $^ -L$(BUILD)/lib -Wl,-Bstatic -laljson -lalcommon -laldev -lalhash  -lalstack -Wl,-Bdynamic

clean:
	rm $(BUILT_BINARIES)

.PHONY: clean all
