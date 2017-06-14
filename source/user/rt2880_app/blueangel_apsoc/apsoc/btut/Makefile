
# Makefile for btut #

CUR_PATH := $(shell pwd)

CFLAGS = \
	-g \
	-Wall \
	-fshort-wchar \
	-DBTMTK_ON_LINUX \
	-D__LINUX_SUPPRESS_ERROR__ \
	$(NULL)

$(info CONFIG_USER_BLUEANGEL=$(CONFIG_USER_BLUEANGEL))
ifeq ($(CONFIG_USER_BLUEANGEL),y)
CFLAGS += -DRPLAYER_TEST
$(info rplayer will be built: CFLAGS=$(CFLAGS))
endif

LDFLAGS = \
	-Wl,--no-fatal-warnings

LIBS := \
	-L$(BIN_PATH) \
	-laudio.a2dp.blueangel \
	-lbluetooth.blueangel \
	-lbtcust \
	-lbtcust_tab \
	-lbtstd \
	-ldl \
	-lpthread \
	-lrt \
	$(NULL)

INCS := \
	-I$(CUR_PATH)/inc \
	-I$(PRJ_ROOT)/btif/inc \
	$(NULL)

SRCS := \
	$(CUR_PATH)/src/a2dp/btut_a2dp_snk_if.c \
	$(CUR_PATH)/src/btut_cli.c \
	$(CUR_PATH)/src/btut_utl.c \
	$(CUR_PATH)/src/edit.c \
	$(CUR_PATH)/src/eloop.c \
	$(CUR_PATH)/src/gap/btut_gap_if.c \
	$(CUR_PATH)/src/gatt/btut_gatt_if.c \
	$(CUR_PATH)/src/gatt/btut_gatt_client_if.c \
	$(CUR_PATH)/src/gatt/btut_gatt_server_if.c \
	$(CUR_PATH)/src/btut_debug.c \
	$(NULL)

SRC_DIRS := $(sort $(dir $(SRCS)))

OBJ_PATH := $(CUR_PATH)/obj

OBJS := $(addprefix $(OBJ_PATH)/, $(notdir $(SRCS:.c=.o)))

RES_PATH := $(CUR_PATH)/res

VPATH = $(SRC_DIRS)

.PHONY: all clean gen_dirs romfs

all: gen_dirs $(OBJS) $(BIN_PATH)/btcli
	@echo BTUT_SRC_DIRS:
	@echo $(SRC_DIRS) | fmt -1
	@echo BTUT_OBJS:
	@echo $(OBJS) | fmt -1

gen_dirs:
	$(shell mkdir -p $(OBJ_PATH))

-include $(OBJS:.o=.d)

$(OBJ_PATH)/%.o: %.c
	$(CC) $(CFLAGS) $(INCS) -c -o $@ $<
	$(CC) $(INCS) -MM $< > $(OBJ_PATH)/$*.d
	@mv -f $(OBJ_PATH)/$*.d $(OBJ_PATH)/$*.d.tmp
	@sed -e 's|.*:|$@:|' < $(OBJ_PATH)/$*.d.tmp > $(OBJ_PATH)/$*.d
	@sed -e 's/.*://' -e 's/\\$$//'< $(OBJ_PATH)/$*.d.tmp | fmt -1 | \
	 sed -e 's/^ *\([^ \n\r\t]\+\)/\1:/' >> $(OBJ_PATH)/$*.d
	@rm -f $(OBJ_PATH)/$*.d.tmp

$(BIN_PATH)/btcli: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

romfs: 
	$(ROMFSINST) $(BIN_PATH)/btcli /sbin/btcli

clean:
	rm -f $(OBJ_PATH)/*
	rm -f $(BIN_PATH)/btcli
	@echo btut cleaned.
