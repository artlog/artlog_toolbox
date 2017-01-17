#vpath %.c ../src

CC=gcc
LD=gcc
CPPFLAGS=-g

BUILD=build

libsrc=c/json.c c/json_import_internal.c
src=c/main.c
libraries=json alsave altest allist aldev

objects=$(patsubst c/%.c,$(BUILD)/obj/%.o,$(src))
libobjects=$(patsubst c/%.c,$(BUILD)/obj/%.o,$(libsrc))


# default target is to build libraries
libs: $(patsubst %,$(BUILD)/lib/lib%.a,$(libraries))

all: libs test

$(BUILD)/lib/liballist.a: $(BUILD)/obj/allist.o $(BUILD)/obj/dump.o  $(BUILD)/include/allist.h
	ar rccs $@ $(BUILD)/obj/allist.o $(BUILD)/obj/dump.o

$(BUILD)/lib/libjson.a: $(BUILD)/obj/json.o  $(BUILD)/obj/json_import_internal.o $(BUILD)/include/json.h $(BUILD)/include/json_errors.h $(BUILD)/include/json_import_internal.h
	ar rccs $@ $(BUILD)/obj/json.o $(BUILD)/obj/json_import_internal.o

$(BUILD)/lib/libalsave.a:  $(BUILD)/obj/save.o  $(BUILD)/include/save.h
	ar rccs $@ $<

$(BUILD)/lib/libaltest.a:  $(BUILD)/obj/check_test.o $(BUILD)/include/check_test.h
	ar rccs $@ $<

$(BUILD)/lib/libaldev.a:  $(BUILD)/obj/todo.o $(BUILD)/include/todo.h
	ar rccs $@ $<

$(BUILD)/checksave: $(BUILD)/obj/save_main.o $(BUILD)/lib/libalsave.a
	$(LD) -o $@ $(LDFLAGS) $(BUILD)/obj/save_main.o -L$(BUILD)/lib -Wl,-Bstatic -lalsave -Wl,-Bdynamic

$(objects): | $(BUILD)/obj


$(libobjects): | $(BUILD)/lib


test:$(BUILD)/json
	mkdir -p $@
	./$(BUILD)/json test.json >test/parse1.json
	./$(BUILD)/json parse1.json >test/parse2.json
	./$(BUILD)/json refnawak.json >test/parse3.json
	./$(BUILD)/json test.json refnawak.json
	./$(BUILD)/json refnawak.json template.json -debug
	diff test/parse1.json test/parse2.json
	diff test/parse1.json test/parse3.json
	./$(BUILD)/json test2.json

$(BUILD)/json: $(BUILD)/lib/libjson.a $(objects)
	@echo link json objects $(objects) and libjson
	$(LD) -o $@ $(LDFLAGS) $(objects) -L$(BUILD)/lib -Wl,-Bstatic -ljson -Wl,-Bdynamic

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

.PHONY:clean test libs all

# needed to keep those files within include after make ( remove unused )
.PRECIOUS: $(BUILD)/include/%.h

