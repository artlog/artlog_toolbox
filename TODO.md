
cat samples/ololo.json
{
    "abc": "edf",
    "json": "crab",
    "ololo": [ 1, 2, 3 ],
    "subcrab": {
        "name": "crab",
        "surname": "subcrab"
    }
}

```
build/aljson_ld dry_run=samples/ololo.json
[INFO] json_ld is an extension of aljson for json_ld support
[WARNING] NOT YET FULLY IMPLEMENTED
usage:
get key index : get_keyword_index=/keyword/
list keywords : list_keywords=/keyword/
dry_run : dry_run=<inputfile>
-----
json input file=samples/ololo.json
0x59c70e6d7800
{"abc":"edf","json":"crab","ololo":[1,2,3],"subcrab":{"name":"crab","surname":"subcrab"}}
aljson_ld.c:194 aljson_ld_build_from_json NOT YET implemented. complete iri parsing please.
[DEBUG] free bucket 0x59c70e6d4bf0
[DEBUG] alhash release 0x59c70e6d42c0 autogrow 200
```
___________________________________

TODO iri parsing

only iri parsing missing ??? i doubt so.

current try was to implement own abnf parser : working on but seems too much ...

studies :
- ABNF other parsers to parse iri
not yet conclusive
- to take alook on curl ... seems it deserves it ... ( but idea was to get no dependency at all for self-training ).

Might be many more things TODO than only on json_ld ...

take a look into workinprogress directory ...

seems very few progress there...

