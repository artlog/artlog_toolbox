#vpath %.c ../src

CC=gcc
LD=gcc
CPPFLAGS=-g

BUILD=build
TMPTESTDIR=$(BUILD)/tmp
TEMPLATE=template
TEMPLATEHASH=$(TEMPLATE)/alhashsample

libsrc=c/aljson_parser.c c/aljson.c c/aljson_import_internal.c c/aljson_dump.c
src=c/aljson_main.c
libraries=aljson alsave altest allist aldev alhash alcommon alstack

objects=$(patsubst c/%.c,$(BUILD)/obj/%.o,$(src))
libobjects=$(patsubst c/%.c,$(BUILD)/obj/%.o,$(libsrc))


# default target is to build libraries
libs: $(patsubst %,$(BUILD)/lib/lib%.a,$(libraries))

all: libs tests libinclude

libinclude: $(BUILD)/include/aljson.h $(BUILD)/include/aljson_errors.h $(BUILD)/include/aljson_import_internal.h $(BUILD)/include/aljson_parser.h $(BUILD)/include/alstrings.h $(BUILD)/include/json_to_c_stub.h $(BUILD)/include/albitfieldreader.h $(BUILD)/include/albitfieldwriter.h $(BUILD)/include/albase.h $(BUILD)/include/al_options.h $(BUILD)/include/altoken.h $(BUILD)/include/aljson_print.h $(BUILD)/include/alpathfile.h

$(BUILD)/lib/liballist.a: $(BUILD)/obj/allist.o $(BUILD)/obj/dump.o  $(BUILD)/include/allist.h
	ar rccs $@ $(BUILD)/obj/allist.o $(BUILD)/obj/dump.o

$(BUILD)/lib/libaljson.a: $(BUILD)/obj/aljson_parser.o $(BUILD)/obj/aljson.o $(BUILD)/obj/aljson_import_internal.o $(BUILD)/obj/alstrings.o $(BUILD)/obj/json_to_c_stub.o $(BUILD)/obj/al_options.o $(BUILD)/obj/aljson_dump.o $(BUILD)/obj/aljson_unify.o $(BUILD)/obj/aljson_walk.o $(BUILD)/obj/altoken.o
	ar rccs $@ $^

$(BUILD)/lib/libalsave.a:  $(BUILD)/obj/save.o  $(BUILD)/include/save.h
	ar rccs $@ $<

$(BUILD)/lib/libaltest.a:  $(BUILD)/obj/check_test.o $(BUILD)/include/check_test.h
	ar rccs $@ $<

$(BUILD)/lib/libaldev.a:  $(BUILD)/obj/altodo.o $(BUILD)/include/altodo.h
	ar rccs $@ $<

$(BUILD)/lib/libalcommon.a: $(BUILD)/obj/aloutput.o $(BUILD)/obj/alinput.o $(BUILD)/obj/alcommon.o $(BUILD)/obj/aldebug.o $(BUILD)/obj/albtree.o $(BUILD)/obj/albitfieldreader.o $(BUILD)/obj/albitfieldwriter.o $(BUILD)/obj/albase.o $(BUILD)/obj/alpathfile.o $(BUILD)/include/alinput.h $(BUILD)/include/aloutput.h $(BUILD)/include/alcommon.h $(BUILD)/include/aldebug.h $(BUILD)/include/albase.h $(BUILD)/include/alpathfile.h
	ar rccs $@  $(BUILD)/obj/aloutput.o $(BUILD)/obj/alinput.o $(BUILD)/obj/alcommon.o $(BUILD)/obj/albtree.o $(BUILD)/obj/aldebug.o  $(BUILD)/obj/albitfieldreader.o $(BUILD)/obj/albitfieldwriter.o  $(BUILD)/obj/albase.o $(BUILD)/obj/alpathfile.o

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


$(objects): | $(BUILD)/obj


$(libobjects): | $(BUILD)/lib


tests: testjson testhash $(BUILD)/test_alstack testbtree testallist


