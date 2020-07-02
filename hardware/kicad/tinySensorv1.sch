EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "TinySensor board"
Date "2020-06-28"
Rev "2.1"
Comp ""
Comment1 "Open Hardware"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Device:R R2
U 1 1 5BCB85EA
P 4900 1400
F 0 "R2" H 4970 1446 50  0000 L CNN
F 1 "4,7k" H 4970 1355 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal" V 4830 1400 50  0001 C CNN
F 3 "~" H 4900 1400 50  0001 C CNN
	1    4900 1400
	1    0    0    -1  
$EndComp
$Comp
L Device:R R3
U 1 1 5BCB8747
P 5200 1400
F 0 "R3" H 5270 1446 50  0000 L CNN
F 1 "4,7k" H 5270 1355 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal" V 5130 1400 50  0001 C CNN
F 3 "~" H 5200 1400 50  0001 C CNN
	1    5200 1400
	1    0    0    -1  
$EndComp
$Comp
L tinySensorv1-rescue:ATtiny84A-PU-MCU_Microchip_ATtiny-tinySensorv1-rescue U1
U 1 1 5BCB880C
P 2250 2000
F 0 "U1" H 1720 2046 50  0000 R CNN
F 1 "ATtiny84A-PU" H 1720 1955 50  0000 R CNN
F 2 "Package_DIP:DIP-14_W7.62mm" H 2250 2000 50  0001 C CIN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/doc8183.pdf" H 2250 2000 50  0001 C CNN
	1    2250 2000
	1    0    0    -1  
$EndComp
$Comp
L Device:R R1
U 1 1 5BCB91A0
P 2400 6350
F 0 "R1" V 2193 6350 50  0000 C CNN
F 1 "1k" V 2284 6350 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal" V 2330 6350 50  0001 C CNN
F 3 "~" H 2400 6350 50  0001 C CNN
	1    2400 6350
	0    1    1    0   
$EndComp
$Comp
L Device:LED D1
U 1 1 5BCB9383
P 2800 6350
F 0 "D1" H 2792 6095 50  0000 C CNN
F 1 "LED" H 2792 6186 50  0000 C CNN
F 2 "LED_THT:LED_D1.8mm_W3.3mm_H2.4mm" H 2800 6350 50  0001 C CNN
F 3 "~" H 2800 6350 50  0001 C CNN
	1    2800 6350
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0102
U 1 1 5BCB96FB
P 2250 3100
F 0 "#PWR0102" H 2250 2850 50  0001 C CNN
F 1 "GND" H 2255 2927 50  0000 C CNN
F 2 "" H 2250 3100 50  0001 C CNN
F 3 "" H 2250 3100 50  0001 C CNN
	1    2250 3100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0104
U 1 1 5BCB9757
P 3050 6350
F 0 "#PWR0104" H 3050 6100 50  0001 C CNN
F 1 "GND" V 3055 6222 50  0000 R CNN
F 2 "" H 3050 6350 50  0001 C CNN
F 3 "" H 3050 6350 50  0001 C CNN
	1    3050 6350
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3050 6350 2950 6350
Wire Wire Line
	2650 6350 2550 6350
Wire Wire Line
	2250 2900 2250 3100
Wire Wire Line
	2250 1050 2250 1100
Wire Wire Line
	4900 1250 4900 1100
Wire Wire Line
	5200 1250 5200 1100
$Comp
L Connector:Conn_01x03_Male J1
U 1 1 5BCBC5C9
P 8600 3550
F 0 "J1" V 8753 3362 50  0000 R CNN
F 1 "RADIO" V 8662 3362 50  0000 R CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 8600 3550 50  0001 C CNN
F 3 "~" H 8600 3550 50  0001 C CNN
	1    8600 3550
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR0109
U 1 1 5BCBDFF2
P 8500 3300
F 0 "#PWR0109" H 8500 3050 50  0001 C CNN
F 1 "GND" H 8505 3127 50  0000 C CNN
F 2 "" H 8500 3300 50  0001 C CNN
F 3 "" H 8500 3300 50  0001 C CNN
	1    8500 3300
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR0110
U 1 1 5BCC1563
P 6300 1200
F 0 "#PWR0110" H 6300 950 50  0001 C CNN
F 1 "GND" V 6305 1072 50  0000 R CNN
F 2 "" H 6300 1200 50  0001 C CNN
F 3 "" H 6300 1200 50  0001 C CNN
	1    6300 1200
	0    1    1    0   
$EndComp
$Comp
L power:+BATT #PWR0111
U 1 1 5BCC15A5
P 6600 1100
F 0 "#PWR0111" H 6600 950 50  0001 C CNN
F 1 "+BATT" V 6615 1227 50  0000 L CNN
F 2 "" H 6600 1100 50  0001 C CNN
F 3 "" H 6600 1100 50  0001 C CNN
	1    6600 1100
	0    -1   -1   0   
