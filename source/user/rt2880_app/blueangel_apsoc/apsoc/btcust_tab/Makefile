
# Makefile for libbtcust_tab.so #

CUR_PATH := $(shell pwd)

CFLAGS = \
	-g \
	-Wall \
	-fshort-wchar \
	-D__LINUX_SUPPRESS_ERROR__ \
	-D__BT_4_0_BLE__ \
	$(NULL)

LDFLAGS = \
	-Wl,--no-fatal-warnings

INCS := \
	-I$(CUR_PATH)/inc \
	$(NULL)

SRCS := \
	$(CUR_PATH)/src/bt_cust.c \
	$(NULL)

SRC_DIRS := $(sort $(dir $(SRCS)))

OBJ_PATH := $(CUR_PATH)/obj

OBJS := $(addprefix $(OBJ_PATH)/, $(notdir $(SRCS:.c=.o)))

VPATH = $(SRC_DIRS)

.PHONY: all clean gen_dirs romfs

all: gen_dirs $(OBJS) $(BIN_PATH)/libbtcust_tab.so
	@echo BTCUST_TAB_SRC_DIRS:
	@echo $(SRC_DIRS) | fmt -1
	@echo BTCUST_TAB_OBJS:
	@echo $(OBJS) | fmt -1

gen_dirs:
	$(shell mkdir -p $(OBJ_PATH))

-include $(OBJS:.o=.d)

$(OBJ_PATH)/%.o: %.c
	$(CC) $(CFLAGS) $(INCS) -fPIC -c -o $@ $<
	$(CC) $(INCS) -MM $< > $(OBJ_PATH)/$*.d
	@mv -f $(OBJ_PATH)/$*.d $(OBJ_PATH)/$*.d.tmp
	@sed -e 's|.*:|$@:|' < $(OBJ_PATH)/$*.d.tmp > $(OBJ_PATH)/$*.d
	@sed -e 's/.*://' -e 's/\\$$//'< $(OBJ_PATH)/$*.d.tmp | fmt -1 | \
	 sed -e 's/^ *\([^ \n\r\t]\+\)/\1:/' >> $(OBJ_PATH)/$*.d
	@rm -f $(OBJ_PATH)/$*.d.tmp

$(BIN_PATH)/libbtcust_tab.so: $(OBJS)
	$(CC) -shared -o $@ $(OBJS) $(LDFLAGS) 

romfs: 
	$(ROMFSINST) $(BIN_PATH)/libbtcust_tab.so /lib/libbtcust_tab.so

clean:
	rm -f $(OBJ_PATH)/*
	rm -f $(BIN_PATH)/libbtcust_tab.so
	@echo btcust_tab cleaned.
