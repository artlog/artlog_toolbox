# convert from a c struct to a json parser c code using this libjson.a

intermediate representation of c struct :

```
struct test {
       char * a[5];
       };
       
"def_struct":{
"name":"test"
"members":["a":{
		"c_type":["array":{arity:5},"pointer","char"]
		}		
	]
}
```

```
struct test2 {
       struct test1 ** b[];
       char c;
}

"def_struct":{
"name":"test2",
"members":[
	"b":{"c_type":["array":{"arity":null},"pointer","pointer","struct":{"name":"test"}]}
	"c":{"c_type":["char"]}
]
}
```
_______________


map string <-> enum

when stored in json enum are stored as string

"enum":{
"alconst1":{
"ALCONST_TEST":{
	"name":"test",
	"constant":,
	"value":1	
	}
	}

to use as input for c generation ...

should it remains an external resource (file) or embedded in storing/restoring c code ?
==> internal

______________________________


current status :

```
./build/c_parser infile=./c/input_for_c_parser.h
[INFO] file to parse './c/input_for_c_parser.h'
struct  [DEBUG]test_1{ // type 4 definition. (code line 1939)
[TODO] fixme hardcoded 1000 structures
// struct definition 
int   [DEBUG]a // struct member 0
;int   [DEBUG]b // struct member 1
;int   [DEBUG]c // struct member 2
;char  * [DEBUG]d // struct member 3
;} // close struct
;struct  [DEBUG]test_2{ // type 4 definition. (code line 1939)
[TODO] fixme hardcoded 1000 structures
// struct definition 
int   [DEBUG]aSAME TOKEN SEEN
 // struct member 0
;struct  [DEBUG]test_1SAME TOKEN SEEN
 [DEBUG]cSAME TOKEN SEEN
 // struct member 1
;struct  [DEBUG]test_1SAME TOKEN SEEN
* [DEBUG]e // struct member 2
;struct  [DEBUG]test_1SAME TOKEN SEEN
* [DEBUG]f // struct member 3
;} // close struct
;struct  [DEBUG]flat_info{ // type 4 definition. (code line 1939)
[TODO] fixme hardcoded 1000 structures
// struct definition 
char  * [DEBUG]name // struct member 0
;char  * [DEBUG]imafile // struct member 1
;char  * [DEBUG]cubemappingfile // struct member 2
;} // close struct
;EOF id reached// non NULL token at toplevel parsing
```