$EndComp
Wire Wire Line
	6600 1100 6700 1100
Text Label 3000 1800 0    50   ~ 0
SCL
Text Label 3000 2000 0    50   ~ 0
SDA
Wire Wire Line
	3000 1800 2850 1800
Text Label 3000 2300 0    50   ~ 0
TX_RADIO
Text Label 8700 3150 0    50   ~ 0
TX_RADIO
Text Label 3000 2400 0    50   ~ 0
LED_OUT
Wire Wire Line
	2850 2400 3000 2400
Text Label 4900 1700 0    50   ~ 0
SDA
Text Label 5200 1700 0    50   ~ 0
SCL
Wire Wire Line
	5200 1550 5200 1700
Wire Wire Line
	4900 1550 4900 1700
Text Label 1200 6300 0    50   ~ 0
LED_OUT
$Comp
L power:PWR_FLAG #FLG0101
U 1 1 5BCD7ADF
P 9750 5650
F 0 "#FLG0101" H 9750 5725 50  0001 C CNN
F 1 "PWR_FLAG" H 9750 5824 50  0000 C CNN
F 2 "" H 9750 5650 50  0001 C CNN
F 3 "~" H 9750 5650 50  0001 C CNN
	1    9750 5650
	1    0    0    -1  
$EndComp
$Comp
L power:PWR_FLAG #FLG0102
U 1 1 5BCD7B19
P 10150 5650
F 0 "#FLG0102" H 10150 5725 50  0001 C CNN
F 1 "PWR_FLAG" H 10150 5824 50  0000 C CNN
F 2 "" H 10150 5650 50  0001 C CNN
F 3 "~" H 10150 5650 50  0001 C CNN
	1    10150 5650
	1    0    0    -1  
$EndComp
$Comp
L power:PWR_FLAG #FLG0103
U 1 1 5BCD7B4B
P 9350 5650
F 0 "#FLG0103" H 9350 5725 50  0001 C CNN
F 1 "PWR_FLAG" H 9350 5824 50  0000 C CNN
F 2 "" H 9350 5650 50  0001 C CNN
F 3 "~" H 9350 5650 50  0001 C CNN
	1    9350 5650
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0113
U 1 1 5BCD7B84
P 9350 5750
F 0 "#PWR0113" H 9350 5500 50  0001 C CNN
F 1 "GND" H 9355 5577 50  0000 C CNN
F 2 "" H 9350 5750 50  0001 C CNN
F 3 "" H 9350 5750 50  0001 C CNN
	1    9350 5750
	1    0    0    -1  
$EndComp
Wire Wire Line
	10150 5750 10150 5650
Wire Wire Line
	9750 5750 9750 5650
Wire Wire Line
	9350 5750 9350 5650
$Comp
L power:+BATT #PWR0115
U 1 1 5BCDA299
P 10150 5750
F 0 "#PWR0115" H 10150 5600 50  0001 C CNN
F 1 "+BATT" H 10165 5923 50  0000 C CNN
F 2 "" H 10150 5750 50  0001 C CNN
F 3 "" H 10150 5750 50  0001 C CNN
	1    10150 5750
	-1   0    0    1   
$EndComp
Wire Wire Line
	6300 1200 6650 1200
$Comp
L Device:CP C1
U 1 1 5BCDDA39
P 6500 1550
F 0 "C1" V 6550 1450 50  0000 C CNN
F 1 "10u" V 6500 1250 50  0000 C CNN
F 2 "Capacitor_THT:CP_Radial_D5.0mm_P2.50mm" H 6538 1400 50  0001 C CNN
F 3 "~" H 6500 1550 50  0001 C CNN
	1    6500 1550
	0    -1   -1   0   
$EndComp
Connection ~ 6650 1200
Wire Wire Line
	6650 1200 6700 1200
Connection ~ 6350 1300
Wire Wire Line
	6350 1300 6700 1300
$Comp
L Device:C C5
U 1 1 5BCDF3E0
P 6500 2150
F 0 "C5" V 6450 2250 50  0000 C CNN
F 1 "1n" V 6500 2400 50  0000 C CNN
F 2 "Capacitor_THT:CP_Radial_Tantal_D4.5mm_P2.50mm" H 6538 2000 50  0001 C CNN
F 3 "~" H 6500 2150 50  0001 C CNN
	1    6500 2150
	0    -1   -1   0   
$EndComp
Text Notes 10200 2300 0    50   ~ 0
Power management
Text Notes 4850 2300 0    50   ~ 0
I2C bus pullups
Wire Notes Line
	5500 850  4650 850 
