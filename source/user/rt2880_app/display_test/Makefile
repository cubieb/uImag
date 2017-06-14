EXEC = display
OBJS = display.o
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/video/

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) 

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o