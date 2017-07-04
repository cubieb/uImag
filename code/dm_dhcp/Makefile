EXE = dm_dhcp
OBJS = dm_dhcp.o

CFLAGS +=-O0

CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/include -I$(ROOTDIR)/lib/libnvram
LDFLAGS += -L$(ROOTDIR)/lib/libnvram -L.
LDNVRAMLIB += -lnvram 

all: $(OBJS) 
	$(CC) -o $(EXE) $(OBJS) $(LDFLAGS) $(LDNVRAMLIB)

.c.o:
	$(CC) $(CFLAGS) -c $<

romfs:
	$(ROMFSINST) /bin/$(EXE)

clean:
	@rm -rf $(EXE) *.o

