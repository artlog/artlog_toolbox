#!/bin/bash

# those two calls should do same thing :

# build/json arg=[template/refnawak.json,template/template.json]
# build/json -- template/refnawak.json template/template.json

# with one difference :
# arg# key is not explicelty set in second case ( kept in argsnumber )

pushd ../..

build/json arg=[template/refnawak.json,template/template.json]
build/json -- template/refnawak.json template/template.json

# duplicate arg NYI
build/json arg=template/refnawak.json arg=template/template.json

popd