$Comp
L Connector:Screw_Terminal_01x02 J12
U 1 1 5BCE863C
P 10300 1350
F 0 "J12" V 10250 1450 50  0000 L CNN
F 1 "AA Socket" V 10450 1200 50  0000 L CNN
F 2 "Battery:BatteryHolder_MPD_BH-18650-PC2" H 10300 1350 50  0001 C CNN
F 3 "~" H 10300 1350 50  0001 C CNN
	1    10300 1350
	0    -1   -1   0   
$EndComp
$Comp
L Device:Jumper JP1
U 1 1 5BCE88B9
P 1850 6350
F 0 "JP1" H 1850 6614 50  0000 C CNN
F 1 "LED Jumper" H 1850 6523 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical" H 1850 6350 50  0001 C CNN
F 3 "~" H 1850 6350 50  0001 C CNN
	1    1850 6350
	1    0    0    -1  
$EndComp
Wire Wire Line
	2150 6350 2250 6350
Wire Wire Line
	1200 6300 1200 6350
Wire Wire Line
	1200 6350 1550 6350
$Comp
L power:+BATT #PWR0116
U 1 1 5BCEAF71
P 8000 1350
F 0 "#PWR0116" H 8000 1200 50  0001 C CNN
F 1 "+BATT" H 8000 1500 50  0000 C CNN
F 2 "" H 8000 1350 50  0001 C CNN
F 3 "" H 8000 1350 50  0001 C CNN
	1    8000 1350
	1    0    0    -1  
$EndComp
Wire Notes Line
	4650 2350 5500 2350
Wire Notes Line
	4650 850  4650 2350
Wire Notes Line
	5500 850  5500 2350
Wire Notes Line
	9300 2900 9300 4300
Wire Notes Line
	9350 4250 8100 4250
Wire Notes Line
	8100 4250 8100 2850
Wire Notes Line
	8050 2900 9300 2900
Text Notes 8200 4200 0    50   ~ 0
433Mhz emitter connection
Text Label 3000 1700 0    50   ~ 0
OneWire
Wire Wire Line
	2850 1700 3000 1700
Text Label 3000 1900 0    50   ~ 0
MISO
Wire Wire Line
	2850 1900 3000 1900
Text Label 3000 2600 0    50   ~ 0
RESET
Wire Wire Line
	2850 2600 3000 2600
$Comp
L tinySensorv1-rescue:AVR-ISP-6-Connector J6
U 1 1 5BCF975A
P 2250 4300
F 0 "J6" H 1970 4396 50  0000 R CNN
F 1 "ISP" H 1970 4305 50  0000 R CNN
F 2 "TinySensor lib:ICSP_SOCKET_MINI_2x3" V 2000 4350 50  0001 C CNN
F 3 " ~" H 975 3750 50  0001 C CNN
	1    2250 4300
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0119
U 1 1 5BCF99E9
P 2150 4750
F 0 "#PWR0119" H 2150 4500 50  0001 C CNN
F 1 "GND" H 2155 4577 50  0000 C CNN
F 2 "" H 2150 4750 50  0001 C CNN
F 3 "" H 2150 4750 50  0001 C CNN
	1    2150 4750
	1    0    0    -1  
$EndComp
Text Label 2800 4100 0    50   ~ 0
MISO
Text Label 2800 4200 0    50   ~ 0
SDA
Text Label 2800 4300 0    50   ~ 0
SCL
Text Label 2800 4400 0    50   ~ 0
RESET
Wire Wire Line
	2650 4100 2800 4100
Wire Wire Line
	2650 4200 2800 4200
Wire Wire Line
	2650 4300 2800 4300
Wire Wire Line
	2650 4400 2800 4400
Wire Wire Line
	2150 4700 2150 4750
Connection ~ 2150 4750
Wire Wire Line
	2150 4750 2150 4900
Wire Wire Line
	2150 3800 2150 3750
Wire Wire Line
	2850 2000 3000 2000
Text Notes 3150 1800 0    50   ~ 0
(and SCK)
Text Notes 3150 2000 0    50   ~ 0
(and MOSI)
Text Label 3000 1500 0    50   ~ 0
Battery_Checker
Wire Wire Line
	2850 1500 3000 1500
Text Label 9450 850  2    50   ~ 0
Battery_Checker
Wire Wire Line
	8500 3300 8500 3350
Wire Wire Line
	8700 3350 8700 3150
Text Label 3000 1400 0    50   ~ 0
Analog_sensor
Wire Wire Line
	2850 1400 3000 1400
$Comp
L Connector_Generic:Conn_01x03 J7
U 1 1 5BD28967
P 4500 4550
F 0 "J7" H 4650 4550 50  0000 C CNN
F 1 "Analog sensor" H 4500 4800 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 4500 4550 50  0001 C CNN
F 3 "~" H 4500 4550 50  0001 C CNN
	1    4500 4550
	-1   0    0    1   
$EndComp
Text Label 5050 4550 0    50   ~ 0
Analog_sensor
Wire Notes Line
	4050 3250 7750 3250
