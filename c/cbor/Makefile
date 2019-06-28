BUILD=../../build
INCLUDEDIR=$(BUILD)/include

BINARIES=cbor_main
BUILT_BINARIES=$(addprefix $(BUILD)/,$(BINARIES))

$(BUILD)/cbor_main: cbor_main.c alcbor_decoder.c ../aljson_encoder.c
	gcc -I$(INCLUDEDIR) -I.. $^ -o $@  -L$(BUILD)/lib -Wl,-Bstatic -laljson -lalcommon  -laldev -lalstack -lalhash -Wl,-Bdynamic

clean:
	rm -f $(BUILT_BINARIES)

.PHONY: clean
