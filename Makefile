#vpath %.c ../src

CC=gcc
LD=gcc
CPPFLAGS=-g

BUILD=build
TMPTESTDIR=$(BUILD)/tmp
TEMPLATE=template
TEMPLATEHASH=$(TEMPLATE)/alhashsample
INCLUDEDIR=$(BUILD)/include

libsrc=c/aljson_parser.c c/aljson.c c/aljson_import_internal.c c/aljson_dump.c
src=c/aljson_main.c
libraries=aljson alsave altest allist aldev alhash alcommon alstack

staticlibraries=$(patsubst %,$(BUILD)/lib/lib%.a,$(libraries))

objects=$(patsubst c/%.c,$(BUILD)/obj/%.o,$(src))
libobjects=$(patsubst c/%.c,$(BUILD)/obj/%.o,$(libsrc))

LIBINCLUDES=aljson.h aljson_errors.h aljson_import_internal.h aljson_parser.h\
 alstrings.h json_to_c_stub.h albitfieldreader.h albitfieldwriter.h albase.h\
 al_options.h al_options_output.h altoken.h aljson_print.h alpathfile.h\
 aldebug.h aldebug_output.h alinput_file.h aloutput_file.h

LIBINCLUDESABS=$(addprefix $(INCLUDEDIR)/,$(LIBINCLUDES))

COMMONOBJS=alstrings.o aloutput.o alinput.o alcommon.o aldebug.o  albtree.o\
 albitfieldreader.o albitfieldwriter.o albase.o alpathfile.o alinput_file.o\
 aloutput_file.o

COMMONOBJSABS=$(addprefix $(BUILD)/obj/,$(COMMONOBJS))

libaljsonsources=c/aljson_parser.c c/aljson.c c/aljson_import_internal.c\
 c/alstrings.c c/json_to_c_stub.c c/al_options.c c/al_options_output.c\
 c/aljson_dump.c c/aljson_unify.c c/aljson_walk.c c/altoken.c\
 c/aljson_encoder.c c/aljson_output.c

libaljsonobjects=$(patsubst c/%.c,$(BUILD)/obj/%.o,$(libaljsonsources))

# default target is to build libraries
libs: $(staticlibraries)

all: libinclude libs tests $(BUILD)/base64 $(BUILD)/aljson_ld $(BUILD)/cbor_main $(BUILD)/alabnf $(BUILD)/alabnf_matcher $(BUILD)/c_parser

libinclude: $(LIBINCLUDESABS)

$(BUILD)/lib/liballist.a: $(BUILD)/obj/allist.o $(BUILD)/obj/aldump.o\
 $(INCLUDEDIR)/allist.h
	ar rccs $@ $(BUILD)/obj/allist.o $(BUILD)/obj/aldump.o

$(BUILD)/lib/libaljson.a: $(libaljsonobjects)
	ar rccs $@ $^

$(BUILD)/lib/libalsave.a:  $(BUILD)/obj/alsave.o  $(INCLUDEDIR)/alsave.h
	ar rccs $@ $<

$(BUILD)/lib/libaltest.a:  $(BUILD)/private/obj/tests/check_test.o c/private/check_test.h
	ar rccs $@ $<

$(BUILD)/lib/libaldev.a:  $(BUILD)/obj/altodo.o $(INCLUDEDIR)/altodo.h
	ar rccs $@ $<

$(BUILD)/lib/libalcommon.a: $(COMMONOBJSABS)  $(INCLUDEDIR)/alinput.h\
 $(INCLUDEDIR)/aloutput.h $(INCLUDEDIR)/alcommon.h $(INCLUDEDIR)/aldebug.h\
 $(INCLUDEDIR)/albase.h $(INCLUDEDIR)/alpathfile.h
	ar rccs $@  $(COMMONOBJSABS)

$(BUILD)/lib/libalhash.a:  $(BUILD)/obj/alhash.o $(BUILD)/obj/alhash_output.o\
 $(INCLUDEDIR)/alhash.h
	ar rccs $@  $(BUILD)/obj/alhash.o $(BUILD)/obj/alhash_output.o

$(BUILD)/lib/libalstack.a:  $(BUILD)/obj/alstack.o $(INCLUDEDIR)/alstack.h
	ar rccs $@ $<

$(BUILD)/checksave: $(BUILD)/obj/save_main.o $(BUILD)/lib/libalsave.a
	$(LD) -o $@ $(LDFLAGS) $(BUILD)/obj/save_main.o -L$(BUILD)/lib\
 -Wl,-Bstatic -lalsave -Wl,-Bdynamic

$(BUILD)/test_auto_c_gen:  $(BUILD)/obj/json_to_c_stub.o
	@echo link json objects $^ and libjson
	$(LD) -o $@ $(LDFLAGS) $^ -L$(BUILD)/lib -Wl,-Bstatic -laljson\
 -Wl,-Bdynamic

$(BUILD)/test_alstack:  $(BUILD)/private/obj/tests/test_alstack.o
	@echo link test objects $^ and libalstack
	$(LD) -o $@ $(LDFLAGS) $^ -L$(BUILD)/lib \
 -Wl,-Bstatic -lalstack -lalhash -lalcommon -Wl,-Bdynamic

$(BUILD)/testbtree: $(BUILD)/obj/albtree.o $(BUILD)/obj/albtreetest.o
	$(LD) -o $@ $(LDFLAGS) $^ -L$(BUILD)/lib\
 -Wl,-Bstatic -lalstack -lalhash -lalcommon -Wl,-Bdynamic