Text Notes 6900 6050 0    50   ~ 0
Sensors connections
$Comp
L Connector:Screw_Terminal_01x02 J11
U 1 1 5BD45003
P 8450 1350
F 0 "J11" V 8400 1450 50  0000 L CNN
F 1 "AAA socket" V 8600 1100 50  0000 L CNN
F 2 "Battery:BatteryHolder_Keystone_2466_1xAAA" H 8450 1350 50  0001 C CNN
F 3 "~" H 8450 1350 50  0001 C CNN
	1    8450 1350
	0    -1   -1   0   
$EndComp
$Comp
L Connector:Screw_Terminal_01x02 J10
U 1 1 5BD4648E
P 9050 1350
F 0 "J10" V 9000 1450 50  0000 L CNN
F 1 "CR2032 socket" V 9200 1000 50  0000 L CNN
F 2 "battery supp.:BatteryHolder_Keystone_106_1x21mm" H 9050 1350 50  0001 C CNN
F 3 "~" H 9050 1350 50  0001 C CNN
	1    9050 1350
	0    -1   -1   0   
$EndComp
$Comp
L Connector:Screw_Terminal_01x02 J13
U 1 1 5BD4A406
P 9750 1350
F 0 "J13" V 9700 1450 50  0000 L CNN
F 1 "RAW IN" V 9900 1200 50  0000 L CNN
F 2 "TerminalBlock:TerminalBlock_bornier-2_P5.08mm" H 9750 1350 50  0001 C CNN
F 3 "~" H 9750 1350 50  0001 C CNN
	1    9750 1350
	0    -1   -1   0   
$EndComp
$Comp
L Connector_Generic:Conn_01x03 J2
U 1 1 5BCCF337
P 6900 1200
F 0 "J2" H 6979 1242 50  0000 L CNN
F 1 "DCDC converter" H 6979 1151 50  0000 L CNN
F 2 "TinySensor lib:DC-DC_0.9v-3.3v_to_3.3v" H 6900 1200 50  0001 C CNN
F 3 "~" H 6900 1200 50  0001 C CNN
	1    6900 1200
	1    0    0    -1  
$EndComp
Wire Wire Line
	2850 2300 3000 2300
Text Label 3000 2100 0    50   ~ 0
PWM_OUT
$Comp
L power:GND #PWR0120
U 1 1 5BDB0112
P 4900 5600
F 0 "#PWR0120" H 4900 5350 50  0001 C CNN
F 1 "GND" H 5000 5450 50  0000 R CNN
F 2 "" H 4900 5600 50  0001 C CNN
F 3 "" H 4900 5600 50  0001 C CNN
	1    4900 5600
	1    0    0    -1  
$EndComp
$Comp
L Device:R R4
U 1 1 5BDB015F
P 4900 5100
F 0 "R4" H 4970 5146 50  0000 L CNN
F 1 "75R" H 4970 5055 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal" V 4830 5100 50  0001 C CNN
F 3 "~" H 4900 5100 50  0001 C CNN
	1    4900 5100
	1    0    0    -1  
$EndComp
$Comp
L Device:CP1 C3
U 1 1 5BDB032D
P 4900 5400
F 0 "C3" H 5015 5446 50  0000 L CNN
F 1 "1u" H 5015 5355 50  0000 L CNN
F 2 "Capacitor_THT:CP_Radial_D4.0mm_P1.50mm" H 4900 5400 50  0001 C CNN
F 3 "~" H 4900 5400 50  0001 C CNN
	1    4900 5400
	1    0    0    -1  
$EndComp
Wire Notes Line
	5850 2400 11000 2400
Wire Notes Line
	5850 600  11000 600 
Wire Notes Line
	5850 600  5850 2350
Wire Notes Line
	11000 600  11000 2350
$Comp
L Jumper:SolderJumper_2_Open JP2
U 1 1 5BE77668
P 8250 1800
F 0 "JP2" V 8250 1900 50  0000 L CNN
F 1 "DIRECT Vcc" V 8150 1300 50  0000 L CNN
F 2 "Jumper:SolderJumper-2_P1.3mm_Open_TrianglePad1.0x1.5mm" H 8250 1800 50  0001 C CNN
F 3 "~" H 8250 1800 50  0001 C CNN
	1    8250 1800
	0    -1   -1   0   
$EndComp
$Comp
L Mechanical:MountingHole MH1
U 1 1 5BE894C2
P 9150 5150
F 0 "MH1" H 9250 5196 50  0000 L CNN
F 1 "MountingHole" H 9250 5105 50  0000 L CNN
F 2 "MountingHole:MountingHole_2.5mm" H 9150 5150 50  0001 C CNN
F 3 "~" H 9150 5150 50  0001 C CNN
	1    9150 5150
	0    -1   -1   0   
