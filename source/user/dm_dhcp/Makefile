EXE = dm_dhcp
OBJS = dm_dhcp.o

CFLAGS +=-O0

all: $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $<

romfs:
	$(ROMFSINST) /bin/$(EXE)

clean:
	@rm -rf $(EXE) *.o

