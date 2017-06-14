
PRJ_ROOT := $(shell pwd)

BIN_PATH := $(PRJ_ROOT)/bin
BA_PATH := $(PRJ_ROOT)/../blueangel
LINUX_PATH := $(ROOTDIR)/$(LINUXDIR)

TARGETS := btdrv btcust_tab

ifeq ($(CONFIG_USER_BLUEANGEL_BTCLI),y)
TARGETS += btut
endif

ifeq ($(CONFIG_USER_BLUEANGEL_RELAYER),y)
TARGETS += relayer
endif

ifeq ($(CONFIG_DEFAULTS_MEDIATEK_MT7623),y)
ifeq ($(CONFIG_USER_BLUEANGEL_AUTOBT),y)
TARGETS += autobt
endif
endif

TARGETS_CLEAN := $(addsuffix .clean, $(TARGETS))
TARGETS_ROMFS := $(addsuffix .romfs, $(TARGETS))

.PHONY: all clean gen_dirs romfs $(TARGETS) $(TARGETS_CLEAN) $(TARGETS_ROMFS)

all: gen_dirs $(TARGETS)

gen_dirs:
	$(shell mkdir -p $(BIN_PATH))

autobt: btdrv
	$(MAKE) -C $@ CC="$(CC)" ROMFSINST=$(ROMFSINST) BIN_PATH=$(BIN_PATH) BA_PATH=$(BA_PATH) PRJ_ROOT=$(PRJ_ROOT)

btdrv:
	$(MAKE) -C $@ CC="$(CC)" ROMFSINST=$(ROMFSINST) BIN_PATH=$(BIN_PATH) BA_PATH=$(BA_PATH) PRJ_ROOT=$(PRJ_ROOT)

btut: btcust_tab
	$(MAKE) -C $@ CC="$(CC)" ROMFSINST=$(ROMFSINST) BIN_PATH=$(BIN_PATH) BA_PATH=$(BA_PATH) PRJ_ROOT=$(PRJ_ROOT)

btcust_tab:
	$(MAKE) -C $@ CC="$(CC)" ROMFSINST=$(ROMFSINST) BIN_PATH=$(BIN_PATH) BA_PATH=$(BA_PATH) PRJ_ROOT=$(PRJ_ROOT)

relayer: btdrv
	$(MAKE) -C $@ CC="$(CC)" ROMFSINST=$(ROMFSINST) BIN_PATH=$(BIN_PATH) BA_PATH=$(BA_PATH) PRJ_ROOT=$(PRJ_ROOT)

romfs: $(TARGETS_ROMFS)
	$(ROMFSINST) $(BIN_PATH)/libbtcust.so /lib/libbtcust.so
	$(ROMFSINST) $(BIN_PATH)/libaudio.a2dp.blueangel.so /lib/libaudio.a2dp.blueangel.so
	$(ROMFSINST) $(BIN_PATH)/libbluetooth.blueangel.so /lib/libbluetooth.blueangel.so
	$(ROMFSINST) $(BIN_PATH)/libbtstd.so /lib/libbtstd.so
ifeq ($(CONFIG_USER_BLUEANGEL_MTKBT),y)
	$(ROMFSINST) $(BIN_PATH)/mtkbt /sbin/mtkbt
endif

$(TARGETS_ROMFS): %.romfs:
	$(MAKE) -C $* BIN_PATH=$(BIN_PATH) romfs

clean: $(TARGETS_CLEAN)

$(TARGETS_CLEAN): %.clean:
	$(MAKE) -C $* BIN_PATH=$(BIN_PATH) clean

$(warning =============================================)
$(warning CC="$(CC)" )
$(warning ROMFSINST=$(ROMFSINST) )
$(warning PRJ_ROOT=$(PRJ_ROOT) )
$(warning BIN_PATH=$(BIN_PATH) )
$(warning BA_PATH=$(BA_PATH) )
$(warning =============================================)
