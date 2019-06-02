#!/bin/bash

rm out.dot

../../build/hashtree 'ceci est la premiere pharse' 'la seconde phrase' 'la troisieme phrase' '4' '5' '6' 'sept'

gvpr -f split.gvpr out.dot

dotty *_7.dot
