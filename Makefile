#vpath %.c ../src

CC=gcc
LD=gcc
CPPFLAGS=-g

BUILD=build

src=c/json.c c/main.c

objects=$(patsubst c/%.c,$(BUILD)/obj/%.o,$(src))

$(objects): | $(BUILD)/obj

test:$(BUILD)/json
	mkdir -p $@
	./$(BUILD)/json test.json >test/parse1.json
	./$(BUILD)/json parse1.json >test/parse2.json
	./$(BUILD)/json refnawak.json >test/parse3.json
	diff test/parse1.json test/parse2.json
	diff test/parse1.json test/parse3.json

$(BUILD)/json: $(objects)
	@echo link json objects $(objects)
	$(LD) $(LDFLAGS) $^ -o $@

$(BUILD)/obj:
	mkdir -p $@

$(BUILD)/obj/%.o: c/%.c
	@echo compile $< 
	@$(CC) -Wall -c $(CFLAGS) $(CPPFLAGS) $< -o $@

clean:
	rm -f $(objects)

.PHONY:clean test