$EndComp
$Comp
L Mechanical:MountingHole MH2
U 1 1 5BE89558
P 9350 5150
F 0 "MH2" H 9450 5196 50  0000 L CNN
F 1 "MountingHole" H 9450 5105 50  0000 L CNN
F 2 "MountingHole:MountingHole_2.5mm" H 9350 5150 50  0001 C CNN
F 3 "~" H 9350 5150 50  0001 C CNN
	1    9350 5150
	0    -1   -1   0   
$EndComp
$Comp
L Mechanical:MountingHole MH3
U 1 1 5BE8964E
P 9550 5150
F 0 "MH3" H 9650 5196 50  0000 L CNN
F 1 "MountingHole" H 9650 5105 50  0000 L CNN
F 2 "MountingHole:MountingHole_2.5mm" H 9550 5150 50  0001 C CNN
F 3 "~" H 9550 5150 50  0001 C CNN
	1    9550 5150
	0    -1   -1   0   
$EndComp
$Comp
L Mechanical:MountingHole MH4
U 1 1 5BE896FE
P 9750 5150
F 0 "MH4" H 9850 5196 50  0000 L CNN
F 1 "MountingHole" H 9850 5105 50  0000 L CNN
F 2 "MountingHole:MountingHole_2.5mm" H 9750 5150 50  0001 C CNN
F 3 "~" H 9750 5150 50  0001 C CNN
	1    9750 5150
	0    -1   -1   0   
$EndComp
$Comp
L Mechanical:MountingHole MH5
U 1 1 5BE897A6
P 9950 5150
F 0 "MH5" H 10050 5196 50  0000 L CNN
F 1 "MountingHole" H 10050 5105 50  0000 L CNN
F 2 "MountingHole:MountingHole_2.5mm" H 9950 5150 50  0001 C CNN
F 3 "~" H 9950 5150 50  0001 C CNN
	1    9950 5150
	0    -1   -1   0   
$EndComp
Wire Notes Line
	3600 5950 3600 6700
Wire Notes Line
	3600 6700 1000 6700
Wire Notes Line
	1000 6700 1000 5950
Wire Notes Line
	1000 5950 3600 5950
Text Notes 2850 6650 0    50   ~ 0
Radio activity led
Wire Notes Line rgb(0, 194, 0)
	4450 4950 5800 4950
Text Notes 4500 5950 0    50   ~ 0
Optional RC filter for LM35 sensor \nor resistor bridge for LDR
Wire Notes Line rgb(114, 194, 0)
	5950 4950 5950 5950
Wire Notes Line rgb(0, 194, 0)
	4450 4950 4450 5950
Wire Notes Line
	4050 6200 7750 6200
Wire Notes Line
	7750 3250 7750 6200
Wire Notes Line
	4050 3250 4050 6200
$Comp
L power:GND #PWR0123
U 1 1 5BE989C1
P 9150 3850
F 0 "#PWR0123" H 9150 3600 50  0001 C CNN
F 1 "GND" H 9155 3677 50  0000 C CNN
F 2 "" H 9150 3850 50  0001 C CNN
F 3 "" H 9150 3850 50  0001 C CNN
	1    9150 3850
	1    0    0    -1  
$EndComp
Wire Wire Line
	8600 3850 8700 3850
$Comp
L Mechanical:MountingHole MH6
U 1 1 5BEA80B3
P 10150 5150
F 0 "MH6" H 10250 5196 50  0000 L CNN
F 1 "MountingHole" H 10250 5105 50  0000 L CNN
F 2 "MountingHole:MountingHole_2.5mm" H 10150 5150 50  0001 C CNN
F 3 "~" H 10150 5150 50  0001 C CNN
	1    10150 5150
	0    -1   -1   0   
$EndComp
$Comp
L Device:C C2
U 1 1 5BEB17B5
P 6500 1850
F 0 "C2" V 6450 1950 50  0000 C CNN
F 1 "100n" V 6500 2150 50  0000 C CNN
F 2 "Capacitor_THT:C_Rect_L7.0mm_W2.0mm_P5.00mm" H 6538 1700 50  0001 C CNN
F 3 "~" H 6500 1850 50  0001 C CNN
	1    6500 1850
	0    1    1    0   
$EndComp
Connection ~ 6350 1550
Connection ~ 6650 1550
Wire Wire Line
	6650 1200 6650 1550
Wire Wire Line
	6350 1300 6350 1550
Connection ~ 6650 1850
Wire Wire Line
	6650 1850 6650 2150
Connection ~ 6350 1850
Wire Wire Line
	6350 1850 6350 2150
Wire Wire Line
	6350 1550 6350 1850
Wire Wire Line
	6650 1550 6650 1850
Text Label 3000 1600 0    50   ~ 0
SENSOR_VCC
Wire Wire Line
	2850 1600 3000 1600
