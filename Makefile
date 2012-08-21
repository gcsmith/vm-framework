#
# SCCS ID: @(#)Makefile	1.9	04/04/08
#
# Makefile to control the compiling, assembling and linking of
# standalone programs in the DSL.  Used for both 4003-406 and
# 4003-506 (with appropriate tweaking).
#

#
# External definitions
#
USB_DEV ?= /dev/null

#
# User supplied files
#
U_C_SRC = clock.c klibc.c processes.c queues.c scheduler.c sio.c stacks.c \
	syscalls.c system.c ulibc.c user.c segment.c gdt_support.c elf.c
U_C_OBJ = clock.o klibc.o processes.o queues.o scheduler.o sio.o stacks.o \
	syscalls.o system.o ulibc.o user.o segment.o gdt_support.o elf.o
U_S_SRC = klibs.S ulibs.S
U_S_OBJ = klibs.o ulibs.o
U_LIBS	=

#TODO: integrate this into the build system better
USER_OBJS = ustart.o ulibs.o ulibc.o
USER_PROG = user1 user2 user3
IMG_OBJS  = hswap.b 0x3000	\
            kern.b  0x10000	\
			user1   0x40000	\
			user2   0x50000	\
			user3   0x60000

UTILS = BuildImage Offsets ElfTest

#
# User compilation/assembly definable options
#
#	ISR_DEBUGGING_CODE	include context restore debugging code
#	CLEAR_BSS_SEGMENT	include code to clear all BSS space
#	SP2_CONFIG		enable SP2-specific startup variations
#
USER_OPTIONS = -DCLEAR_BSS_SEGMENT -DISR_DEBUGGING_CODE -DSP2_CONFIG

#
# YOU SHOULD NOT NEED TO CHANGE ANYTHING BELOW THIS POINT!!!
#
# Compilation/assembly control
#

#
# We only want to include from the current directory and ~wrc/include
#
INCLUDES = -I. -I./include

#
# Compilation/assembly/linking commands and options
#
CPP = cpp
CPPFLAGS = $(USER_OPTIONS) -nostdinc -iquote $(INCLUDES)

CC = gcc
CFLAGS = -fno-builtin -ffreestanding -Wall -Wstrict-prototypes -m32 $(CPPFLAGS)

# Comment this line out for GCC 3 or below
CFLAGS += -fno-stack-protector

# Comment this line out to remove debugging symbols
CFLAGS += -gstabs

AS = as
ASFLAGS =

LD = ld
LFLAGS = -m elf_i386

EMU = qemu
EFLAGS = -drive file=hdrive.image,format=raw,index=1,if=ide -s -boot c -m 128 \
		 -drive file=floppy.image,format=raw,index=0,if=floppy

OD = objdump
OC = objcopy

#		
# Transformation rules - these ensure that all compilation
# flags that are necessary are specified
#
# Note use of 'cpp' to convert .S files to temporary .s files: this allows
# use of #include/#define/#ifdef statements. However, the line numbers of
# error messages reflect the .s file rather than the original .S file. 
# (If the .s file already exists before a .S file is assembled, then
# the temporary .s file is not deleted.  This is useful for figuring
# out the line numbers of error messages, but take care not to accidentally
# start fixing things by editing the .s file.)
#

.SUFFIXES:	.S .b

.c.s:
	$(CC) $(CFLAGS) -S $*.c

.S.s:
	$(CPP) $(CPPFLAGS) -o $*.s $*.S

.S.o:
	$(CPP) $(CPPFLAGS) -o $*.s $*.S
	$(AS) -o $*.o $*.s -a=$*.lst --32

.s.b:
	$(AS) -o $*.o $*.s -a=$*.lst --32
	$(LD) $(LFLAGS) -Ttext 0x0 -s --oformat binary -e begtext -o $*.b $*.o

.c.o:
	$(CC) $(CFLAGS) -c $*.c

# Binary/source file for system bootstrap code

BOOT_OBJ = bootstrap.b
BOOT_SRC = bootstrap.S

# Binary/source file for system hotswap code

SWAP_OBJ = hswap.b
SWAP_SRC = hswap.S

# Assembly language object/source files

S_OBJ = startup.o isr_stubs.o $(U_S_OBJ)
S_SRC =	startup.S isr_stubs.S $(U_S_SRC)

# C object/source files

C_OBJ =	c_io.o support.o $(U_C_OBJ)
C_SRC =	c_io.c support.c $(U_C_SRC)

# Collections of files

OBJECTS = $(S_OBJ) $(C_OBJ)

SOURCES = $(BOOT_SRC) $(SWAP_SRC) $(S_SRC) $(C_SRC)

#
# Targets for remaking bootable image of the program
#
# This is the default target
#

all: floppy.image hdrive.image

#
# Run under QEMU (no debugging), piping serial to standard out
#
qrun: floppy.image hdrive.image
	$(EMU) $(EFLAGS) -serial stdio &

#
# Run under QEMU (no debugging), establish a telnet session for I/O
#
qruns: floppy.image hdrive.image
	$(EMU) $(EFLAGS) -serial telnet:localhost:4321,server &
	@sleep 1s
	@telnet localhost 4321

#
# Run under QEMU with debugging through a remote GDB session
#
qdbg: floppy.image hdrive.image
	$(EMU) $(EFLAGS) -S &
	@sleep 1s
	@gdb

