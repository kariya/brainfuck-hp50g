#! /bin/sh

## customize the following lines for your architecure:
CC=arm-elf-gcc
CFLAGS=
OBJDUMP=arm-elf-objdump


asm() {
	TMP_C=$(mktemp /tmp/XXXXX.c)
	TMP_O=$(mktemp /tmp/XXXXX.o)

	echo "void fsck() {" > $TMP_C
	echo "asm(\"me: $@\");" >> $TMP_C
	echo "}" >> $TMP_C

	$CC $CFLAGS -g -c $TMP_C -o $TMP_O
	$OBJDUMP -S ${TMP_O} | grep "asm(" -A1 | tail -1 | gawk 'BEGIN{FS="\t"} {print $2}' | sed 's/ *$//' | sed 's/^/0x/' | sed 's/ /,0x/g'

	rm -f $TMP_O $TMP_C
	
}

IFS="" 
while read -r l
do
	if echo "$l" | grep '\[\[.*]]' > /dev/null; then
		A=$(echo "$l" | sed 's/^.*\[\[\(.*\)]].*$/\1/')
		C=$(asm $A)
		ARGC=$(echo $C | gawk 'BEGIN{FS=","} {print NF}')
		echo "$l 	/* $A */" | sed "s/\[\[.*]]/$ARGC,$C/" | sed 's///g'
	else
		echo "$l"
	fi
done < $1
		