$(BUILD)/obj $(BUILD)/private/obj $(BUILD)/lib $(INCLUDEDIR) $(TMPTESTDIR):
	@echo 'create $@ directory'
	mkdir -p $@

$(objects): | $(BUILD)/obj

$(libobjects): | $(BUILD)/lib

tests: testjson testhash $(BUILD)/test_alstack testbtree testallist

testbtree: $(BUILD)/testbtree | $(TMPTESTDIR)
	$< ceci est un test depuis le makefile\
 2>$(TMPTESTDIR)/$@.out.2 >$(TMPTESTDIR)/$@.out

testhash: $(BUILD)/hash | $(TMPTESTDIR)
	$< $(TEMPLATEHASH)/sample2.txt \
 2>$(TMPTESTDIR)/$@.sample2.out.2 >$(TMPTESTDIR)/sample2.out
	$< c/c_parser.c \
 2>$(TMPTESTDIR)/$@.c_parser.out.2 >$(TMPTESTDIR)/c_parser.out
	@$< $(TEMPLATEHASH)/words.txt\
 2>$(TMPTESTDIR)/$@.words.out.2 >$(TMPTESTDIR)/words.out
	@diff $(TEMPLATEHASH)/words.out $(TMPTESTDIR)/words.out\
 && echo "hash words [OK]"

$(BUILD)/hash:  $(BUILD)/obj/alhash_test.o
	$(LD) -o $@ $(LDFLAGS) $^ -L$(BUILD)/lib \
 -Wl,-Bstatic -lalhash -lalcommon -Wl,-Bdynamic

testjson: $(BUILD)/json | $(TMPTESTDIR)
	$< -- $(TEMPLATE)/test.json >$(TMPTESTDIR)/parse1.json\
 2>$(TMPTESTDIR)/$@.parse1.json.out.2
	$< -- $(TEMPLATE)/parse1.json >$(TMPTESTDIR)/parse2.json\
 2>$(TMPTESTDIR)/$@.parse2.json.out.2
	$< -- $(TEMPLATE)/refnawak.json >$(TMPTESTDIR)/parse3.json\
 2>$(TMPTESTDIR)/$@.parse3.json.out.2
	$< template=$(TEMPLATE)/refnawak.json -- $(TEMPLATE)/test.json \
 >$(TMPTESTDIR)/$@.test.refnawak.json.out 2>$(TMPTESTDIR)/$@.refnawak.json.out.2
	$< template=$(TEMPLATE)/template.json -- $(TEMPLATE)/refnawak.json \
 -debug 2>$(TMPTESTDIR)/$@.parse1.json.out.12 1>&2
	@diff $(TMPTESTDIR)/parse1.json $(TMPTESTDIR)/parse2.json\
 && echo "parse2.json [OK]"
	@diff $(TMPTESTDIR)/parse1.json $(TMPTESTDIR)/parse3.json\
 && echo "parse3.json [OK]"
	$< -- $(TEMPLATE)/test2.json  >$(TMPTESTDIR)/test2.json\
 2>$(TMPTESTDIR)/$@.test2.json.out.2

testallist: $(BUILD)/testallist | $(TMPTESTDIR)
	$< 10x 10x 10x -decomp -trace >$(TMPTESTDIR)/$@.1000.decomp.out 2>$(TMPTESTDIR)/$@.1000.decomp.trace

$(BUILD)/testallist: $(BUILD)/private/obj/tests/allist_test.o
	gcc -g $^ -o $@ -I$(INCLUDEDIR) -L$(BUILD)/lib \
 -Wl,-Bstatic -lallist -lalcommon -laltest -Wl,-Bdynamic

$(BUILD)/json: $(objects) $(BUILD)/lib/libaljson.a
	@echo link json objects $(objects) and libjson
	$(LD) -o $@ $(LDFLAGS) $(objects) -L$(BUILD)/lib \
 -Wl,-Bstatic -laljson  -lalstack -lalhash -lalcommon -laldev  -Wl,-Bdynamic -lm

$(INCLUDEDIR)/%.h: c/%.h | $(INCLUDEDIR)
	cp $< $@

$(BUILD)/obj/%.o: c/%.c | $(BUILD)/obj
	@echo compile $<
	$(CC) -Wall -Wcast-align=strict -c $(CFLAGS) $(CPPFLAGS) -I$(INCLUDEDIR) $< -o $@

$(BUILD)/private/obj/%.o: c/%.c | $(BUILD)/private/obj
	@echo "bad hack fixme" && mkdir -p $(BUILD)/private/obj/tests
	@echo compile private $<
	@$(CC) -Wall -c $(CFLAGS) $(CPPFLAGS) -I$(INCLUDEDIR)\
 -I c/private $< -o $@

$(BUILD)/base64: $(staticlibraries) | libinclude
	cd c; make ../$(BUILD)/base64

$(BUILD)/cbor_main: $(staticlibraries) | libinclude
	cd c/cbor; make ../../$(BUILD)/cbor_main

$(BUILD)/alabnf $(BUILD)/alabnf_matcher: $(staticlibraries) | libinclude
	cd c/abnf; make ../../$@

$(BUILD)/aljson_ld: $(staticlibraries) | libinclude
	cd c/; make ../$@

$(BUILD)/c_parser: $(staticlibraries) | libinclude
	cd c/; make ../$@

clean:
	rm -rf $(BUILD)

dev:
	cd c/; etags *.[ch]

.PHONY:clean test libs all tests testjson testhash libinclude dev

# needed to keep those files within include after make ( remove unused )
.PRECIOUS: $(INCLUDEDIR)/%.h

