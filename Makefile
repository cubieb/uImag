#CROSS := mipsel-linux-
#CC := $(CROSS)gcc

#CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/include

sta: mt_demo.o 
	$(CC)  $(CFLAGS) $^ -o $@

clean: 
	rm *.o sta
