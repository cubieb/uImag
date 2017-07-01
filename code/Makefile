#TARGTE := test
#TARGTE := wifi_setting.cgi
#TARGTE := sys_setting.cgi
TARGTE := client_info.cgi
#TARGTE := router_setting.cgi

CROSS := mipsel-linux-
CC := $(CROSS)gcc

all: test.o utils.o crc32.o flash_api.o nvram_env.o
	$(CC) $^ -o $(TARGTE)

all: client_info.o utils.o crc32.o flash_api.o nvram_env.o
	$(CC) $^ -o $(TARGTE)

#all: sys_setting.o utils.o crc32.o flash_api.o nvram_env.o
#	$(CC) $^ -o $(TARGTE)

# all: wifi_setting.o utils.o crc32.o flash_api.o nvram_env.o 
#	$(CC) $^ -o $(TARGTE)

#all: router_setting.o utils.o crc32.o flash_api.o nvram_env.o 
#	$(CC) $^ -o $(TARGTE)
clean: 
	rm client_info.o sys_setting.o utils.o wifi_setting.o $(TARGTE)
