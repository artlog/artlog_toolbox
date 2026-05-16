# INSTALL

within parent directory

make all

for more information see ../README.txt

# Usage

```
Output : dump parsed json to standard output.
-d                         debug
-m                         non recursive
-c                         check only (no print)
-b                         bare, no indent
maxdepth=<integer value for max depth>     over maxdepth switch to non recursive
out=<output filename>, use stdout if not set
indent=flat|spaces[:x]|tabs    indentation flat or with x ( default 3 ) spaces or with tabs 
space_after   add a space after : of a pair
json_path=<path>
template=filename          file to open in read only mode to parse in json for template.
          template is used for json unification ie extracting fields from a template pattern
-- to separate options from arguments
First argument filename    file to open in read only mode to parse in json, use '-' for stdin.

aljson_main version 0.3.0
```

## format output

build/json -- template/list.json 
["a","b","c","d","e"]p

```
build/json indent=spaces:2 -- template/parse1.json
indent set to 2 spaces
{
  "menu":{
    "id":"file",
    "value":"File",
    "popup":{
      "menuitem":[
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
```

check : checks file is a uses a valid json syntax.

## JSON path

very limited implementation.

does not handle '$' '@' , comparisons or filters.

assuming it is always from root ($).

To extract a value from a json hierarchy by specifying a path to value.

```
build/json arg=[template/parse1.json] 2>/dev/null
{"menu":{"id":"file","value":"File","popup":{"menuitem":[{"value":"New","onclick":"CreateNewDoc()"},{"value":"Open","onclick":"OpenDoc()"},{"value":"Close","onclick":"CloseDoc()"}]}}}
```

```
build/json json_path=menu.value arg=[template/parse1.json] 2>/dev/null
"File"
```

```
build/json json_path=menu.popup.menuitem[1] arg=[template/parse1.json] 2>/dev/null
{"value":"Open","onclick":"OpenDoc()"
```

```
build/json json_path=menu.popup.menuitem[1].onclick arg=[template/parse1.json] 2>/dev/null
"OpenDoc()"
```

```
build/json json_path= arg=[template/parse1.json] 2>/dev/null
```

## JSON template

unify

```
cat template/template.json
{
    "menu":
    {
        "id": "file",
        "value": "File",
        "popup":
        {
            "menuitem":
	    ?menu?
        }
    }
}
```

?menu? is a variable that will get corresponding content from json.

earlier version did that :
```
build/json -c template=template/template.json -- template/refnawak.json 2>/dev/null
.menu.popup.menuitem.?menu?=[{"value":"New","onclick":"CreateNewDoc()"},{"value":"Open","onclick":"OpenDoc()"},{"value":"Close","onclick":"CloseDoc()"}]
```

latest behavior ( seems expected output to err is done to stdout )

```
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
template 'template/template.json' and 'template/refnawak.json' json match
```