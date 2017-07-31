#vpath %.c ../src

CC=gcc
LD=gcc
CPPFLAGS=-g

BUILD=build

libsrc=c/json_parser.c c/json.c c/json_import_internal.c
src=c/main.c
libraries=json alsave altest allist aldev alhash alcommon

objects=$(patsubst c/%.c,$(BUILD)/obj/%.o,$(src))
libobjects=$(patsubst c/%.c,$(BUILD)/obj/%.o,$(libsrc))


# default target is to build libraries
libs: $(patsubst %,$(BUILD)/lib/lib%.a,$(libraries))


all: libs tests libinclude

libinclude: $(BUILD)/include/json.h $(BUILD)/include/json_errors.h $(BUILD)/include/json_import_internal.h $(BUILD)/include/json_parser.h


$(BUILD)/lib/liballist.a: $(BUILD)/obj/allist.o $(BUILD)/obj/dump.o  $(BUILD)/include/allist.h
	ar rccs $@ $(BUILD)/obj/allist.o $(BUILD)/obj/dump.o

$(BUILD)/lib/libjson.a: $(BUILD)/obj/json_parser.o $(BUILD)/obj/json.o  $(BUILD)/obj/json_import_internal.o
	ar rccs $@ $(BUILD)/obj/json_parser.o $(BUILD)/obj/json.o $(BUILD)/obj/json_import_internal.o

$(BUILD)/lib/libalsave.a:  $(BUILD)/obj/save.o  $(BUILD)/include/save.h
	ar rccs $@ $<
$(BUILD)/lib/libaltest.a:  $(BUILD)/obj/check_test.o $(BUILD)/include/check_test.h
	ar rccs $@ $<

$(BUILD)/lib/libaldev.a:  $(BUILD)/obj/todo.o $(BUILD)/include/todo.h
	ar rccs $@ $<

$(BUILD)/lib/libalcommon.a: $(BUILD)/obj/outputstream.o $(BUILD)/obj/inputstream.o $(BUILD)/obj/alcommon.o $(BUILD)/include/inputstream.h $(BUILD)/include/outputstream.h $(BUILD)/include/alcommon.h
	ar rccs $@ $(BUILD)/obj/outputstream.o $(BUILD)/obj/inputstream.o $(BUILD)/obj/alcommon.o

$(BUILD)/lib/libalhash.a:  $(BUILD)/obj/alhash.o $(BUILD)/include/alhash.h
	ar rccs $@ $<

$(BUILD)/checksave: $(BUILD)/obj/save_main.o $(BUILD)/lib/libalsave.a
	$(LD) -o $@ $(LDFLAGS) $(BUILD)/obj/save_main.o -L$(BUILD)/lib -Wl,-Bstatic -lalsave -Wl,-Bdynamic

$(BUILD)/test_auto_c_gen:  $(BUILD)/obj/json_to_c_stub.o
	@echo link json objects $^ and libjson
	$(LD) -o $@ $(LDFLAGS) $^ -L$(BUILD)/lib -Wl,-Bstatic -ljson -Wl,-Bdynamic

$(objects): | $(BUILD)/obj


$(libobjects): | $(BUILD)/lib


tests: testjson testhash


testhash: $(BUILD)/hash
	$^ alhashsample/sample2.txt

$(BUILD)/hash:  $(BUILD)/obj/alhash_test.o
	$(LD) -o $@ $(LDFLAGS) $^ -L$(BUILD)/lib -Wl,-Bstatic -lalhash -Wl,-Bdynamic

testjson:$(BUILD)/json
	$^ test.json >test/parse1.json
	$^ parse1.json >test/parse2.json
	$^ refnawak.json >test/parse3.json
	$^ test.json refnawak.json
	$^ refnawak.json template.json -debug
	diff test/parse1.json test/parse2.json
	diff test/parse1.json test/parse3.json
	$^ test2.json

$(BUILD)/json: $(objects)
	@echo link json objects $(objects) and libjson
	$(LD) -o $@ $(LDFLAGS) $^ -L$(BUILD)/lib -Wl,-Bstatic -ljson -lalcommon -Wl,-Bdynamic

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

