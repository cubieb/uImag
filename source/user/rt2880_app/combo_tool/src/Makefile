EXEC := wmt_launcher wmt_lpbk wmt_concurrency
#OBJO := stp_uart_launcher.o \
	wmt_concurrency.c \
	wmt_loopback.c

#OBJS := $(OBJC:.c=.o)
$(warning haman here was compiled)

ifeq ($(CONFIG_ARCH_MT7623),y)
CFLAGS += -DCONFIG_ARCH_MT7623
endif

all: $(EXEC)

wmt_launcher: stp_uart_launcher.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

wmt_lpbk: wmt_loopback.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

wmt_concurrency: wmt_concurrency.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f *.o $(EXEC)

romfs:
	for i in $(EXEC) ; do $(ROMFSINST) $$i /bin/$$i ; done
