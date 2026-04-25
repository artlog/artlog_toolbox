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

# map string <-> enum

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

# current status :

```
./build/c_parser infile=./c/input_for_c_parser.h
struct  test_1 { // type 4 definition. (code line 1950)
[TODO] fixme hardcoded 1000 structures
// struct definition 
int   a ; // struct member 0
int   b ; // struct member 1
int   c ; // struct member 2
char  * d ; // struct member 3
};
struct  test_2 { // type 4 definition. (code line 1950)
[TODO] fixme hardcoded 1000 structures
// struct definition 
int   abcdtest_2 ; // struct member 0
struct   cdtest_2 ; // struct member 1
struct  * e ; // struct member 2
struct  * f ; // struct member 3
};
struct  flat_info { // type 4 definition. (code line 1950)
[TODO] fixme hardcoded 1000 structures
// struct definition 
char  * name ; // struct member 0
char  * imafile ; // struct member 1
char  * cubemappingfile ; // struct member 2
};
```
