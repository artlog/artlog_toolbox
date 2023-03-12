

../../build/hashtree
Don't use, even at your own risks
add each argument at new rightmost element of btree with hash256 value then dump it.

create a out.dot containing a dotty representation of this hashtree and out.dbg for traces.

dotty / kgraphviewer file:out.dot

dot -Tpng out.dot  >out.png

BUG : only one node is displayed while many digraph are set in out.dot...

gvpr -f split.gvpr out.dot

```
BEG_G {
  fname = sprintf("%s.dot",$G.name);
  writeG($G, fname);
}
```

dotty *_18.dot
