#!/bin/bash

rm out.dot
rm root0x*.dot

../../build/hashtree 'ceci est la premiere phrase' 'la seconde phrase' 'la troisieme phrase' '4' '5' '6' 'sept' '8' '9' '10' '11' '12' '13' '14' '15' '16' '17' 'et la fin ?'

gvpr -f split.gvpr out.dot

dotty *_18.dot