floppy.image: bootstrap.b hswap.b kern.b kern.nl $(USER_PROG) $(UTILS)
	./BuildImage -o floppy.image bootstrap.b 0x00 $(IMG_OBJS)

hdrive.image: bootstrap.b hswap.b kern.b kern.nl $(USER_PROG) $(UTILS)
	./BuildImage -o hdrive.image bootstrap.b 0x80 $(IMG_OBJS)

kern.out: $(OBJECTS)
	$(LD) $(LFLAGS) -o kern.out $(OBJECTS)

kern.o:	$(OBJECTS)
	$(LD) $(LFLAGS) -o kern.o -Ttext 0x10000 $(OBJECTS) $(U_LIBS)

kern.b:	kern.o
	$(OD) -S kern.o > kern.lst
	$(LD) $(LFLAGS) -o kern.b -s --oformat binary -Ttext 0x10000 kern.o

hswap.b: hswap.o
	$(OD) -S hswap.o > hswap.lst
	$(LD) $(LFLAGS) -o hswap.b -s --oformat binary -Ttext 0x3000 hswap.o

user1: $(USER_OBJS) user1.o
	$(LD) $(LFLAGS) -o user1 -s -Ttext 0x1000 $(USER_OBJS) user1.o

user2: $(USER_OBJS) user2.o
	$(LD) $(LFLAGS) -o user2 -s -Ttext 0x1000 $(USER_OBJS) user2.o

user3: $(USER_OBJS) user3.o
	$(LD) $(LFLAGS) -o user3 -s -Ttext 0x1000 $(USER_OBJS) user3.o

#
# Target for copying bootable image onto a floppy
#

floppy:	floppy.image
	dd if=floppy.image of=/dev/fd0

flash: hdrive.image
	dd if=hdrive.image of=$(USB_DEV)

#
# Special rule for creating the modification and offset programs
#
# These are required because we don't want to use the same options
# as for the standalone binaries.
#

BuildImage:	BuildImage.c
	$(CC) -o BuildImage BuildImage.c

Offsets:	Offsets.c
	$(CC) $(INCLUDES) -o Offsets Offsets.c

ElfTest:	ElfTest.c elf.h
	$(CC) $(INCLUDES) -o ElfTest ElfTest.c

#
# Clean out this directory
#

clean:
	rm -f *.nl *.lst *.b *.o *.s *.image *.dis $(USER_PROG) $(UTILS)

realclean:	clean

#
# Create a printable namelist from the kern.o file
#

kern.nl: kern.o
	nm -Bn kern.o | pr -w80 -3 > kern.nl
#	nm -Bng kern.o | pr -w80 -3 > kern.nl

#
# Generate a disassembly
#

kern.dis: kern.o
	dis kern.o > kern.dis

#
#       makedepend is a program which creates dependency lists by
#       looking at the #include lines in the source files
#

depend:
	makedepend $(INCLUDES) $(SOURCES)

# DO NOT DELETE THIS LINE -- make depend depends on it.

bootstrap.o: bootstrap.h
startup.o: bootstrap.h
isr_stubs.o: bootstrap.h
ulibs.o: syscalls.h ./include/x86arch.h
c_io.o: c_io.h types.h startup.h support.h ./include/x86arch.h
support.o: startup.h support.h c_io.h types.h ./include/x86arch.h bootstrap.h
clock.o: headers.h defs.h types.h c_io.h support.h klib.h clock.h processes.h
clock.o: segment.h stacks.h scheduler.h queues.h startup.h
clock.o: ./include/x86arch.h
klibc.o: headers.h defs.h types.h c_io.h support.h klib.h
processes.o: headers.h defs.h types.h c_io.h support.h klib.h processes.h
processes.o: segment.h stacks.h queues.h bootstrap.h
queues.o: headers.h defs.h types.h c_io.h support.h klib.h queues.h
queues.o: processes.h segment.h stacks.h
scheduler.o: headers.h defs.h types.h c_io.h support.h klib.h scheduler.h
scheduler.o: processes.h segment.h stacks.h queues.h sio.h
sio.o: headers.h defs.h types.h c_io.h support.h klib.h sio.h processes.h
sio.o: segment.h stacks.h queues.h scheduler.h startup.h ./include/uart.h
sio.o: ./include/x86arch.h
stacks.o: headers.h defs.h types.h c_io.h support.h klib.h stacks.h queues.h
stacks.o: processes.h segment.h
syscalls.o: ./include/x86arch.h headers.h defs.h types.h c_io.h support.h
syscalls.o: klib.h startup.h syscalls.h clock.h processes.h segment.h
syscalls.o: stacks.h queues.h scheduler.h sio.h system.h bootstrap.h
syscalls.o: gdt_support.h elf.h ulib.h
system.o: headers.h defs.h types.h c_io.h support.h klib.h ulib.h processes.h
system.o: segment.h stacks.h system.h clock.h queues.h scheduler.h sio.h
system.o: syscalls.h ./include/x86arch.h user.h bootstrap.h startup.h
ulibc.o: headers.h defs.h types.h c_io.h support.h klib.h
user.o: headers.h defs.h types.h c_io.h support.h klib.h user.h processes.h
user.o: segment.h stacks.h bootstrap.h
segment.o: headers.h defs.h types.h c_io.h support.h klib.h bootstrap.h
segment.o: segment.h gdt_support.h
gdt_support.o: gdt_support.h types.h bootstrap.h defs.h
elf.o: elf.h klib.h types.h c_io.h
