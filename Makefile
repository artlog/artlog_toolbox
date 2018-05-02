#vpath %.c ../src

CC=gcc
LD=gcc
CPPFLAGS=-g

BUILD=build

libsrc=c/aljson_parser.c c/aljson.c c/aljson_import_internal.c
src=c/aljson_main.c
libraries=aljson alsave altest allist aldev alhash alcommon alstack

objects=$(patsubst c/%.c,$(BUILD)/obj/%.o,$(src))
libobjects=$(patsubst c/%.c,$(BUILD)/obj/%.o,$(libsrc))


# default target is to build libraries
libs: $(patsubst %,$(BUILD)/lib/lib%.a,$(libraries))

all: libs tests libinclude

libinclude: $(BUILD)/include/aljson.h $(BUILD)/include/aljson_errors.h $(BUILD)/include/aljson_import_internal.h $(BUILD)/include/aljson_parser.h $(BUILD)/include/alstrings.h $(BUILD)/include/json_to_c_stub.h $(BUILD)/include/albitfieldreader.h $(BUILD)/include/albitfieldwriter.h $(BUILD)/include/albase.h

$(BUILD)/lib/liballist.a: $(BUILD)/obj/allist.o $(BUILD)/obj/dump.o  $(BUILD)/include/allist.h
	ar rccs $@ $(BUILD)/obj/allist.o $(BUILD)/obj/dump.o

$(BUILD)/lib/libaljson.a: $(BUILD)/obj/aljson_parser.o $(BUILD)/obj/aljson.o $(BUILD)/obj/aljson_import_internal.o $(BUILD)/obj/alstrings.o $(BUILD)/obj/json_to_c_stub.o
	ar rccs $@ $^

$(BUILD)/lib/libalsave.a:  $(BUILD)/obj/save.o  $(BUILD)/include/save.h
	ar rccs $@ $<
$(BUILD)/lib/libaltest.a:  $(BUILD)/obj/check_test.o $(BUILD)/include/check_test.h
	ar rccs $@ $<

$(BUILD)/lib/libaldev.a:  $(BUILD)/obj/altodo.o $(BUILD)/include/altodo.h
	ar rccs $@ $<

$(BUILD)/lib/libalcommon.a: $(BUILD)/obj/aloutput.o $(BUILD)/obj/alinput.o $(BUILD)/obj/alcommon.o $(BUILD)/obj/aldebug.o $(BUILD)/obj/albtree.o $(BUILD)/obj/albitfieldreader.o $(BUILD)/obj/albitfieldwriter.o $(BUILD)/obj/albase.o $(BUILD)/include/alinput.h $(BUILD)/include/aloutput.h $(BUILD)/include/alcommon.h $(BUILD)/include/aldebug.h $(BUILD)/include/albase.h
	ar rccs $@  $(BUILD)/obj/aloutput.o $(BUILD)/obj/alinput.o $(BUILD)/obj/alcommon.o $(BUILD)/obj/albtree.o $(BUILD)/obj/aldebug.o  $(BUILD)/obj/albitfieldreader.o $(BUILD)/obj/albitfieldwriter.o  $(BUILD)/obj/albase.o

$(BUILD)/lib/libalhash.a:  $(BUILD)/obj/alhash.o $(BUILD)/obj/alstrings.o $(BUILD)/include/alhash.h
	ar rccs $@  $(BUILD)/obj/alhash.o $(BUILD)/obj/alstrings.o

$(BUILD)/lib/libalstack.a:  $(BUILD)/obj/alstack.o $(BUILD)/include/alstack.h
	ar rccs $@ $<

$(BUILD)/checksave: $(BUILD)/obj/save_main.o $(BUILD)/lib/libalsave.a
	$(LD) -o $@ $(LDFLAGS) $(BUILD)/obj/save_main.o -L$(BUILD)/lib -Wl,-Bstatic -lalsave -Wl,-Bdynamic

$(BUILD)/test_auto_c_gen:  $(BUILD)/obj/json_to_c_stub.o
	@echo link json objects $^ and libjson
	$(LD) -o $@ $(LDFLAGS) $^ -L$(BUILD)/lib -Wl,-Bstatic -laljson -Wl,-Bdynamic

$(BUILD)/test_alstack:  $(BUILD)/obj/test_alstack.o
	@echo link test objects $^ and libalstack
	$(LD) -o $@ $(LDFLAGS) $^ -L$(BUILD)/lib -Wl,-Bstatic -lalstack -lalcommon -Wl,-Bdynamic

$(BUILD)/testbtree: $(BUILD)/obj/albtree.o $(BUILD)/obj/albtreetest.o
	$(LD) -o $@ $(LDFLAGS) $^


testbtree: $(BUILD)/testbtree
	$(BUILD)/testbtree ceci est un test depuis le makefile


$(objects): | $(BUILD)/obj


$(libobjects): | $(BUILD)/lib


tests: testjson testhash $(BUILD)/test_alstack testbtree


testhash: $(BUILD)/hash
	$^ alhashsample/sample2.txt
	$^ c/c_parser.c

$(BUILD)/hash:  $(BUILD)/obj/alhash_test.o
	$(LD) -o $@ $(LDFLAGS) $^ -L$(BUILD)/lib -Wl,-Bstatic -lalhash -lalcommon -Wl,-Bdynamic

testjson:$(BUILD)/json
	$^ template/test.json >test/parse1.json
	$^ template/parse1.json >test/parse2.json
	$^ template/refnawak.json >test/parse3.json
	$^ template/test.json template/refnawak.json
	$^ template/refnawak.json template/template.json -debug
	diff test/parse1.json test/parse2.json
	diff test/parse1.json test/parse3.json
	$^ template/test2.json

$(BUILD)/json: $(objects)
	@echo link json objects $(objects) and libjson
	$(LD) -o $@ $(LDFLAGS) $^ -L$(BUILD)/lib -Wl,-Bstatic -laljson  -lalstack -lalhash -lalcommon  -Wl,-Bdynamic

$(BUILD)/obj:
	mkdir -p $@

$(BUILD)/lib:
	mkdir -p $@

$(BUILD)/include:
	mkdir -p $@

$(BUILD)/include/%.h: c/%.h $(BUILD)/include
	cp $< $@

$(BUILD)/obj/%.o: c/%.c $(BUILD)/obj
	@echo compile $< 
	@$(CC) -Wall -c $(CFLAGS) $(CPPFLAGS) $< -o $@

clean:
	rm -rf $(BUILD)

.PHONY:clean test libs all tests testjson testhash libinclude

# needed to keep those files within include after make ( remove unused )
.PRECIOUS: $(BUILD)/include/%.h

