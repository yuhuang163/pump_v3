#!/bin/bash - 
#===============================================================================
#
#          FILE: gdb-offline.sh
# 
#         USAGE: ./gdb-offline.sh 
# 
#   DESCRIPTION: 
# 
#       OPTIONS: ---
#  REQUIREMENTS: ---
#          BUGS: ---
#         NOTES: ---
#        AUTHOR: YOUR NAME (), 
#  ORGANIZATION: 
#       CREATED: 2019年06月10日 18:08
#      REVISION:  ---
#===============================================================================

#Zset -o nounset                              # Treat unset variables as an error

if [ "x$1" == "x" ] || [ "x$2" == "x" ]
then
    echo "Usage:"
    echo "gdb-offline elf-files crash_log"
    exit 1
fi

elf=$1
orig_crash_file=$2
crash_file=crash_file

file_type=$(awk '$0 ~ /End of crash dump/ {print "text"}' $orig_crash_file)
tmp_file=tmp_file
if [[ $file_type =~ 'text' ]]
then
    awk -f tools/bins/gen-crash.awk $orig_crash_file > $tmp_file
    #delete the time stamp
    sed 's/^\[.*\]//g' $tmp_file > $crash_file
else
    cp $orig_crash_file $crash_file
fi


arm-none-eabi-gdb $elf -ex "set target-charset ASCII" -ex "target remote | ./tools/bins/lin64/CrashDebug --elf $elf --dump $crash_file"
rm -rf $crash_file $tmp_file
