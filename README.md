# artlog_toolbox

Personal project

Some documentation in my personal wiki https://www4.artisanlogiciel.net/dokuwiki/doku.php?id=it:dev:artlog:toolbox

NOT SUITABLE FOR ANY PURPOSE

intention was to cover json_ld with low level code.

covers json and local libraries

A toy toolbox for toy projects in C and java

provides implementations for static libraries
- allist
- alsave
- altest
- json

# json

a json parser that preserve ordering within arrays

## build

to build all and create .h from imports in build/include :

```
make clean
make all
```

result is within build/ directory

/!\ Makefile is not generated, it is plain odd one ;-)
/!\ configure is there to recall you to read this README.md ;-)

## tests

make tests

=> json test , used for pretty printing json content.
can compare two json files ( with limitations, and key order matter ).

with tests directory

runall.sh

## usage

json tool json_path : allow to extract a specific value.

build/json json_path=@id -- samples/json-ld.json 
"http://dbpedia.org/resource/John_Lennon"

## json template

allow to match a json template

build/json template=template/template.json -- template/test.json
{"menu":{"id":"file","value":"File","popup":{"menuitem":[{"value":"New","onclick":"CreateNewDoc()"},{"value":"Open","onclick":"OpenDoc()"},{"value":"Close","onclick":"CloseDoc()"}]}}}
{
  "menu":{
    "id":"file",
    "value":"File",
    "popup":{
      "menuitem":.menu.popup.menuitem.?menu?=[
        {
          "value":"New",
          "onclick":"CreateNewDoc()"
        },
        {
          "value":"Open",
          "onclick":"OpenDoc()"
        },
        {
          "value":"Close",
          "onclick":"CloseDoc()"
        }
      ]
    }
  }
}
template 'template/template.json' and 'template/test.json' json match

# json-ld

aljson_ld

cd c/
make ../build/aljson_ld


../build/aljson_ld list_keywords
../build/aljson_ld get_keyword_index=@id


==> current dev is about iri 
albnf was developped to be able to parse uri using ABNF description of those.
use alabnf within aliri.c to provide iri parsing


../build/aljson_ld dry_run=../samples/ololo.json
[INFO] json_ld is an extension of aljson for json_ld support
[WARNING] NOT YET FULLY IMPLEMENTED
usage:
get key index : get_keyword_index=/keyword/
list keywords : list_keywords=/keyword/
dry_run keywords : dry_run
-----
json input file=../samples/ololo.json
0x556f59639800
{"abc":"edf","json":"crab","ololo":[1,2,3],"subcrab":{"name":"crab","surname":"subcrab"}}
aljson_ld.c:196 NOT YET implemented
[DEBUG] free bucket 0x556f59636bf0
[DEBUG] alhash release 0x556f596362c0 autogrow 200 



../build/aljson_ld list_keywords
[INFO] json_ld is an extension of aljson for json_ld support
[WARNING] NOT YET FULLY IMPLEMENTED
0 @context CONTEXT Used to define the short-hand names that are used throughout a JSON-LD document. These short-hand names are called terms and help developers to express specific identifiers in a compact manner. The @context keyword is described in detail in section 5.1 The Context.
1 @id ID Used to uniquely identify things that are being described in the document with IRIs or blank node identifiers. This keyword is described in section 5.3 Node Identifiers.
2 @value VALUE Used to specify the data that is associated with a particular property in the graph. This keyword is described in section 6.9 String Internationalization and section 6.4 Typed Values.
3 @language LANGUAGE Used to specify the language for a particular string value or the default language of a JSON-LD document. This keyword is described in section 6.9 String Internationalization.
4 @type TYPE Used to set the data type of a node or typed value. This keyword is described in section 6.4 Typed Values.
5 @container CONTAINER Used to set the default container type for a term. This keyword is described in section 6.11 Sets and Lists.
6 @list LIST Used to express an ordered set of data. This keyword is described in section 6.11 Sets and Lists.
7 @set SET Used to express an unordered set of data and to ensure that values are always represented as arrays. This keyword is described in section 6.11 Sets and Lists.
8 @reverse REVERSE Used to express reverse properties. This keyword is described in section 6.12 Reverse Properties.
9 @index INDEX Used to specify that a container is used to index information and that processing should continue deeper into a JSON data structure. This keyword is described in section 6.16 Data Indexing.
10 @base BASE Used to set the base IRI against which relative IRIs are resolved. This keyword is described in section 6.1 Base IRI.
11 @vocab VOCAB Used to expand properties and values in @type with a common prefix IRI. This keyword is described in section 6.2 Default Vocabulary.
12 @graph GRAPH Used to express a graph. This keyword is described in section 6.13 Named Graphs.
13 : X The separator for JSON keys and values that use compact IRIs.
[DEBUG] free bucket 0x563e036bbbf0
[DEBUG] alhash release 0x563e036bb2c0 autogrow 200 


# base64

build it :

c$ make ../build/base64

build/base64
program <name of file to get base64 url> (<debug>)
-d decode
-e encode
-u use base64url scheme ( ie with -_ instead of +/ ) 
in=<input filnename>out=<output filename>, use stdout if not se


build/base64 -u -e  in=filename out=outfile
will create a outfile in current directory with base64 url encoded of filename content

build/base64 -u -d in=filename out=outfile
will create a outfile in current directory with base64 url decoded of filename content

**WARNING** it is **not** -in=filename but in=filename ( ie there is no - ).

what would lead to this not very nivce error :

```
build/base64 -e -in=build/base64 
[TODO] al_options_get_duplicates NYI
[TODO] al_options_get_duplicates NYI
[TODO] al_options_get_duplicates NYI
program <name of file to get base64 url> (<debug>)
-d decode
-e encode
-u use base64url
in=<input filnename>
out=<output filename>, use stdout if not set
```

# cbor

see c/cbor/README 

cd c/cbor
make

## cbor decoding
c/cbor$ for cbf in samples/*.cbor; do ../../build/cbor_main infile=$cbf outfile=$cbf.out; done

## cbor encoding : use inform=json
../../build/cbor_main inform=json infile=../../samples/ololo.json outfile=samples/ololo.cbor

## decode again...
../../build/cbor_main outfile=samples/ololo.json.out infile=samples/ololo.cbor


# abnf


intention is to parse a anbf decription of iri directly for json_ld ... seems vastly too complicated for purpose.

see documentation/alabnf_matcher.README

c/abnf
make all

read/parse a stream of ABNF syntax and create and struct abnf* internal representation of it

build/alabnf infile=abnf/rfc3986_part.abnf

build/alabnf_matcher 5000 abnf/testrange.abnf

build/alabnf_matcher abnf/rfc3986_part.abnf abnf/rfc5234_core.abnf


# c_parser

parses and generate c code ... very messy

see docuementation/c_parser.txt


