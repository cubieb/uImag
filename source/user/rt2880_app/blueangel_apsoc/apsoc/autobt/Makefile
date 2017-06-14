
# Makefile for autobt #

CUR_PATH := $(shell pwd)

BIN_PATH := $(CUR_PATH)/bin

CFLAGS = \
	-g \
	-Wall \
	-fshort-wchar \
	-D__LINUX_SUPPRESS_ERROR__ \
	-DMTK_MT6625 \
	$(NULL)

LDFLAGS = \
	-Wl,--no-fatal-warnings

SRCS := \
	$(CUR_PATH)/autobt.c \
	$(CUR_PATH)/bt_hw_test.c \
	$(CUR_PATH)/../btdrv/combo/mtk.c \
	$(CUR_PATH)/../btdrv/combo/radiomod.c \
	$(CUR_PATH)/../btdrv/combo/radiomgr.c \
	$(CUR_PATH)/../btdrv/combo/bt_nvram.c \
	$(NULL)

INCS := \
	-I$(BA_PATH)/btstd/common/inc \
	-I$(BA_PATH)/btstd/linux/inc \
	-I$(CUR_PATH)/../btdrv/inc \
	-I$(ROOTDIR)/lib/libnvram \
	-I$(ROOTDIR)/lib/include \
	-I$(LINUX_PATH)/include \
	$(NULL)

LIBS := \
	-L$(BIN_PATH) \
	-lbtstd \
	-ldl \
	-L$(ROOTDIR)/lib/lib \
	-lnvram \
	-lpthread \
	$(NULL)


##### Fixed Parts - Start #####

SRC_DIRS := $(sort $(dir $(SRCS)))

OBJ_PATH := $(CUR_PATH)/obj

OBJS := $(addprefix $(OBJ_PATH)/, $(notdir $(SRCS:.c=.o)))

RES_PATH := $(CUR_PATH)/res

VPATH = $(SRC_DIRS)

.PHONY: all clean gen_dirs romfs

all: gen_dirs $(OBJS) $(BIN_PATH)/autobt
	@echo AUTOBT_SRC_DIRS:
	@echo $(SRC_DIRS) | fmt -1
	@echo AUTOBT_OBJS:
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

$(BIN_PATH)/autobt: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(LDFLAGS) 

romfs: 
	$(ROMFSINST) $(BIN_PATH)/autobt /sbin/autobt

clean:
	rm -f $(OBJ_PATH)/*
	rm -f $(BIN_PATH)/autobt
	@echo autobt cleaned.

##### Fixed Parts - End #####
