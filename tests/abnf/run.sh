#!/bin/bash

rootdir=../../

[[ -d tmp ]] || mkdir tmp

${rootdir}build/alabnf infile=${rootdir}abnf/rfc3986_part.abnf 2>tmp/alabnf_matcher.1.err

#${rootdir}build/alabnf_matcher 5000 ${rootdir}abnf/testrange.abnf

#${rootdir}build/alabnf_matcher ${rootdir}abnf/rfc3986_part.abnf ${rootdir}abnf/rfc5234_core.abnf

echo "http" | ${rootdir}build/alabnf_matcher 5000 ${rootdir}abnf/rfc5234_core.abnf 2>tmp/alabnf_matcher.2.err
