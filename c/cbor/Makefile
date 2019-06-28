BUILD=../../build
INCLUDEDIR=$(BUILD)/include

BINARIES=cbor_main
BUILT_BINARIES=$(addprefix $(BUILD)/,$(BINARIES))
OUTTESTDIR=out

$(BUILD)/cbor_main: cbor_main.c alcbor_decoder.c ../aljson_encoder.c
	gcc -I$(INCLUDEDIR) -I.. $^ -o $@  -L$(BUILD)/lib -Wl,-Bstatic -laljson -lalcommon  -laldev -lalstack -lalhash -Wl,-Bdynamic

clean:
	rm -f $(BUILT_BINARIES)

tests: $(BUILD)/cbor_main
	rm -rf $(OUTTESTDIR)
	mkdir $(OUTTESTDIR)
	$< infile=samples/one.cbor outfile=$(OUTTESTDIR)/1one.json
	$< infile=samples/emptyarray.cbor outfile=$(OUTTESTDIR)/2ea.json
	$< infile=samples/oneelementarray.cbor outfile=$(OUTTESTDIR)/3oea.json
	$< infile=samples/twoeltsarray.cbor outfile=$(OUTTESTDIR)/4tea.json
	cat $(OUTTESTDIR)/*.json

.PHONY: clean tests