$Comp
L power:VCC #PWR02
U 1 1 5BEA0DC7
P 2250 1050
F 0 "#PWR02" H 2250 900 50  0001 C CNN
F 1 "VCC" H 2267 1223 50  0000 C CNN
F 2 "" H 2250 1050 50  0001 C CNN
F 3 "" H 2250 1050 50  0001 C CNN
	1    2250 1050
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR01
U 1 1 5BEA1087
P 2150 3750
F 0 "#PWR01" H 2150 3600 50  0001 C CNN
F 1 "VCC" H 2167 3923 50  0000 C CNN
F 2 "" H 2150 3750 50  0001 C CNN
F 3 "" H 2150 3750 50  0001 C CNN
	1    2150 3750
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR05
U 1 1 5BEA3FC9
P 8600 3850
F 0 "#PWR05" H 8600 3700 50  0001 C CNN
F 1 "VCC" H 8617 4023 50  0000 C CNN
F 2 "" H 8600 3850 50  0001 C CNN
F 3 "" H 8600 3850 50  0001 C CNN
	1    8600 3850
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR04
U 1 1 5BEA4029
P 8600 3100
F 0 "#PWR04" H 8600 2950 50  0001 C CNN
F 1 "VCC" H 8617 3273 50  0000 C CNN
F 2 "" H 8600 3100 50  0001 C CNN
F 3 "" H 8600 3100 50  0001 C CNN
	1    8600 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	8600 3100 8600 3350
$Comp
L power:VCC #PWR06
U 1 1 5BEA6C82
P 7850 1950
F 0 "#PWR06" H 7850 1800 50  0001 C CNN
F 1 "VCC" H 7867 2123 50  0000 C CNN
F 2 "" H 7850 1950 50  0001 C CNN
F 3 "" H 7850 1950 50  0001 C CNN
	1    7850 1950
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR03
U 1 1 5BEA6E62
P 5950 1300
F 0 "#PWR03" H 5950 1150 50  0001 C CNN
F 1 "VCC" H 5967 1473 50  0000 C CNN
F 2 "" H 5950 1300 50  0001 C CNN
F 3 "" H 5950 1300 50  0001 C CNN
	1    5950 1300
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR07
U 1 1 5BEA9B11
P 9750 5750
F 0 "#PWR07" H 9750 5600 50  0001 C CNN
F 1 "VCC" H 9768 5923 50  0000 C CNN
F 2 "" H 9750 5750 50  0001 C CNN
F 3 "" H 9750 5750 50  0001 C CNN
	1    9750 5750
	-1   0    0    1   
$EndComp
Wire Wire Line
	5950 1300 6350 1300
$Comp
L power:GND #PWR0117
U 1 1 5BCEAFCA
P 10300 1850
F 0 "#PWR0117" H 10300 1600 50  0001 C CNN
F 1 "GND" H 10305 1677 50  0000 C CNN
F 2 "" H 10300 1850 50  0001 C CNN
F 3 "" H 10300 1850 50  0001 C CNN
	1    10300 1850
	1    0    0    -1  
$EndComp
Wire Wire Line
	10300 1750 10300 1850
Wire Wire Line
	8000 1650 8250 1650
Wire Wire Line
	8000 1650 8000 1350
Connection ~ 8250 1650
Wire Wire Line
	8250 1950 7850 1950
Wire Wire Line
	9450 850  9450 1650
Connection ~ 9450 1650
Text Notes 7700 2100 0    50   ~ 0
Bypass of DC-DC converter
Wire Wire Line
	4900 1100 5200 1100
Text Label 4900 1000 0    50   ~ 0
SENSOR_VCC
Wire Wire Line
	4900 1100 4900 1000
Connection ~ 4900 1100
Wire Wire Line
	4700 4550 4900 4550
Wire Wire Line
	4900 4950 4900 4550
Connection ~ 4900 4550
Wire Wire Line
	4900 4550 5050 4550
Wire Wire Line
	8250 1650 8450 1650
Wire Wire Line
	8550 1550 8550 1750
Wire Wire Line
	8450 1550 8450 1650
Connection ~ 8450 1650
Wire Wire Line
	8450 1650 9050 1650
Wire Wire Line
	8550 1750 9150 1750
Wire Wire Line
	9050 1550 9050 1650
Connection ~ 9050 1650
Wire Wire Line
	9050 1650 9450 1650
Wire Wire Line
	9150 1550 9150 1750
Connection ~ 9150 1750
$Comp
L Device:CP C4
U 1 1 5E0B7120
P 8850 3850
F 0 "C4" V 8950 3700 50  0000 C CNN
F 1 "10u" V 9014 3850 50  0000 C CNN
F 2 "Capacitor_THT:CP_Radial_D5.0mm_P2.50mm" H 8888 3700 50  0001 C CNN
F 3 "~" H 8850 3850 50  0001 C CNN
	1    8850 3850
	0    -1   -1   0   
