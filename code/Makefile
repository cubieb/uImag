TARGTE := sys_setting.cgi

CROSS := mipsel-linux-
CC := $(CROSS)gcc

# all: client_info.o utils.o crc32.o flash_api.o nvram_env.o  adm_info.o
#	$(CC) $^ -o $(TARGTE)

all: sys_setting.o utils.o crc32.o flash_api.o nvram_env.o
	$(CC) $^ -o $(TARGTE)

clean: 
	rm client_info.o sys_setting.o utils.o $(TARGTE)
