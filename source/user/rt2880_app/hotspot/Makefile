LDFLAGS += -lpthread
HS_EXEC = hs
HSCTRL_EXEC = hsctrl

#PLATFORM = x86
PLATFORM = mipsel

ifeq ($(PLATFORM),x86)
CROSS_COMPILE=""
endif

ifeq ($(PLATFORM),mipsel)
CROSS_COMPILE = mipsel-linux-
endif

CFLAGS = -O2 -Wall -g
CC = $(CROSS_COMPILE)gcc

HS_OBJS = hotspot.o driver_wext.o netlink.o eloop.o os_internal.o main.o util.o ctrl_iface_unix.o debug.o

HSCTRL_OBJS = hotspot_ctrl/hotspot_cli.o hotspot_ctrl/hotspot_ctrl.o os_internal.o debug.o

all: $(HS_EXEC) $(HSCTRL_EXEC)

$(HS_EXEC): $(HS_OBJS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $(HS_EXEC) $(HS_OBJS)

$(HSCTRL_EXEC): $(HSCTRL_OBJS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $(HSCTRL_EXEC) $(HSCTRL_OBJS)

romfs_dir = $(ROOTDIR)/romfs

romfs_dirs = etc_ro/wifi
	
romfs:
	[ -d $(romfs_dir)/$$i ] || mkdir -p $(romfs_dir)
	for i in $(romfs_dirs); do \
		[ -d $(romfs_dir)/$$i ] || mkdir -p $(romfs_dir)/$$i; \
	done 
	$(ROMFSINST) /bin/$(HS_EXEC)
	$(ROMFSINST) /bin/$(HSCTRL_EXEC)
	$(ROMFSINST) /sbin/hotspot.sh
	$(ROMFSINST) /etc_ro/hotspot_ap.conf
	$(ROMFSINST) /etc_ro/hotspot_ap_ra0_default.conf
	$(ROMFSINST) /etc_ro/hotspot_ap_ra0.conf
	$(ROMFSINST) /etc_ro/hotspot_ap_ra1.conf
	$(ROMFSINST) /etc_ro/hotspot_ap2.conf
	$(ROMFSINST) /etc_ro/hotspot_ap_rai0_default.conf
	$(ROMFSINST) /etc_ro/hotspot_ap_rai0.conf
	$(ROMFSINST) /etc_ro/hotspot_ap_rai1.conf
#DBDC
	$(ROMFSINST) /etc_ro/hotspot_ap.conf_dbdc
	$(ROMFSINST) /etc_ro/hotspot_ap_ra2.conf
	$(ROMFSINST) /etc_ro/hotspot_ap2.conf_dbdc
	$(ROMFSINST) /etc_ro/hotspot_ap_rai2.conf
	 
#PF4
#	$(ROMFSINST) /etc_ro/WiFi-alliancelogo_3D.png
#	$(ROMFSINST) /etc_ro/WiFi-alliancelogo_3D_default.png
#	$(ROMFSINST) /etc_ro/wifi-abgn-logo_270x73.png	
#	$(ROMFSINST) /etc_ro/icon_blue.png
#	$(ROMFSINST) /etc_ro/icon_green.png
#	$(ROMFSINST) /etc_ro/icon_orange.png
#	$(ROMFSINST) /etc_ro/icon_red.png
#PF5
	$(ROMFSINST) /etc_ro/wifi-abgn-logo_270x73.png  
	$(ROMFSINST) /etc_ro/icon_blue_eng_default.png
	$(ROMFSINST) /etc_ro/icon_blue_zxx_default.png
	$(ROMFSINST) /etc_ro/icon_blue_eng.png
	$(ROMFSINST) /etc_ro/icon_blue_zxx.png
	$(ROMFSINST) /etc_ro/icon_green_eng_default.png
	$(ROMFSINST) /etc_ro/icon_green_zxx_default.png
	$(ROMFSINST) /etc_ro/icon_green_eng.png
	$(ROMFSINST) /etc_ro/icon_green_zxx.png
	$(ROMFSINST) /etc_ro/icon_orange_eng_default.png
	$(ROMFSINST) /etc_ro/icon_orange_zxx_default.png
	$(ROMFSINST) /etc_ro/icon_orange_eng.png
	$(ROMFSINST) /etc_ro/icon_orange_zxx.png
	$(ROMFSINST) /etc_ro/icon_red_eng_default.png
	$(ROMFSINST) /etc_ro/icon_red_zxx_default.png
	$(ROMFSINST) /etc_ro/icon_red_eng.png
	$(ROMFSINST) /etc_ro/icon_red_zxx.png

clean:
	rm -f *.o hotspot_ctrl/*.o $(HS_EXEC) $(HSCTRL_EXEC)
