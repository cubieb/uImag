EXEC = update

OBJS = update.o cJSON.o

CFLAGS +=-O0
LDFLAGS += -lm

all: $(OBJS)
	$(CC) -o $(EXEC) $(OBJS) $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $<

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	@rm -rf $(EXEC) *.o

