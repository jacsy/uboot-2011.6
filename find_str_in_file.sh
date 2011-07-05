#!/bin/sh

if [ $# -ne 1 ]; then 
	echo Searching $2 ...
	C_FILES=`find $2 -name "*.[ch]"`
else
	echo Searching current directory ...
	C_FILES=`find . -name "*.[ch]"`
fi

for FILE in $C_FILES
do
	#echo Searching $FILE ...
	grep -l -F "$1"  $FILE
	grep -n -F "$1"  $FILE
done
