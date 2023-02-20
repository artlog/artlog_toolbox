
# cbor

https://cbor.io/


## decode cbor -> json

then should find api in aljson to build json object.

```
json_to_c_stub.h , json_encoder.h
```

some samples within samples dir

```
make ../../build/cbor_main
../../build/cbor_main infile=samples/one.cbor outfile=one.json
cat one.json
```

```
make tests
```

## encode json -> cbor


