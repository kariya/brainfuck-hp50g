INCLUDE_PATH = $(HPGCC)/include
LIBS_PATH = $(HPGCC)/lib
HPAPINE = $(HPAPINEROOT)
export CUR_DIR = $(shell pwd)
export CC = arm-elf-gcc
export C_FLAGS = -mtune=arm920t -mcpu=arm920t \
	-mlittle-endian -fomit-frame-pointer -msoft-float -Wall \
	-Os -I$(INCLUDE_PATH) -L$(LIBS_PATH) -mthumb-interwork -mthumb 
export LD = arm-elf-ld
export LD_FLAGS = -L$(LIBS_PATH) -T VCld.script $(LIBS_PATH)/crt0.o 
export LIBS = -lhpg -lhplib -lgcc
export AR = arm-elf-ar
export RANLIB = arm-elf-ranlib
export ELF2HP = elf2hp

SRC = $(shell echo *.c)

OBJ = $(SRC:%.c=%.o)

EXE = $(SRC:%.c=%.exe)

HP = $(SRC:%.c=%.hp)

all: $(HP)

install: all

clean:
	-rm -f *.o
	-rm -f *.hp
	-rm -f *.exe

%.o: %.c
	$(CC) $(C_FLAGS) -c $<

%.exe: %.o
	$(LD) $(LD_FLAGS) $< $(LIBS) -o $@

%.hp: %.exe
	$(ELF2HP) $< $@

%_local: %.c
	gcc -Wall -g -o $@ $< -DHPGCC -I $(HPAPINE)/include/ \
	-DHPAPINE -L $(HPAPINE)/lib/ -lhpapine \
	$(shell pkg-config --libs gdk-2.0 gthread-2.0)
%.c: %.asm
	../../bin/dasm.sh $< > $@
