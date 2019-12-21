# artlog_toolbox

NOT SUITABLE FOR ANY PURPOSE

a toy toolbox for toy projects in C and java

provides implementations for static libraries
- allist
- alsave
- altest
- json

and provide bash scripts for toy projects

______________


to build all and create .h from imports in build/include :

make all

result is within build/ directory

to retest :

make tests

=> json test , used for pretty printing json content.
can compare two json files ( with limitations, and key order matter ).

_______________

json tool json_path : allow to extract a specific value.

build/json json_path=@id -- samples/json-ld.json 
"http://dbpedia.org/resource/John_Lennon"
_______________

json tool template : allow to match a json template

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

_______________

HOW TO create a new project using current toolbox as tool scripts ?


./createproject.sh <project_name> c

or

./createproject.sh <project_name> java

given type of project create a project in parent directory named after project name

________________


aljson_ld

cd c/
make ../build/aljson_ld


../build/aljson_ld list_keywords
../build/aljson_ld get_keyword_index=@id


==> current dev is about iri 
albnf was developped to be able to parse uri using ABNF description of those.
use alabnf within aliri.c to provide iri parsing




../build/aljson_ld dry_run=/home/plhardy.new/artisanlogiciel/code/artlog_toolbox/samples/ololo.json
[INFO] json_ld is an extension of aljson for json_ld support
[WARNING] NOT YET IMPLEMENTED
usage:
get key index : get_keyword_index=/keyword/
list keywords : list_keywords=/keyword/
dry_run keywords : dry_run
-----
json input file=/home/plhardy.new/artisanlogiciel/code/artlog_toolbox/samples/ololo.json
0x55f78870e7c0

aljson_ld.c:196 NOT YET implemented

__________________________________________________________