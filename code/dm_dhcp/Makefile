EXE = dm_dhcp
CROSS := mipsel-linux-
CC := $(CROSS)gcc

#all: client_info.o utils.o crc32.o flash_api.o nvram_env.o
#	$(CC) $^ -o $(TARGTE)

#all: sys_setting.o utils.o crc32.o flash_api.o nvram_env.o
#	$(CC) $^ -o $(TARGTE)

#dall: wifi_setting.o utils.o crc32.o flash_api.o nvram_env.o 
#	$(CC) $^ -o $(TARGTE)

#all: sta.o utils.o crc32.o flash_api.o nvram_env.o 
#	$(CC) $^ -o $(TARGTE)
#clean: 
#	rm client_info.o sys_setting.o utils.o wifi_setting.o $(TARGTE)

all: dm_dhcp.o  crc32.o flash_api.o nvram_env.o 
	$(CC) $^ -o $(EXE)
