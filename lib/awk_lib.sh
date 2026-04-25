
awk_inplace() {
    local awk_script="$1"
    local file="$2"
    enforce file "$awk_script" exists
    enforce file "$file" exists
    local tmp_file='awkXXXXXXXXXX.out'
    [[ -z $defer ]] && tmp_file=$(mktemp "$tmp_file")
    enforcefile "$tmp_file" exists
    execredirectto $tmp_file awk -f $awk_script $file
    $metarun cp $tmp_file $file
    $metarun rm $tmp_file

    # gawk uniquely
    # if gawk is wanted
    # $metarun apt install gawk

    # $metarun awk -i inplace -f $awk_script $file
}
