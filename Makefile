all: 
	c:\users\aurelien\.platformio\penv\Scripts\pio run -e S_03
	copy .pio\build\S_03\firmware.elf TinySensor.elf