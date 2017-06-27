TARGTE := Sys_Setting.cgi

CROSS := mipsel-linux-
CC := $(CROSS)gcc

# all: client_info.o mtk_operate.o  utils.o crc32.o flash_api.o nvram_env.o  adm_info.o
#	$(CC) $^ -o $(TARGTE)

all: sys_setting.o utils.o mtk_operate.o crc32.o flash_api.o nvram_env.o
	$(CC) $^ -o $(TARGTE)

clean: 
	rm client_info.o adm_info.o mtk_operate.o  sys_setting.o utils.o $(TARGTE)
