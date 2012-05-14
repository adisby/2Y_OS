#!/bin/bash
#Program : add_include.sh
#Usage	 : this file is for add #include XXX in a c source file

usage()
{
	echo "usage: $0 after_what add_what"
}

if [ $# -lt 2 ]
then
	usage
	exit 0
fi

AFTER_WHAT=$1
ADD_WHAT=$2

command="sed -i '/#include \"$AFTER_WHAT\"/a#include \"$ADD_WHAT\"' kernel/*.c"
eval $command
command="sed -i '/#include \"$AFTER_WHAT\"/a#include \"$ADD_WHAT\"' fs/*.c"
eval $command
command="sed -i '/#include \"$AFTER_WHAT\"/a#include \"$ADD_WHAT\"' mm/*.c"
eval $command
command="sed -i '/#include \"$AFTER_WHAT\"/a#include \"$ADD_WHAT\"' lib/*.c"
eval $command
