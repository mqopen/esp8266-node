CROSS := xtensa-lx106-elf-
CC := $(CROSS)gcc
LD := $(CROSS)gcc
CFLAGS = -Wall -Wno-implicit-function-declaration -I. -mlongcalls -Os -DICACHE_FLASH
LDLIBS = -nostdlib -Wl,--start-group -lmain -lnet80211 -lwpa -llwip -lpp -lphy -Wl,--end-group -lgcc
LDFLAGS = -Teagle.app.v6.ld -Os -Wl,-static

CSRC = main.c uart.c gpio16.c network.c node.c umqtt.c mqttclient.c i2c_master.c bmp180.c actsig.c
OBJS = $(subst .c,.o,$(CSRC))

all: esp8266-firmware.elf

esp8266-firmware-0x00000.bin: esp8266-firmware.elf
	esptool.py elf2image $^

esp8266-firmware.elf: $(OBJS)
	$(LD) $(LDLIBS) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

flash: esp8266-firmware-0x00000.bin
	esptool.py  write_flash 0 esp8266-firmware.elf-0x00000.bin 0x40000 esp8266-firmware.elf-0x40000.bin

clean:
	rm -f esp8266-firmware.elf *.d *.o *.bin

# Generate dependecy tree.
-include $(subst .c,.d,$(CSRC))
define create-dep
	$(CC) -M $(CFLAGS) $(INCLUDE_FLAGS) $(DEFINE_FLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(BUILD_DIR)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
endef

%.d: %.c
	$(create-dep)

.PHONY: clean flash all