$EndComp
Wire Wire Line
	9150 3850 9000 3850
$Comp
L power:GND #PWR0101
U 1 1 5E0E2B34
P 4150 2100
F 0 "#PWR0101" H 4150 1850 50  0001 C CNN
F 1 "GND" H 4155 1927 50  0000 C CNN
F 2 "" H 4150 2100 50  0001 C CNN
F 3 "" H 4150 2100 50  0001 C CNN
	1    4150 2100
	1    0    0    -1  
$EndComp
Wire Wire Line
	5450 5600 4900 5600
Wire Wire Line
	4900 5550 4900 5600
Connection ~ 4900 5600
Wire Notes Line rgb(7, 194, 0)
	5800 4950 5950 4950
Wire Wire Line
	4900 5250 5450 5250
Wire Wire Line
	5450 5250 5450 5300
Wire Notes Line rgb(0, 194, 0)
	4450 5950 5950 5950
Connection ~ 4900 5250
$Comp
L Connector_Generic:Conn_01x02 J8
U 1 1 5F04FEAF
P 2200 5450
F 0 "J8" H 2280 5442 50  0000 L CNN
F 1 "EXT. INT0" H 1800 5650 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical" H 2200 5450 50  0001 C CNN
F 3 "~" H 2200 5450 50  0001 C CNN
	1    2200 5450
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0103
U 1 1 5F052C10
P 2000 5600
F 0 "#PWR0103" H 2000 5350 50  0001 C CNN
F 1 "GND" H 2005 5427 50  0000 C CNN
F 2 "" H 2000 5600 50  0001 C CNN
F 3 "" H 2000 5600 50  0001 C CNN
	1    2000 5600
	1    0    0    -1  
$EndComp
Wire Wire Line
	2000 5550 2000 5600
Text Label 1700 5450 0    50   ~ 0
INT0
Wire Wire Line
	1700 5450 2000 5450
Wire Wire Line
	2850 2500 3000 2500
Text Label 3000 2500 0    50   ~ 0
INT0
Wire Wire Line
	9450 1650 9750 1650
Wire Wire Line
	10300 1550 10300 1650
Wire Wire Line
	9750 1550 9750 1650
Connection ~ 9750 1650
Wire Wire Line
	9750 1650 10300 1650
Wire Wire Line
	9150 1750 9850 1750
Connection ~ 10300 1750
Wire Wire Line
	10300 1750 10400 1750
Wire Wire Line
	10400 1550 10400 1750
Wire Wire Line
	9850 1550 9850 1750
Connection ~ 9850 1750
Wire Wire Line
	9850 1750 10300 1750
Wire Wire Line
	2850 2100 3900 2100
$Comp
L Connector:Conn_01x02_Male J4
U 1 1 5E0CE887
P 3900 2300
F 0 "J4" H 4006 2478 50  0000 C CNN
F 1 "PWM out" V 3800 2250 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical" H 3900 2300 50  0001 C CNN
F 3 "~" H 3900 2300 50  0001 C CNN
	1    3900 2300
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4150 2100 4000 2100
Wire Wire Line
	6000 4650 4700 4650
Text Label 6550 3950 0    50   ~ 0
OneWire
Text Label 6000 3400 0    50   ~ 0
SENSOR_VCC
Connection ~ 5750 3850
Wire Wire Line
	5750 3500 5750 3850
$Comp
L power:GND #PWR0105
U 1 1 5BCCA06F
P 5750 3500
F 0 "#PWR0105" H 5750 3250 50  0001 C CNN
F 1 "GND" H 5755 3327 50  0000 C CNN
F 2 "" H 5750 3500 50  0001 C CNN
F 3 "" H 5750 3500 50  0001 C CNN
	1    5750 3500
	-1   0    0    1   
$EndComp
$Comp
L Connector:Conn_01x06_Female J3
U 1 1 5BDADDD8
P 7600 4650
F 0 "J3" H 7627 4626 50  0000 L CNN
F 1 "I2C Sensor (BME/BMP 280)" H 6600 4150 50  0000 L CNN
F 2 "TinySensor lib:BMx280_breakout" H 7600 4650 50  0001 C CNN
F 3 "~" H 7600 4650 50  0001 C CNN
	1    7600 4650
	1    0    0    -1  
$EndComp
Text Notes 7150 4850 0    50   ~ 0
CSB
Text Notes 7150 4950 0    50   ~ 0
SDO
Wire Wire Line
	7150 4750 7400 4750
Wire Wire Line
	7150 4650 7400 4650
Wire Wire Line
	7150 4950 7400 4950
