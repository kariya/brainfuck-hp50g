#! /bin/sh

CC=arm-elf-gcc
OBJDUMP=arm-elf-objdump

asm() {
	TMP_C=$(mktemp /tmp/XXXXX.c)
	TMP_O=$(mktemp /tmp/XXXXX.o)

	echo "void fsck() {" > $TMP_C
	echo "asm(\"$@\");" >> $TMP_C
	echo "}" >> $TMP_C

	$CC -mthumb -g -c $TMP_C -o $TMP_O
	$OBJDUMP -S ${TMP_O} | grep "asm(" -A1 | tail -1 | sed 's/^.*:[^0-9a-f]*\(....\).*$/\1/' 

	rm -f $TMP_O $TMP_C
	
}

IFS="" 
while read l
do
	if echo "$l" | grep '\[\[.*]]' > /dev/null; then
		A=$(echo "$l" | sed 's/^.*\[\[\(.*\)]].*$/\1/')
		C=$(asm $A)
		echo "$l 	/* $A */" | sed "s/\[\[.*]]/0x$C/" | sed 's///g'
	else
		echo "$l"
	fi
done < $1
		
