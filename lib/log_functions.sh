# SHOULD BE INCLUDED WITH source or . from a bash script

# EXAMPLE
# log_functions=./log_functions.sh
# [[ -f $log_functions ]] || { echo "[FATAL] Missing $log_functions" >&2 ; exit 1 ;}
# source $log_functions

metalog_color_start() {
    if [[ -n $metalog_color ]]
    then
	echo -en "${metalog_color}"
    fi
}

metalog_color_stop() {
    if [[ -n $metalog_color ]]
    then
	echo -en "\033[0m"
    fi
}

log_any()
{
  priority=$1
  shift
  metalog_color_start
  echo "[$priority] $@" >&2
  metalog_color_stop
}

log_fatal()
{
   local metalog_color=$metalog_color_error
   log_any FATAL "$*"
}

log_error()
{
   local metalog_color=$metalog_color_error
   log_any ERROR "$*"
}

log_warn()
{
   local metalog_color=$metalog_color_warning
   log_any WARN "$*"
}

log_info()
{
   log_any INFO "$*"
}

log_success()
{
    local metalog_color=$metalog_color_success
    log_info "$*"
}

log_debug()
{
    local metalog_color=$metalog_color_info
    [[ -n $debug ]] && log_any DEBUG "$*"
}

verbose()
{
    [[ -n $verbose ]] && log_any $verbose $@
}

metalog_no_colors()
{
    metalog_color_info=
    metalog_color_success=
    metalog_color_error=
    metalog_color_warning=
}

# default colors
metalog_default_colors()
{
    metalog_color_info="\033[38;5;79m"
    metalog_color_success="\033[1;32m"
    metalog_color_error="\033[1;31m"
    metalog_color_warning="\033[1;34m"
}