$Comp
L power:GND #PWR0106
U 1 1 5BCCC7EF
P 7150 4950
F 0 "#PWR0106" H 7150 4700 50  0001 C CNN
F 1 "GND" V 7155 4822 50  0000 R CNN
F 2 "" H 7150 4950 50  0001 C CNN
F 3 "" H 7150 4950 50  0001 C CNN
	1    7150 4950
	0    1    1    0   
$EndComp
NoConn ~ 7400 4850
Text Label 7150 4750 0    50   ~ 0
SDA
Text Label 7150 4650 0    50   ~ 0
SCL
Wire Wire Line
	6000 4450 7400 4450
Connection ~ 6000 4450
Wire Wire Line
	6000 4450 6000 4650
Wire Wire Line
	5750 4550 7400 4550
$Comp
L Connector_Generic:Conn_01x03 J5
U 1 1 5F02EE88
P 7100 3850
F 0 "J5" H 7250 3850 50  0000 C CNN
F 1 "OneWire sensor" H 7100 4100 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 7100 3850 50  0001 C CNN
F 3 "~" H 7100 3850 50  0001 C CNN
	1    7100 3850
	1    0    0    -1  
$EndComp
Connection ~ 6000 3750
$Comp
L Device:R R6
U 1 1 5F036A1B
P 6250 4200
F 0 "R6" V 6043 4200 50  0000 C CNN
F 1 "4k7" V 6134 4200 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal" V 6180 4200 50  0001 C CNN
F 3 "~" H 6250 4200 50  0001 C CNN
	1    6250 4200
	0    1    1    0   
$EndComp
Wire Wire Line
	6900 4200 6900 3950
Wire Wire Line
	6000 4200 6100 4200
Connection ~ 6000 4200
Wire Wire Line
	6000 4200 6000 4450
Wire Wire Line
	6400 4200 6900 4200
Wire Wire Line
	6900 3950 6550 3950
Connection ~ 6900 3950
Wire Wire Line
	5750 3850 6900 3850
Wire Wire Line
	6000 3750 6900 3750
Wire Wire Line
	6000 3400 6000 3750
Wire Wire Line
	6000 3750 6000 4200
$Comp
L Diode:1N914 D3
U 1 1 5F0654D3
P 5400 3850
F 0 "D3" H 5400 3633 50  0000 C CNN
F 1 "1N914" H 5400 3724 50  0000 C CNN
F 2 "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal" H 5400 3675 50  0001 C CNN
F 3 "http://www.vishay.com/docs/85622/1n914.pdf" H 5400 3850 50  0001 C CNN
	1    5400 3850
	-1   0    0    1   
$EndComp
$Comp
L Diode:1N914 D2
U 1 1 5F0666CE
P 5100 3850
F 0 "D2" H 5100 3633 50  0000 C CNN
F 1 "1N914" H 5100 3724 50  0000 C CNN
F 2 "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal" H 5100 3675 50  0001 C CNN
F 3 "http://www.vishay.com/docs/85622/1n914.pdf" H 5100 3850 50  0001 C CNN
	1    5100 3850
	-1   0    0    1   
$EndComp
Text Label 4450 3800 0    50   ~ 0
OneWire
Wire Wire Line
	4450 3850 4450 3800
Wire Wire Line
	5750 3850 5550 3850
Wire Wire Line
	4450 3850 4800 3850
Connection ~ 4800 3850
Wire Wire Line
	4800 3850 4950 3850
$Comp
L Jumper:SolderJumper_2_Open JP3
U 1 1 5EF9D131
P 5450 5450
F 0 "JP3" V 5450 5200 50  0000 L CNN
F 1 " " V 5350 4950 50  0000 L CNN
F 2 "Jumper:SolderJumper-2_P1.3mm_Open_TrianglePad1.0x1.5mm" H 5450 5450 50  0001 C CNN
F 3 "~" H 5450 5450 50  0001 C CNN
	1    5450 5450
	0    -1   -1   0   
$EndComp
$Comp
L Jumper:SolderJumper_3_Open JP4
U 1 1 5F0DC052
P 5000 4150
F 0 "JP4" H 5000 4250 50  0000 C CNN
F 1 "Analog ref" H 4550 4150 50  0000 C CNN
F 2 "Jumper:SolderJumper-3_P1.3mm_Open_Pad1.0x1.5mm_NumberLabels" H 5000 4150 50  0001 C CNN
F 3 "~" H 5000 4150 50  0001 C CNN
	1    5000 4150
	1    0    0    -1  
$EndComp
Wire Wire Line
	5750 3850 5750 4150
Wire Wire Line
	5200 4150 5750 4150
Connection ~ 5750 4150
Wire Wire Line
	5750 4150 5750 4550
Wire Wire Line
	4800 3850 4800 4150
Wire Wire Line
	5000 4450 5000 4300
Wire Wire Line
	4700 4450 5000 4450
$EndSCHEMATC
