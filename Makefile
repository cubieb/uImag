TARGTE := stacountinfo 

CROSS := mipsel-linux-
CC := $(CROSS)gcc

#all: wifi_setting.o mtk_operate.o  utils.o crc32.o flash_api.o nvram_env.o 
#	$(CC) $^ -o $(TARGTE)

#all: stacountinfo.o mtk_operate.o  utils.o crc32.o flash_api.o nvram_env.o 
#	$(CC) $^ -o $(TARGTE)

#all: mytest.o mtk_operate.o  utils.o crc32.o flash_api.o nvram_env.o 
#   $(CC) $^ -o $(TARGTE)

#all: test.o  utils.o 
#	$(CC) $^ -o $(TARGTE)

all: mt_demo.o 
	$(CC) $^ -o $(TARGTE)
clean: 
	rm test.o wifi_setting.o mtk_operate.o  utils.o $(TARGTE)
