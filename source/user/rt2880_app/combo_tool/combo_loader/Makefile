EXEC := wmt_loader
OBJC := loader.c
OBJO := $(OBJC:.c=.o)
CFGFILES := ${PWD}/cfg_folder/WMT.cfg
PATCHPATH := ${PWD}/patch_folder/
PATCHFILES := ROMv2_lm_patch_1_0_hdr.bin \
		ROMv2_lm_patch_1_1_hdr.bin \
		WIFI_RAM_CODE_7623

ifeq ($(CONFIG_ARCH_MT7623),y)
CFLAGS += -DCONFIG_ARCH_MT7623
endif

all: $(EXEC)

$(EXEC): $(OBJO)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f *.o $(EXEC)

romfs:
	$(ROMFSINST) $(EXEC) /bin/$(EXEC)
	$(ROMFSINST) $(CFGFILES) /etc_ro/wmt/WMT_SOC.cfg
	for i in $(PATCHFILES); do $(ROMFSINST) $(PATCHPATH)$$i /etc_ro/wmt/$$i; done
