PIO     ?= $(HOME)/.platformio/penv/bin/pio
TARGET  ?= S_03

all:
	$(PIO) run -e $(TARGET)
	cp .pio/build/$(TARGET)/firmware.elf TinySensor.elf
