BUILD=../../build

BINARIES=alabnf alabnf_matcher
BUILT_BINARIES=$(addprefix $(BUILD)/,$(BINARIES))
CDEPS=../altoken.c ../altokenizer.c ../alinput_util.c

all: $(BUILT_BINARIES)

$(BUILD)/alabnf: alabnf.c alabnf_util.c alabnf_test.c $(CDEPS)  alabnf_dump.c
	gcc -Wall -Wextra -g -I.. -o $@ $^ -L$(BUILD)/lib  -Wl,-Bstatic -laldev -laljson -lalhash  -lalstack -lalcommon -laldev -Wl,-Bdynamic

$(BUILD)/alabnf_matcher: alabnf.c alabnf_util.c alabnf_matcher.c alabnf_matcher_test.c $(CDEPS) alabnf_dump.c
	gcc -Wall -g -I.. -o $@ $^ -L$(BUILD)/lib -Wl,-Bstatic -laldev -laljson -lalhash  -lalstack -lalcommon  -Wl,-Bdynamic

clean:
	rm $(BUILT_BINARIES)

.PHONY: clean all
