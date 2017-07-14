#TARGTE := wifi_setting.cgi
#TARGTE := sys_setting.cgi
#TARGTE := client_info.cgi
TARGTE := test
MYPATH := /opt/buildroot-gcc463/usr/bin/

CROSS := $(MYPATH)mipsel-linux-
CC := $(CROSS)gcc

all: cmcc_manage.o utils.o crc32.o flash_api.o nvram_env.o 
	$(CC) $^ -o $(TARGTE)

#all: test.o utils.o crc32.o flash_api.o nvram_env.o 
#	$(CC) $^ -o $(TARGTE)
clean: 
	rm cmcc_manage.o test.o utils.o $(TARGTE)
