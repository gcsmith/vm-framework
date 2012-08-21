symbol-file kern.o

# switch to high level debugging mode
define cmode
    set architecture i386
    undisplay
end

# switch to 32-bit protected mode
define pmode
    set architecture i386
    undisplay
    display/i $pc
end

# switch to 16-bit real mode
define rmode
    set architecture i8086
    undisplay
    display/i $cs*16+$eip
end

# start up in real mode and attach to qemu remote session
rmode
target remote localhost:1234