testbtree: $(BUILD)/testbtree $(TMPTESTDIR)
	$< ceci est un test depuis le makefile 2>$(TMPTESTDIR)/$@.out.2 >$(TMPTESTDIR)/$@.out


testhash: $(BUILD)/hash $(TMPTESTDIR)
	$< $(TEMPLATEHASH)/sample2.txt 2>$(TMPTESTDIR)/$@.sample2.out.2 >$(TMPTESTDIR)/sample2.out
	$< c/c_parser.c 2>$(TMPTESTDIR)/$@.c_parser.out.2 >$(TMPTESTDIR)/c_parser.out
	@$< $(TEMPLATEHASH)/words.txt 2>$(TMPTESTDIR)/$@.words.out.2 >$(TMPTESTDIR)/words.out
	@diff $(TEMPLATEHASH)/words.out $(TMPTESTDIR)/words.out && echo "hash words [OK]"


$(BUILD)/hash:  $(BUILD)/obj/alhash_test.o
	$(LD) -o $@ $(LDFLAGS) $^ -L$(BUILD)/lib -Wl,-Bstatic -lalhash -lalcommon -Wl,-Bdynamic

testjson:$(BUILD)/json $(TMPTESTDIR)
	$< $(TEMPLATE)/test.json >$(TMPTESTDIR)/parse1.json 2>$(TMPTESTDIR)/$@.parse1.json.out.2
	$< $(TEMPLATE)/parse1.json >$(TMPTESTDIR)/parse2.json 2>$(TMPTESTDIR)/$@.parse2.json.out.2
	$< $(TEMPLATE)/refnawak.json >$(TMPTESTDIR)/parse3.json 2>$(TMPTESTDIR)/$@.parse3.json.out.2
	$< $(TEMPLATE)/test.json $(TEMPLATE)/refnawak.json >$(TMPTESTDIR)/$@.test.refnawak.json.out 2>$(TMPTESTDIR)/$@.refnawak.json.out.2
	$< $(TEMPLATE)/refnawak.json $(TEMPLATE)/template.json -debug 2>$(TMPTESTDIR)/$@.parse1.json.out.12 1>&2
	@diff $(TMPTESTDIR)/parse1.json $(TMPTESTDIR)/parse2.json && echo "parse2.json [OK]"
	@diff $(TMPTESTDIR)/parse1.json $(TMPTESTDIR)/parse3.json && echo "parse3.json [OK]"
	$< $(TEMPLATE)/test2.json  >$(TMPTESTDIR)/test2.json 2>$(TMPTESTDIR)/$@.test2.json.out.2

testallist:$(BUILD)/testallist $(TMPTESTDIR)
	$< 10x 10x 10x -decomp >$(TMPTESTDIR)/$@.1000.decomp.out

$(BUILD)/testallist: $(BUILD)/private/obj/tests/allist_test.o
	gcc -g $^ -o $@ -I$(BUILD)/include -L$(BUILD)/lib -Wl,-Bstatic -lallist -lalcommon -laltest -Wl,-Bdynamic

$(BUILD)/tmp:
	mkdir -p $@

$(BUILD)/json: $(objects)
	@echo link json objects $(objects) and libjson
	$(LD) -o $@ $(LDFLAGS) $^ -L$(BUILD)/lib -Wl,-Bstatic -laljson  -lalstack -lalhash -lalcommon  -Wl,-Bdynamic

$(BUILD)/obj:
	mkdir -p $@

$(BUILD)/private/obj:
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

$(BUILD)/private/obj/%.o: c/%.c $(BUILD)/private/obj
	@echo "bad hack fixme" && mkdir $(BUILD)/private/obj/tests
	@echo compile private $< 
	@$(CC) -Wall -c $(CFLAGS) $(CPPFLAGS) -I c/private $< -o $@

clean:
	rm -rf $(BUILD)

.PHONY:clean test libs all tests testjson testhash libinclude

# needed to keep those files within include after make ( remove unused )
.PRECIOUS: $(BUILD)/include/%.h

