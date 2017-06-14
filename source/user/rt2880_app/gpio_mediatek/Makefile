EXEC = gpio
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/char
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/include
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/arch/arm/mach-mt7623/include/
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/arch/arm/include/
all: $(EXEC)

$(EXEC): $(EXEC).c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c $(LDLIBS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o

