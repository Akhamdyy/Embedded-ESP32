ESP32 Series
Datasheet Version 5.2

2.4 GHz Wi-Fi + Bluetooth® + Bluetooth LE SoC

Including:

ESP32-D0WD-V3

ESP32-U4WDH

ESP32-S0WD – Not Recommended for New Designs (NRND)

ESP32-D0WD – Not Recommended for New Designs (NRND)

ESP32-D0WDQ6 – Not Recommended for New Designs (NRND)

ESP32-D0WDQ6-V3 – Not Recommended for New Designs (NRND)

ESP32-D0WDR2-V3 – End of Life (EOL), upgraded to ESP32-D0WDRH2-V3

www.espressif.com

Product Overview

ESP32 is a single 2.4 GHz Wi-Fi-and-Bluetooth combo chip designed with the TSMC low-power 40 nm

technology. It is designed to achieve the best power and RF performance, showing robustness, versatility and

reliability in a wide variety of applications and power scenarios.

For details on part numbers and ordering information, please refer to Section 1 ESP32 Series Comparison. For

details on chip revisions, please refer to ESP32 Chip Revision v3.0 User Guide and

ESP32 Series SoC Errata.

The functional block diagram of the SoC is shown below.

ESP32 Functional Block Diagram

Espressif Systems

2
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

Core and memoryROMCryptographic hardware accelerationAESSHARSARTCULP coprocessorRecovery memoryPMUBluetooth link controllerBluetooth basebandWi-Fi MAC Wi-Fi basebandSPI2 (or 1) x Xtensa® 32-bit LX6 Microprocessors RF receiveRF transmitSwitchBalunI2CI2SSDIOUARTTWAI®ETHRMTPWMTouch sensorDACADCClock generatorRNGSRAMIn-Package Flash or PSRAMTimersFeatures

Wi-Fi

• 802.11b/g/n

• 802.11n (2.4 GHz), up to 150 Mbps

• WMM

• TX/RX A-MPDU, RX A-MSDU

• Immediate Block ACK

• Defragmentation

• Automatic Beacon monitoring (hardware TSF)

• Four virtual Wi-Fi interfaces

• Simultaneous support for Infrastructure Station, SoftAP, and Promiscuous modes

Note that when ESP32 is in Station mode, performing a scan, the SoftAP channel will be changed.

• Antenna diversity

Bluetooth®

• Compliant with Bluetooth v4.2 BR/EDR and Bluetooth LE specifications

• Class-1, class-2 and class-3 transmitter without external power amplifier

• Enhanced Power Control

• +9 dBm transmitting power

• NZIF receiver with –94 dBm Bluetooth LE sensitivity

• Adaptive Frequency Hopping (AFH)

• Standard HCI based on SDIO/SPI/UART

• High-speed UART HCI, up to 4 Mbps

• Bluetooth 4.2 BR/EDR and Bluetooth LE dual mode controller

• Synchronous Connection-Oriented/Extended (SCO/eSCO)

• CVSD and SBC for audio codec

• Bluetooth Piconet and Scatternet

• Multi-connections in Classic Bluetooth and Bluetooth LE

• Simultaneous advertising and scanning

CPU and Memory

• Xtensa® single-/dual-core 32-bit LX6 microprocessor(s)

• CoreMark® score:

– 1 core at 240 MHz: 539.98 CoreMark; 2.25 CoreMark/MHz

Espressif Systems

3
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

– 2 cores at 240 MHz: 1079.96 CoreMark; 4.50 CoreMark/MHz

• 448 KB ROM

• 520 KB SRAM

• 16 KB SRAM in RTC

• QSPI supports multiple flash/SRAM chips

Clocks and Timers

• Internal 8 MHz oscillator with calibration

• Internal RC oscillator with calibration

• External 2 MHz ~ 60 MHz crystal oscillator (40 MHz only for Wi-Fi/Bluetooth functionality)

• External 32 kHz crystal oscillator for RTC with calibration

• Two timer groups, including 2 × 64-bit timers and 1 × main watchdog in each group

• One RTC timer

• RTC watchdog

Advanced Peripheral Interfaces

• 34 programmable GPIOs

– Five strapping GPIOs

– Six input-only GPIOs

– Six GPIOs needed for in-package flash (ESP32-U4WDH) and in-package PSRAM

(ESP32-D0WDRH2-V3)

• 12-bit SAR ADC up to 18 channels

• Two 8-bit DAC

• 10 touch sensors

• Four SPI interfaces

• Two I2S interfaces

• Two I2C interfaces

• Three UART interfaces

• One host (SD/eMMC/SDIO)

• One slave (SDIO/SPI)

• Pulse count controller

• Ethernet MAC interface with dedicated DMA and IEEE 1588 support

• TWAI®, compatible with ISO 11898-1 (CAN Specification 2.0)

• RMT (TX/RX)

Espressif Systems

4
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

• Motor PWM

• LED PWM up to 16 channels

Power Management

• Fine-resolution power control through a selection of clock frequency, duty cycle, Wi-Fi operating modes,

and individual power control of internal components

• Five power modes designed for typical scenarios: Active, Modem-sleep, Light-sleep, Deep-sleep,

Hibernation

• Power consumption in Deep-sleep mode is 10 µA

• Ultra-Low-Power (ULP) coprocessors

• RTC memory remains powered on in Deep-sleep mode

Security

• Secure boot

• Flash encryption

• 1024-bit OTP, up to 768-bit for customers

• Cryptographic hardware acceleration:

– AES

– Hash (SHA-2)

– RSA

– Random Number Generator (RNG)

Applications

With low power consumption, ESP32 is an ideal choice for IoT devices in the following areas:

• Smart Home

• Industrial Automation

• Health Care

• Consumer Electronics

• Smart Agriculture

• POS Machines

• Service Robot

• Audio Devices

• Generic Low-power IoT Sensor Hubs

• Generic Low-power IoT Data Loggers

• Cameras for Video Streaming

• Speech Recognition

• Image Recognition

• SDIO Wi-Fi + Bluetooth Networking Card

Espressif Systems

5
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

Contents

Note:

Check the link or the QR code to make sure that you use the latest version of this document:

https://www.espressif.com/documentation/esp32_datasheet_en.pdf

Contents

Product Overview
Features

Applications

1
1.1

1.2

ESP32 Series Comparison
Nomenclature

Comparison

2 Pins
2.1

Pin Layout

2.2

2.3

2.4

2.5

Pin Overview

IO Pins

2.3.1

Restrictions for GPIOs and RTC_GPIOs

Analog Pins

Power Supply

2.5.1

Power Pins

2.5.2 Power Scheme

2.5.3 Chip Power-up and Reset

2.6

Pin Mapping Between Chip and Flash/PSRAM

3 Boot Configurations
3.1

Chip Boot Mode Control

3.2

3.3

3.4

3.5

4
4.1

Internal LDO (VDD_SDIO) Voltage Control

U0TXD Printing Control

Timing Control of SDIO Slave

JTAG Signal Source Control

Functional Description
CPU and Memory

4.1.1

4.1.2

4.1.3

4.1.4

4.1.5

CPU

Internal Memory

External Flash and RAM

Address Mapping Structure

Cache

4.2

System Clocks

4.2.1

CPU Clock

2

3

5

11

11

11

12

12

14

17

17

17

17

17

18

19

20

22

23

24

25

25

25

26

26

26

26

27

27

29

29

29

Espressif Systems

6
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

Contents

4.2.2

RTC Clock

4.2.3

Audio PLL Clock

4.3

RTC and Low-power Management

4.3.1

Power Management Unit (PMU)

4.3.2 Ultra-Low-Power Coprocessor

4.4

Timers and Watchdogs

4.5

4.6

4.4.1

General Purpose Timers

4.4.2 Watchdog Timers

Cryptographic Hardware Accelerators

Radio and Wi-Fi

4.6.1

2.4 GHz Receiver

4.6.2

2.4 GHz Transmitter

4.6.3 Clock Generator

4.6.4 Wi-Fi Radio and Baseband

4.6.5 Wi-Fi MAC

4.7

Bluetooth

4.7.1

4.7.2

4.7.3

4.7.4

Bluetooth Radio and Baseband

Bluetooth Interface

Bluetooth Stack

Bluetooth Link Controller

4.8

Digital Peripherals

4.8.1

General Purpose Input / Output Interface (GPIO)

4.8.2

Serial Peripheral Interface (SPI)

4.8.3 Universal Asynchronous Receiver Transmitter (UART)

4.8.4

4.8.5

4.8.6

4.8.7

I2C Interface

I2S Interface

Remote Control Peripheral

Pulse Counter Controller (PCNT)

4.8.8

LED PWM Controller

4.8.9 Motor Control PWM

4.8.10 SD/SDIO/MMC Host Controller

SDIO/SPI Slave Controller

4.8.11
4.8.12 TWAI® Controller
4.8.13 Ethernet MAC Interface

4.9

Analog Peripherals

4.9.1

Analog-to-Digital Converter (ADC)

4.9.2 Digital-to-Analog Converter (DAC)

4.9.3

Touch Sensor

4.10

Peripheral Pin Configurations

5
5.1

5.2

5.3

5.4

5.5

Electrical Characteristics
Absolute Maximum Ratings

Recommended Power Supply Characteristics

DC Characteristics (3.3 V, 25 °C)

RF Current Consumption in Active Mode

Reliability

29

30

30

30

31

31

31

31

32

32

32

33

33

33

34

34

34

34

35

35

37

37

37

37

38

39

39

39

40

40

41

42

43

43

44

44

45

45

47

52

52

52

53

53

54

Espressif Systems

7
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

Contents

5.6 Wi-Fi Radio

5.7

Bluetooth Radio

5.7.1

5.7.2

5.7.3

5.7.4

Receiver –Basic Data Rate
Transmitter –Basic Data Rate
Receiver –Enhanced Data Rate
Transmitter –Enhanced Data Rate

5.8

Bluetooth LE Radio

5.8.1

Receiver

5.8.2

Transmitter

6 Packaging

Related Documentation and Resources

Appendix A –ESP32 Pin Lists
A.1. Notes on ESP32 Pin Lists

A.2. GPIO_Matrix

A.3. Ethernet_MAC

A.4. IO_MUX

Revision History

54

55

55

55

56

56

57

57

59

60

61

62

62

64

69

69

71

Espressif Systems

8
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

List of Tables

List of Tables

1-1

ESP32 Series Comparison

2-1 Pin Overview

2-2 Analog Pins

2-3 Power Pins

2-4 Description of Timing Parameters for Power-up and Reset

2-5 Pin-to-Pin Mapping Between Chip and In-Package Flash/PSRAM

2-6 Pin-to-Pin Mapping Between Chip and Off-Package Flash/PSRAM

3-1 Default Configuration of Strapping Pins

3-2 Description of Timing Parameters for the Strapping Pins

3-3 Chip Boot Mode Control

3-4 U0TXD Printing Control

3-5 Timing Control of SDIO Slave

4-1 Memory and Peripheral Mapping

4-2 Power Consumption by Power Modes

4-3 ADC Characteristics

4-4 ADC Calibration Results

4-5 Capacitive-Sensing GPIOs Available on ESP32

4-6 Peripheral Pin Configurations

5-1 Absolute Maximum Ratings

5-2 Recommended Power Supply Characteristics

5-3 DC Characteristics (3.3 V, 25 °C)

5-4 Current Consumption Depending on RF Modes

5-5 Reliability Qualifications

5-6 Wi-Fi Radio Characteristics
5-7 Receiver Characteristics –Basic Data Rate
5-8 Transmitter Characteristics –Basic Data Rate
5-9 Receiver Characteristics –Enhanced Data Rate
5-10 Transmitter Characteristics –Enhanced Data Rate
5-11 Receiver Characteristics –Bluetooth LE
5-12 Transmitter Characteristics –Bluetooth LE

6-1 Notes on ESP32 Pin Lists

6-2 GPIO_Matrix

6-3 Ethernet_MAC

11

14

17

18

19

20

20

22

23

23

25

25

28

30

44

45

46

47

52

52

53

53

54

54

55

55

56

57

57

59

62

64

69

Espressif Systems

9
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

List of Figures

List of Figures

1-1

ESP32 Series Nomenclature

2-1 ESP32 Pin Layout (QFN 6*6, Top View)

2-2 ESP32 Pin Layout (QFN 5*5, Top View)

2-3 ESP32 Power Scheme

2-4 Visualization of Timing Parameters for Power-up and Reset

3-1 Visualization of Timing Parameters for the Strapping Pins

3-2 Chip Boot Flow

4-1 Address Mapping Structure

6-1 QFN48 (6×6 mm) Package

6-2 QFN48 (5×5 mm) Package

11

12

13

18

19

23

24

27

60

60

Espressif Systems

10
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

1 ESP32 Series Comparison

1 ESP32 Series Comparison

1.1 Nomenclature

Figure 1-1. ESP32 Series Nomenclature

1.2 Comparison

Table 1-1. ESP32 Series Comparison

Part Number1

ESP32-D0WD-V3

Core

Dual core

Chip Revision2
v3.0/v3.14

In-Package

Flash/PSRAM

—

VDD_SDIO

Package

Voltage

QFN 5*5

1.8 V/3.3 V

ESP32-D0WDR2-V3 (EOL)
Upgraded to ESP32-D0WDRH2-V3 7 Dual core
Dual core3
ESP32-U4WDH
Dual core

ESP32-D0WDQ6-V3 (NRND)

ESP32-D0WD (NRND)

ESP32-D0WDQ6 (NRND)

ESP32-S0WD (NRND)

Dual core

Dual core

Single core

v3.0/v3.14

v3.0/v3.14
v3.0/v3.14
v1.0/v1.15
v1.0/v1.15
v1.0/v1.15

2 MB PSRAM

QFN 5*5

3.3 V

4 MB flash6
—

—

—

—

QFN 5*5

3.3 V

QFN 6*6

1.8 V/3.3 V

QFN 5*5

1.8 V/3.3 V

QFN 6*6

1.8 V/3.3 V

QFN 5*5

1.8 V/3.3 V

1 All above chips support Wi-Fi b/g/n + Bluetooth/Bluetooth LE Dual Mode connection. For details on chip marking and

packing, see Section 6 Packaging.

2 Differences between ESP32 chip revisions and how to distinguish them are described in ESP32 Series SoC Errata.
3 ESP32-U4WDH will be produced as dual-core instead of single core. See PCN-2021-021 for details.
4 The chips will be produced with chip revision v3.1 inside. See PCN20220901 for details.
5 The chips will be produced with chip revision v1.1 inside. See PCN20220901 for details.
6 The in-package flash supports:

- More than 100,000 program/erase cycles

- More than 20 years data retention time

7 ESP32-D0WDR2-V3 is end of life and upgraded to ESP32-D0WDRH2-V3. See PCN20251001 for details.

Espressif Systems

11
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

ESP32DWDChip SeriesCoreD/U: Dual coreS: Single coreConnectionWD: Wi-Fi b/g/n + Bluetooth/Bluetooth LE dual modeR2HIn-package PSRAMR2: 2 MB PSRAMHigh temperature0In-package ﬂash0: No in-package ﬂash 2: 2 MB ﬂash4: 4 MB ﬂashQ6Chip revision v3.0 or newerV3PackageQ6: QFN 6*6N/A: QFN 5*52 Pins

2 Pins

2.1 Pin Layout

Figure 2-1. ESP32 Pin Layout (QFN 6*6, Top View)

Espressif Systems

12
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

32K_XP12VDET_21110987654321VDET_1CHIP_PUSENSOR_VNSENSOR_CAPNSENSOR_CAPPSENSOR_VPVDD3P3VDD3P3LNA_INVDDA252627282930313233343536GPIO16VDD_SDIOGPIO5VDD3P3_CPU37GPIO193839404142434445464748GPIO22U0RXDU0TXDGPIO21XTAL_NXTAL_PVDDACAP2CAP1GPIO224MTDO2322212019181716151413MTCKVDD3P3_RTCMTDIMTMSGPIO27GPIO26GPIO2532K_XNSD_DATA_2SD_DATA_3SD_CMDSD_CLKSD_DATA_0SD_DATA_1GPIO4GPIO0GPIO23GPIO18VDDAGPIO17ESP3249 GND2 Pins

Figure 2-2. ESP32 Pin Layout (QFN 5*5, Top View)

Espressif Systems

13
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

10987654321VDET_1CHIP_PUSENSOR_VNSENSOR_CAPNSENSOR_CAPPSENSOR_VPVDD3P3VDD3P3LNA_INVDDA25262728293031323334GPIO16VDD_SDIOGPIO5VDD3P3_CPUGPIO1939404142434445464748GPIO22U0RXDU0TXDGPIO21XTAL_NXTAL_PVDDACAP2CAP1GPIO224MTDO232221201918171615MTCKVDD3P3_RTCMTDIMTMSGPIO27GPIO26GPIO2532K_XNSD_DATA_2SD_DATA_3SD_CMDSD_CLKSD_DATA_0SD_DATA_1GPIO4GPIO0VDDAGPIO1732K_XPVDET_2GPIO18GPIO231112131435363738ESP3249 GNDS
u
b
m

i
t

D
o
c
u
m
e
n
t
a
t
i
o
n
F
e
e
d
b
a
c
k

E
s
p
r
e
s
s
i
f
S
y
s
t
e
m
s

1
4

E
S
P
3
2
S
e
r
i
e
s
D
a
t
a
s
h
e
e
t

v
5
.
2

2.2 Pin Overview

Name

No.

Type

Function

Table 2-1. Pin Overview

Analog

2

i

P
n
s

VDDA

LNA_IN

VDD3P3

VDD3P3

SENSOR_VP

SENSOR_CAPP

SENSOR_CAPN

SENSOR_VN

CHIP_PU

VDET_1

VDET_2

32K_XP

32K_XN

GPIO25

GPIO26

GPIO27

MTMS

MTDI

VDD3P3_RTC

MTCK

MTDO

1

2

3

4

5

6

7

8

9

10

11

12

13

14

15

16

17

18

19

20

21

P

I/O

P

P

Analog power supply (2.3 V ∼ 3.6 V)
RF input and output
Analog power supply (2.3 V ∼ 3.6 V)
Analog power supply (2.3 V ∼ 3.6 V)

VDD3P3_RTC

GPIO36, ADC1_CH0,

RTC_GPIO0

GPIO37,

ADC1_CH1,

RTC_GPIO1

GPIO38, ADC1_CH2,

RTC_GPIO2

GPIO39, ADC1_CH3,

RTC_GPIO3

High: On; enables the chip

Low: Off; the chip shuts down

Note: Do not leave the CHIP_PU pin floating.

GPIO34, ADC1_CH6,

RTC_GPIO4

GPIO35, ADC1_CH7,

RTC_GPIO5

GPIO32, ADC1_CH4,

RTC_GPIO9,

TOUCH9,

32K_XP (32.768 kHz crystal oscillator input)

GPIO33, ADC1_CH5,

RTC_GPIO8,

TOUCH8,

32K_XN (32.768 kHz crystal oscillator output)

GPIO25, ADC2_CH8,

RTC_GPIO6,

DAC_1,

EMAC_RXD0

GPIO26, ADC2_CH9,

RTC_GPIO7,

DAC_2,

EMAC_RXD1

GPIO27,

ADC2_CH7,

RTC_GPIO17,

TOUCH7,

EMAC_RX_DV

GPIO14,

ADC2_CH6,

RTC_GPIO16, TOUCH6,

EMAC_TXD2,

HSPICLK,

HS2_CLK,

SD_CLK,

MTMS

ADC2_CH5,

GPIO12,
Input power supply for RTC IO (2.3 V ∼ 3.6 V)
GPIO13,

ADC2_CH4,

RTC_GPIO14, TOUCH4,

RTC_GPIO15, TOUCH5,

EMAC_TXD3,

HSPIQ,

HS2_DATA2, SD_DATA2, MTDI

EMAC_RX_ER, HSPID,

HS2_DATA3, SD_DATA3, MTCK

GPIO15,

ADC2_CH3,

RTC_GPIO13,

TOUCH3,

EMAC_RXD3,

HSPICS0, HS2_CMD,

SD_CMD,

MTDO

I

I

I

I

I

I

I

I/O

I/O

I/O

I/O

I/O

I/O

I/O

P

I/O

I/O

2

i

P
n
s

S
u
b
m

i
t

D
o
c
u
m
e
n
t
a
t
i
o
n
F
e
e
d
b
a
c
k

E
s
p
r
e
s
s
i
f
S
y
s
t
e
m
s

1
5

E
S
P
3
2
S
e
r
i
e
s
D
a
t
a
s
h
e
e
t

v
5
.
2

Name

GPIO2

GPIO0

GPIO4

GPIO16

VDD_SDIO

GPIO17

SD_DATA_2

SD_DATA_3

SD_CMD

SD_CLK

SD_DATA_0

SD_DATA_1

GPIO5

GPIO18

GPIO23

VDD3P3_CPU

GPIO19

GPIO22

U0RXD

U0TXD

GPIO21

VDDA

XTAL_N

XTAL_P

VDDA

CAP2

No.

Type

Function

22

23

24

25

26

27

28

29

30

31

32

33

34

35

36

37

38

39

40

41

42

43

44

45

46

47

I/O

I/O

I/O

I/O

P

I/O

I/O

I/O

I/O

I/O

I/O

I/O

I/O

I/O

I/O

P

I/O

I/O

I/O

I/O

I/O

P

O

I

P

I

GPIO2,

ADC2_CH2,

RTC_GPIO12, TOUCH2,

HSPIWP,

HS2_DATA0, SD_DATA0

GPIO0,

ADC2_CH1,

RTC_GPIO11,

TOUCH1,

EMAC_TX_CLK, CLK_OUT1,

GPIO4,

ADC2_CH0,

RTC_GPIO10, TOUCH0,

EMAC_TX_ER, HSPIHD,

HS2_DATA1, SD_DATA1

VDD_SDIO

GPIO16, HS1_DATA4,

U2RXD,

EMAC_CLK_OUT

Output power supply: 1.8 V or the same voltage as VDD3P3_RTC

GPIO17,

HS1_DATA5,

U2TXD,

EMAC_CLK_OUT_180

GPIO9,

HS1_DATA2,

U1RXD,

SD_DATA2,

SPIHD

GPIO10, HS1_DATA3,

U1TXD,

SD_DATA3,

SPIWP

GPIO11,

HS1_CMD,

GPIO6,

HS1_CLK,

U1RTS,

U1CTS,

GPIO7,

HS1_DATA0,

U2RTS,

GPIO8,

HS1_DATA1,

U2CTS,

SD_CMD,

SPICS0

SD_CLK,

SPICLK

SD_DATA0,

SD_DATA1,

SPIQ

SPID

GPIO5,

HS1_DATA6,

VSPICS0,

EMAC_RX_CLK

GPIO18, HS1_DATA7,

VSPICLK

VDD3P3_CPU

GPIO23, HS1_STROBE, VSPID
Input power supply for CPU IO (1.8 V ∼ 3.6 V)
GPIO19, U0CTS,

VSPIQ,

EMAC_TXD0

GPIO22, U0RTS,

VSPIWP,

EMAC_TXD1

GPIO3,

U0RXD,

CLK_OUT2

GPIO1,

U0TXD,

CLK_OUT3,

EMAC_RXD2

GPIO21,

VSPIHD,

EMAC_TX_EN

Analog

Analog power supply (2.3 V ∼ 3.6 V)
External crystal output

External crystal input
Analog power supply (2.3 V ∼ 3.6 V)
Connects to a 3.3 nF (10%) capacitor and 20 kΩ resistor in parallel to CAP1

Name

CAP1

GND

No.

Type

Function

48

49

I

P

Connects to a 10 nF series capacitor to ground

Ground

2

i

P
n
s

Notes for Table 2-1 Pin Overview:

1. Function names:

CLK_OUT… clock output



SPICLK

HSPICLK

VSPICLK



SPI clock signal

HS…_CLK

SDIO Master clock signal

SD_CLK

SDIO Slave clock signal

}

EMAC_TX_CLK

EMAC_RX_CLK

U…_RTS

U…_CTS

U…_RXD

U…_TXD

MTMS

MTDI

MTCK

MTDO

}

}






EMAC clock signal

UART0/1/2 hardware flow control signals

UART0/1/2 receive/transmit signals

JTAG interface signals

GPIO… General-purpose input/output with signals routed via the GPIO matrix. For

more details on the GPIO matrix, see ESP32 Technical Reference Manual >
Chapter IO MUX and GPIO Matrix。

2. Regarding highlighted cells, see Section 2.3.1 Restrictions for GPIOs and RTC_GPIOs.

3. For a quick reference guide to using the IO_MUX, Ethernet MAC, and GPIO Matrix pins of ESP32, please refer to Appendix ESP32 Pin Lists.

S
u
b
m

i
t

D
o
c
u
m
e
n
t
a
t
i
o
n
F
e
e
d
b
a
c
k

E
s
p
r
e
s
s
i
f
S
y
s
t
e
m
s

1
6

E
S
P
3
2
S
e
r
i
e
s
D
a
t
a
s
h
e
e
t

v
5
.
2

2 Pins

2.3 IO Pins

2.3.1 Restrictions for GPIOs and RTC_GPIOs

All IO pins of the ESP32 have GPIO and some have RTC_GPIO pin functions. However, these IO pins are

multifunctional and can be configured for different purposes based on the requirements. Some IOs have

restrictions for usage. It is essential to consider their multiplexed nature and the limitations when using these

IO pins.

In Table 2-1 Pin Overview some pin functions are highlighted, specically:

• GPIO – Input only pins, output is not supported due to lack of pull-up/pull-down resistors.

• GPIO – allocated for communication with in-package flash/PSRAM and NOT recommended for other

uses. For details, see Section 2.6 Pin Mapping Between Chip and Flash/PSRAM.

• GPIO – have one of the following important functions:

– Strapping pins – need to be at certain logic levels at startup. See Section 3 Boot Configurations.

– JTAG interface – often used for debugging.

– UART interface – often used for debugging.

See also Appendix A.1 – Notes on ESP32 Pin Lists.

2.4 Analog Pins

Table 2-2. Analog Pins

Pin

Pin

No. Name

Pin

Pin

Type

Function

2

9

44

45

LNA_IN

I/O

CHIP_PU

XTAL_N

XTAL_P

I

—

—

Low Noise Amplifier (LNA) input signal, Power Amplifier (PA) output signal
High: on, enables the chip (Powered up).
Low: off, the chip powers off (powered down).
Note: Do not leave the CHIP_PU pin floating.

External clock input/output connected to chip’s crystal or oscillator.

P/N means differential clock positive/negative.

2.5 Power Supply

2.5.1 Power Pins

ESP32’s digital pins are divided into three different power domains:

• VDD3P3_RTC

• VDD3P3_CPU

• VDD_SDIO

Espressif Systems

17
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

2 Pins

Pin

Pin

No. Name

1

3

4

19

26

37

43

46

49

VDDA

VDD3P3

VDD3P3
VDD3P3_RTC1
VDD3P3_SDIO2
VDD3P3_CPU3
VDDA

VDDA

GND

Table 2-3. Power Pins

Direction

Power Domain / Other

IO Pins

Power Supply

Input

Input

Input

Input

Analog power domain

Analog power domain

Analog power domain

RTC and part of Digital power domains

RTC IO

Input/Output

Analog power domain

Input

Input

Input

–

Digital power domain

Analog power domain

Analog power domain

External ground connection

Digital IO

1 VDD3P3_RTC is also the input power supply for RTC and CPU.
2 VDD_SDIO connects to the output of an internal LDO whose input is VDD3P3_RTC. When
VDD_SDIO is connected to the same PCB net together with VDD3P3_RTC, the internal

LDO is disabled automatically.

3 VDD3P3_CPU is also the input power supply for CPU.

2.5.2 Power Scheme

The power scheme is shown in Figure 2-3 ESP32 Power Scheme.

Figure 2-3. ESP32 Power Scheme

Espressif Systems

18
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

SDIODomainRTCDomainCPUDomainLDOLDOLDO1.8 V1.1 V1.1 VVDD3P3_RTCVDD3P3_CPUVDD_SDIO3.3 V/1.8 VR = 6 Ω2 Pins

The internal LDO can be configured as having 1.8 V, or the same voltage as VDD3P3_RTC. It can be powered

off via software to minimize the current of flash/SRAM during the Deep-sleep mode.

2.5.3 Chip Power-up and Reset

Once the power is supplied to the chip, its power rails need a short time to stabilize. After that, CHIP_PU – the

pin used for power-up and reset – is pulled high to activate the chip. For information on CHIP_PU as well as

power-up and reset timing, see Figure 2-4 and Table 2-4.

tST BL

tRST

VDD3P3_RTC Min

VDD

CHIP_PU

Figure 2-4. Visualization of Timing Parameters for Power-up and Reset

Table 2-4. Description of Timing Parameters for Power-up and Reset

Parameter Description

tST BL

tRST

Time reserved for the 3.3 V rails to stabilize before the CHIP_PU

pin is pulled high to activate the chip

Time reserved for CHIP_PU to stay below VIL_nRST to reset the
chip (see Table 5-3)

Min (µs)

50

50

• In scenarios where ESP32 is powered up and down repeatedly by switching the power rails, while there
is a large capacitor on the VDD33 rail and CHIP_PU and VDD33 are connected, simply switching off the

CHIP_PU power rail and immediately switching it back on may cause an incomplete power discharge

cycle and failure to reset the chip adequately.

An additional discharge circuit may be required to accelerate the discharge of the large capacitor on rail

VDD33, which will ensure proper power-on-reset when the ESP32 is powered up again.

• When a battery is used as the power supply for the ESP32 series of chips and modules, a supply voltage

supervisor is recommended, so that a boot failure due to low voltage is avoided. Users are

recommended to pull CHIP_PU low if the power supply for ESP32 is below 2.3 V.

Notes on power supply:

• The operating voltage of ESP32 ranges from 2.3 V to 3.6 V. When using a single-power supply, the

recommended voltage of the power supply is 3.3 V, and its recommended output current is 500 mA or

more.

• PSRAM and flash both are powered by VDD_SDIO. If the chip has an in-package flash, the voltage of

VDD_SDIO is determined by the operating voltage of the in-package flash. If the chip also connects to

an external PSRAM, the operating voltage of external PSRAM must match that of the in-package flash.

This also applies if the chip has an in-package PSRAM but also connects to an external flash.

Espressif Systems

19
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

VIL_nRST2 Pins

• When VDD_SDIO 1.8 V is used as the power supply for external flash/PSRAM, a 2 kΩ grounding resistor

should be added to VDD_SDIO. For the circuit design, please refer to

ESP32 Hardware Design Guidelines.

• When the three digital power supplies are used to drive peripherals, e.g., 3.3 V flash, they should comply

with the peripherals’ specifications.

2.6 Pin Mapping Between Chip and Flash/PSRAM

Table 2-5 lists the pin-to-pin mapping between the chip and the in-package flash/PSRAM. The chip pins listed

here are not recommended for other usage.

For the data port connection between ESP32 and off-package flash/PSRAM please refer to Table 2-6.

Table 2-5. Pin-to-Pin Mapping Between Chip and In-Package Flash/PSRAM

ESP32-U4WDH

In-Package Flash (4 MB)

SD_DATA_1

GPIO17

SD_DATA_0

SD_CMD

SD_CLK

GPIO16

GND
VDD_SDIO1

IO0/DI

IO1/DO

IO2/WP#

IO3/HOLD#

CLK

CS#

VSS

VDD

ESP32-D0WDRH2-V3

In-Package PSRAM (2 MB)

SD_DATA_1

SD_DATA_0

SD_DATA_3

SD_DATA_2

SD_CLK
GPIO162

GND
VDD_SDIO1

SIO0/SI

SIO1/SO

SIO2

SIO3

SCLK

CE#

VSS

VDD

Table 2-6. Pin-to-Pin Mapping Between Chip and Off-Package Flash/PSRAM

Chip Pin

Off-Package Flash

SD_DATA_1/SPID

SD_DATA_0/SPIQ

IO0/DI

IO1/DO

SD_DATA_3/SPIWP

IO2/WP#

SD_DATA_2/SPIHD

IO3/HOLD#

SD_CLK

SD_CMD

GND

VDD_SDIO

CLK

CS#

VSS

VDD

Cont’d on next page

Espressif Systems

20
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

2 Pins

Note:

Table 2-6 – cont’d from previous page

Chip Pin

Chip Pin

SD_DATA_1

SD_DATA_0

SD_DATA_3

SD_DATA_2
SD_CLK/GPIO173
GPIO162

GND

VDD_SDIO

Off-Package PSRAM

Off-Package PSRAM

SIO0/SI

SIO1/SO

SIO2

SIO3

SCLK

CE#

VSS

VDD

1. As the in-package flash (ESP32-U4WDH) and the in-package PSRAM (ESP32-D0WDRH2-V3) operate at 3.3 V,

VDD_SDIO must be powered by VDD3P3_RTC via a 6 Ω resistor. See Figure 2-3 ESP32 Power Scheme.

2. If GPIO16 is used to connect to PSRAM’s CE# signal, please add a pull-up resistor at the GPIO16 pin. See

ESP32-WROVER-E Datasheet > Figure Schematics of ESP32-WROVER-E.

3. SD_CLK and GPIO17 pins are available to connect to the SCLK signal of external PSRAM.

• If SD_CLK pin is selected, one GPIO (i.e., GPIO17) will be saved. The saved GPIO can be used for other
purposes. This connection has passed internal tests, but relevant certification has not been completed.
• Or GPIO17 pin is used to connect to the SCLK signal. This connection has passed relevant certification,

see certificates for ESP32-WROVER-E.

Please select the proper pin for your specific applications.

Espressif Systems

21
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

3 Boot Configurations

3 Boot Configurations

The chip allows for configuring the following boot parameters through strapping pins and eFuse bits at

power-up or a hardware reset, without microcontroller interaction.

• Chip boot mode

– Strapping pin: GPIO0 and GPIO2

• Internal LDO (VDD_SDIO) Voltage

– Strapping pin: MTDI

– eFuse bit: EFUSE_SDIO_FORCE and EFUSE_SDIO_TIEH

• U0TXD printing

– Strapping pin: MTDO

• Timing of SDIO Slave

– Strapping pin: MTDO and GPIO5

• JTAG signal source

– eFuse bit: EFUSE_DISABLE_JTAG

The default values of all the above eFuse bits are 0, which means that they are not burnt. Given that eFuse is

one-time programmable, once an eFuse bit is programmed to 1, it can never be reverted to 0. For how to

program eFuse bits, please refer to ESP32 Technical Reference Manual > Chapter eFuse Controller.

The default values of the strapping pins, namely the logic levels, are determined by pins’internal weak

pull-up/pull-down resistors at reset if the pins are not connected to any circuit, or connected to an external

high-impedance circuit.

Table 3-1. Default Configuration of Strapping Pins

Strapping Pin Default Configuration Bit Value

GPIO0

GPIO2

MTDI

MTDO

GPIO5

Pull-up

Pull-down

Pull-down

Pull-up

Pull-up

1

0

0

1

1

To change the bit values, the strapping pins should be connected to external pull-down/pull-up resistances. If

the ESP32 is used as a device by a host MCU, the strapping pin voltage levels can also be controlled by the

host MCU.

All strapping pins have latches. At system reset, the latches sample the bit values of their respective strapping

pins and store them until the chip is powered down or shut down. The states of latches cannot be changed in

any other way. It makes the strapping pin values available during the entire chip operation, and the pins are

freed up to be used as regular IO pins after reset.

The timing of signals connected to the strapping pins should adhere to the setup time and hold time

specifications in Table 3-2 and Figure 3-1.

Espressif Systems

22
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

3 Boot Configurations

Table 3-2. Description of Timing Parameters for the Strapping Pins

Parameter Description

tSU

tH

Setup time is the time reserved for the power rails to stabilize be-

fore the CHIP_PU pin is pulled high to activate the chip.

Hold time is the time reserved for the chip to read the strapping

pin values after CHIP_PU is already high and before these pins

start operating as regular IO pins.

Min (ms)

0

1

tSU

tH

CHIP_PU

Figure 3-1. Visualization of Timing Parameters for the Strapping Pins

3.1 Chip Boot Mode Control

GPIO0 and GPIO2 control the boot mode after the reset is released. See Table 3-3 Chip Boot Mode

Control.

Table 3-3. Chip Boot Mode Control

Boot Mode

GPIO0

GPIO2

SPI Boot Mode
Joint Download Boot Mode 2

1

0

Any value

0

1 Bold marks the default value and configuration.
2 Joint Download Boot mode supports the following

download methods:

• SDIO Download Boot
• UART Download Boot

In Joint Download Boot mode, the detailed boot flow of the chip is put below 3-2.

Espressif Systems

23
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

Strapping pinVIH_nRSTVIH3 Boot Configurations

Figure 3-2. Chip Boot Flow

uart_download_dis controls boot mode behaviors:

It permanently disables Download Boot mode when uart_download_dis is set to 1 (valid only for ESP32 chip

revisions v3.0 and higher).

3.2 Internal LDO (VDD_SDIO) Voltage Control

The required VDD_SPI voltage for the chips of the ESP32 Series can be found in Table 1-1 Comparison.

MTDI is used to select the VDD_SDIO power supply voltage at reset:

• MTDI = 0 (by default), VDD_SDIO pin is powered directly from VDD3P3_RTC. Typically this voltage is 3.3

V. For more information, see Section 2.5.2 Power Scheme.

• MTDI = 1, VDD_SDIO pin is powered from internal 1.8 V LDO.

This functionality can be overridden by setting EFUSE_SDIO_FORCE to 1, in which case the EFUSE_SDIO_TIEH

determines the VDD_SDIO voltage:

• EFUSE_SDIO_TIEH = 0, VDD_SDIO connects to 1.8 V LDO.

• EFUSE_SDIO_TIEH = 1, VDD_SDIO connects to VDD3P3_RTC.

Espressif Systems

24
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

3 Boot Configurations

3.3 U0TXD Printing Control

During booting, the strapping pin MTDO can be used to control the U0TXD Printing, as Table 3-4 shows.

Table 3-4. U0TXD Printing Control

U0TXD Printing Control MTDO
Enabled 1
Disabled

0

1

1 Bold marks the default value and

configuration.

3.4 Timing Control of SDIO Slave

The strapping pin MTDO and GPIO5 can be used to control the timing of SDIO slave, see Table 3-5 Timing

Control of SDIO Slave.

Table 3-5. Timing Control of SDIO Slave

Edge behavior

MTDO GPIO5

Falling edge sampling, falling edge output

Falling edge sampling, rising edge output

Rising edge sampling, falling edge output

Rising edge sampling, rising edge output

0

0

1

1

1 Bold marks the default value and configuration.

0

1

0

1

3.5 JTAG Signal Source Control

If EFUSE_DISABLE_JTAG is set to 1, the source of JTAG signals can be disabled.

Espressif Systems

25
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

4 Functional Description

4 Functional Description

4.1 CPU and Memory

4.1.1 CPU

ESP32 contains one or two low-power Xtensa® 32-bit LX6 microprocessor(s) with the following
features:

• 7-stage pipeline to support the clock frequency of up to 240 MHz (160 MHz for ESP32-S0WD (NRND))

• 16/24-bit Instruction Set provides high code-density

• Support for Floating Point Unit

• Support for DSP instructions, such as a 32-bit multiplier, a 32-bit divider, and a 40-bit MAC

• Support for 32 interrupt vectors from about 70 interrupt sources

The single-/dual-CPU interfaces include:

• Xtensa RAM/ROM Interface for instructions and data

• Xtensa Local Memory Interface for fast peripheral register access

• External and internal interrupt sources

• JTAG for debugging

For information about the Xtensa® Instruction Set Architecture, please refer to
Xtensa® Instruction Set Architecture (ISA) Summary.

4.1.2 Internal Memory

ESP32’s internal memory includes:

• 448 KB of ROM for booting and core functions

• 520 KB of on-chip SRAM for data and instructions

• 8 KB of SRAM in RTC, which is called RTC FAST Memory and can be used for data storage; it is accessed

by the main CPU during RTC Boot from the Deep-sleep mode.

• 8 KB of SRAM in RTC, which is called RTC SLOW Memory and can be accessed by the ULP coprocessor

during the Deep-sleep mode.

• 1 Kbit of eFuse: 256 bits are used for the system (MAC address and chip configuration) and the

remaining 768 bits are reserved for customer applications, including flash-encryption and chip-ID.

• In-package flash or PSRAM

Note:

Products in the ESP32 series differ from each other, in terms of their support for in-package flash or PSRAM and the

size of them. For details, please refer to Section 1 ESP32 Series Comparison.

Espressif Systems

26
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

4 Functional Description

4.1.3 External Flash and RAM

ESP32 supports multiple external QSPI flash and external RAM (SRAM) chips. More details can be found in

ESP32 Technical Reference Manual > Chapter SPI Controller. ESP32 also supports hardware

encryption/decryption based on AES to protect developers’ programs and data in flash.

ESP32 can access the external QSPI flash and SRAM through high-speed caches.

• Up to 16 MB of external flash can be mapped into CPU instruction memory space and read-only memory

space simultaneously.

– When external flash is mapped into CPU instruction memory space, up to 11 MB + 248 KB can be

mapped at a time. Note that if more than 3 MB + 248 KB are mapped, cache performance will be

reduced due to speculative reads by the CPU.

– When external flash is mapped into read-only data memory space, up to 4 MB can be mapped at a

time. 8-bit, 16-bit and 32-bit reads are supported.

• External RAM can be mapped into CPU data memory space. SRAM up to 8 MB is supported and up to 4

MB can be mapped at a time. 8-bit, 16-bit and 32-bit reads and writes are supported.

Note:

After ESP32 is initialized, firmware can customize the mapping of external RAM or flash into the CPU address space.

4.1.4 Address Mapping Structure

The structure of address mapping is shown in Figure 4-1. The memory and peripheral mapping is shown in

Table 4-1.

Figure 4-1. Address Mapping Structure

Espressif Systems

27
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

4 Functional Description

Table 4-1. Memory and Peripheral Mapping

Category

Target

Start Address

End Address

Size

Internal ROM 0

0×4000_0000

0×4005_FFFF

384 KB

Internal ROM 1

0×3FF9_0000

0×3FF9_FFFF

Internal SRAM 0

0×4007_0000

0×4009_FFFF

Embedded

Internal SRAM 1

0×3FFE_0000

0×3FFF_FFFF

0×400A_0000

0×400B_FFFF

Internal SRAM 2

0×3FFA_E000

0×3FFD_FFFF

200 KB

RTC FAST Memory

0×3FF8_0000

0×3FF8_1FFF

0×400C_0000

0×400C_1FFF

RTC SLOW Memory

0×5000_0000

0×5000_1FFF

External Flash

0×3F40_0000

0×3F7F_FFFF

0×400C_2000

0×40BF_FFFF

11 MB+248 KB

64 KB

192 KB

128 KB

8 KB

8 KB

4 MB

Memory

External

Memory

Peripheral

External RAM

0×3F80_0000

0×3FBF_FFFF

4 MB

DPort Register

0×3FF0_0000

0×3FF0_0FFF

AES Accelerator

0×3FF0_1000

0×3FF0_1FFF

RSA Accelerator

0×3FF0_2000

0×3FF0_2FFF

SHA Accelerator

0×3FF0_3000

0×3FF0_3FFF

Secure Boot

0×3FF0_4000

0×3FF0_4FFF

4 KB

4 KB

4 KB

4 KB

4 KB

Cache MMU Table

0×3FF1_0000

0×3FF1_3FFF

16 KB

PID Controller

0×3FF1_F000

0×3FF1_FFFF

UART0

SPI1

SPI0

GPIO

RTC

IO MUX

SDIO Slave

UDMA1

I2S0

UART1

I2C0

UDMA0

0×3FF4_0000

0×3FF4_0FFF

0×3FF4_2000

0×3FF4_2FFF

0×3FF4_3000

0×3FF4_3FFF

0×3FF4_4000

0×3FF4_4FFF

0×3FF4_8000

0×3FF4_8FFF

0×3FF4_9000

0×3FF4_9FFF

0×3FF4_B000

0×3FF4_BFFF

0×3FF4_C000

0×3FF4_CFFF

0×3FF4_F000

0×3FF4_FFFF

0×3FF5_0000

0×3FF5_0FFF

0×3FF5_3000

0×3FF5_3FFF

0×3FF5_4000

0×3FF5_4FFF

SDIO Slave

0×3FF5_5000

0×3FF5_5FFF

RMT

PCNT

SDIO Slave

LED PWM

0×3FF5_6000

0×3FF5_6FFF

0×3FF5_7000

0×3FF5_7FFF

0×3FF5_8000

0×3FF5_8FFF

0×3FF5_9000

0×3FF5_9FFF

eFuse Controller

0×3FF5_A000

0×3FF5_AFFF

Flash Encryption

0×3FF5_B000

0×3FF5_BFFF

PWM0

TIMG0

TIMG1

SPI2

SPI3

0×3FF5_E000

0×3FF5_EFFF

0×3FF5_F000

0×3FF5_FFFF

0×3FF6_0000

0×3FF6_0FFF

0×3FF6_4000

0×3FF6_4FFF

0×3FF6_5000

0×3FF6_5FFF

4 KB

4 KB

4 KB

4 KB

4 KB

4 KB

4 KB

4 KB

4 KB

4 KB

4 KB

4 KB

4 KB

4 KB

4 KB

4 KB

4 KB

4 KB

4 KB

4 KB

4 KB

4 KB

4 KB

4 KB

4 KB

Espressif Systems

28
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

4 Functional Description

Category

Peripheral

Target

SYSCON

I2C1

SDMMC

EMAC

TWAI

PWM1

I2S1

UART2

PWM2

PWM3

RNG

Start Address

End Address

0×3FF6_6000

0×3FF6_6FFF

0×3FF6_7000

0×3FF6_7FFF

0×3FF6_8000

0×3FF6_8FFF

0×3FF6_9000

0×3FF6_AFFF

0×3FF6_B000

0×3FF6_BFFF

0×3FF6_C000

0×3FF6_CFFF

0×3FF6_D000

0×3FF6_DFFF

0×3FF6_E000

0×3FF6_EFFF

0×3FF6_F000

0×3FF6_FFFF

0×3FF7_0000

0×3FF7_0FFF

0×3FF7_5000

0×3FF7_5FFF

Size

4 KB

4 KB

4 KB

8 KB

4 KB

4 KB

4 KB

4 KB

4 KB

4 KB

4 KB

4.1.5 Cache

ESP32 uses a two-way set-associative cache. Each of the two CPUs has 32 KB of cache featuring a block

size of 32 bytes for accessing external storage.

For details, see ESP32 Technical Reference Manual > Chapter System and Memory > Section Cache.

4.2 System Clocks

4.2.1 CPU Clock

Upon reset, an external crystal clock source is selected as the default CPU clock. The external crystal clock

source also connects to a PLL to generate a high-frequency clock (typically 160 MHz).

In addition, ESP32 has an internal 8 MHz oscillator. The application can select the clock source from the

external crystal clock source, the PLL clock or the internal 8 MHz oscillator. The selected clock source drives

the CPU clock directly, or after division, depending on the application.

4.2.2 RTC Clock

The RTC clock has five possible sources:

• External low-speed (32 kHz) crystal clock

• External crystal clock divided by 4

• Internal RC oscillator (typically about 150 kHz, and adjustable)

• Internal 8 MHz oscillator

• Internal 31.25 kHz clock (derived from the internal 8 MHz oscillator divided by 256)

When the chip is in the normal power mode and needs faster CPU accessing, the application can choose the

external high-speed crystal clock divided by 4 or the internal 8 MHz oscillator. When the chip operates in the

low-power mode, the application chooses the external low-speed (32 kHz) crystal clock, the internal RC clock

or the internal 31.25 kHz clock.

Espressif Systems

29
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

4 Functional Description

4.2.3 Audio PLL Clock

The audio clock is generated by the ultra-low-noise fractional-N PLL.

For details, see ESP32 Technical Reference Manual > Chapter Reset and Clock.

4.3 RTC and Low-power Management

4.3.1 Power Management Unit (PMU)

With the use of advanced power-management technologies, ESP32 can switch between different power

modes.

• Power modes

– Active mode: The chip radio is powered up. The chip can receive, transmit, or listen.

– Modem-sleep mode: The CPU is operational and the clock is configurable. The Wi-Fi/Bluetooth

baseband and radio are disabled.

– Light-sleep mode: The CPU is paused. The RTC memory and RTC peripherals, as well as the ULP

coprocessor are running. Any wake-up events (MAC, SDIO host, RTC timer, or external interrupts)

will wake up the chip.

– Deep-sleep mode: Only the RTC memory and RTC peripherals are powered up. Wi-Fi and Bluetooth

connection data are stored in the RTC memory. The ULP coprocessor is functional.

– Hibernation mode: The internal 8 MHz oscillator and ULP coprocessor are disabled. The RTC

recovery memory is powered down. Only one RTC timer on the slow clock and certain RTC GPIOs

are active. The RTC timer or the RTC GPIOs can wake up the chip from the Hibernation mode.

Table 4-2. Power Consumption by Power Modes

Power mode

Active (RF working)

Description

Wi-Fi Tx packet

Wi-Fi/BT Tx packet

Wi-Fi/BT Rx and listening

Power Consumption

Please refer to

Table 5-4 for details.

Modem-sleep

The CPU is

powered up.

Light-sleep

Deep-sleep

Hibernation

Power off

*

*

240 MHz

160 MHz

Normal speed: 80 MHz

-

Dual-core chip(s)

30 mA ~ 68 mA

Single-core chip(s)

N/A

Dual-core chip(s)

27 mA ~ 44 mA

Single-core chip(s)

27 mA ~ 34 mA

Dual-core chip(s)

20 mA ~ 31 mA

Single-core chip(s)

20 mA ~ 25 mA

The ULP coprocessor is powered up.

ULP sensor-monitored pattern

RTC timer + RTC memory

RTC timer only

CHIP_PU is set to low level, the chip is powered down.

0.8 mA

150 µA
100 µA @1% duty
10 µA

5 µA

1 µA

• * Among the ESP32 series of SoCs, ESP32-D0WD-V3, ESP32-D0WDRH2-V3, ESP32-U4WDH, ESP32-D0WD

(NRND), ESP32-D0WDQ6 (NRND), and ESP32-D0WDQ6-V3 (NRND) have a maximum CPU frequency of 240 MHz,

Espressif Systems

30
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

4 Functional Description

ESP32-S0WD (NRND) has a maximum CPU frequency of 160 MHz.

• When Wi-Fi is enabled, the chip switches between Active and Modem-sleep modes. Therefore, power consumption

changes accordingly.

• In Modem-sleep mode, the CPU frequency changes automatically. The frequency depends on the CPU load and

the peripherals used.

• During Deep-sleep, when the ULP coprocessor is powered on, peripherals such as GPIO and RTC I2C are able to

operate.

• When the system works in the ULP sensor-monitored pattern, the ULP coprocessor works with the ULP sensor

periodically and the ADC works with a duty cycle of 1%, so the power consumption is 100 µA.

4.3.2 Ultra-Low-Power Coprocessor

The ULP coprocessor and RTC memory remain powered on during the Deep-sleep mode. Hence, the

developer can store a program for the ULP coprocessor in the RTC slow memory to access the peripheral

devices, internal timers and internal sensors during the Deep-sleep mode. This is useful for designing

applications where the CPU needs to be woken up by an external event, or a timer, or a combination of the

two, while maintaining minimal power consumption.

For details, see ESP32 Technical Reference Manual > Chapter ULP Coprocessor.

4.4 Timers and Watchdogs

4.4.1 General Purpose Timers

There are four general-purpose timers embedded in the chip. They are all 64-bit generic timers which are

based on 16-bit prescalers and 64-bit auto-reload-capable up/down-timers.

The timers feature:

• A 16-bit clock prescaler, from 2 to 65536

• A 64-bit timer

• Configurable up/down timer: incrementing or decrementing

• Halt and resume of time-base counter

• Auto-reload at alarming

• Software-controlled instant reload

• Level and edge interrupt generation

For details, see ESP32 Technical Reference Manual > Chapter Timer Group.

4.4.2 Watchdog Timers

The chip has three watchdog timers: one in each of the two timer modules (called the Main Watchdog Timer,

or MWDT) and one in the RTC module (called the RTC Watchdog Timer, or RWDT). These watchdog timers are

intended to recover from an unforeseen fault causing the application program to abandon its normal

sequence. A watchdog timer has four stages. Each stage may trigger one of three or four possible actions

upon the expiry of its programmed time period, unless the watchdog is fed or disabled. The actions are:

Espressif Systems

31
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

4 Functional Description

interrupt, CPU reset, core reset, and system reset. Only the RWDT can trigger the system reset, and is able to

reset the entire chip, including the RTC itself. A timeout value can be set for each stage individually.

During flash boot the RWDT and the first MWDT start automatically in order to detect, and recover from,

booting problems.

The watchdogs have the following features:

• Four stages, each of which can be configured or disabled separately

• A programmable time period for each stage

• One of three or four possible actions (interrupt, CPU reset, core reset, and system reset) upon the expiry

of each stage

• 32-bit expiry counter

• Write protection that prevents the RWDT and MWDT configuration from being inadvertently altered

• SPI flash boot protection

If the boot process from an SPI flash does not complete within a predetermined time period, the

watchdog will reboot the entire system.

For details, see ESP32 Technical Reference Manual > Chapter Watchdog Timers.

4.5 Cryptographic Hardware Accelerators

ESP32 is equipped with hardware accelerators of general algorithms, such as AES (FIPS PUB 197), SHA (FIPS

PUB 180-4), and RSA. The chip also supports independent arithmetic, such as large-number modular

multiplication and large-number multiplication. The maximum operation length for RSA, large-number modular

multiplication, and large-number multiplication is 4096 bits.

The hardware accelerators greatly improve operation speed and reduce software complexity. They also

support code encryption and dynamic decryption, which ensures that code in the flash will not be

hacked.

4.6 Radio and Wi-Fi

The radio module consists of the following blocks:

• 2.4 GHz receiver

• 2.4 GHz transmitter

• Bias and regulators

• Balun and transmit-receive switch

• Clock generator

4.6.1 2.4 GHz Receiver

The 2.4 GHz receiver demodulates the 2.4 GHz RF signal to quadrature baseband signals and converts them

to the digital domain with two high-resolution, high-speed ADCs. To adapt to varying signal channel

conditions, RF filters, Automatic Gain Control (AGC), DC offset cancelation circuits and baseband filters are

integrated in the chip.

Espressif Systems

32
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

4 Functional Description

4.6.2 2.4 GHz Transmitter

The 2.4 GHz transmitter modulates the quadrature baseband signals to the 2.4 GHz RF signal, and drives the

antenna with a high-powered Complementary Metal Oxide Semiconductor (CMOS) power amplifier. The use of

digital calibration further improves the linearity of the power amplifier, enabling state-of-the-art performance in

delivering up to +20.5 dBm of power for an 802.11b transmission and +18 dBm for an 802.11n transmission.

Additional calibrations are integrated to cancel any radio imperfections, such as:

• Carrier leakage

• I/Q phase matching

• Baseband nonlinearities

• RF nonlinearities

• Antenna matching

These built-in calibration routines reduce the amount of time required for product testing, and render the

testing equipment unnecessary.

4.6.3 Clock Generator

The clock generator produces quadrature clock signals of 2.4 GHz for both the receiver and the transmitter. All

components of the clock generator are integrated into the chip, including all inductors, varactors, filters,

regulators and dividers.

The clock generator has built-in calibration and self-test circuits. Quadrature clock phases and phase noise

are optimized on-chip with patented calibration algorithms which ensure the best performance of the receiver

and the transmitter.

4.6.4 Wi-Fi Radio and Baseband

ESP32 implements a TCP/IP and full 802.11 b/g/n Wi-Fi MAC protocol. It supports the Basic Service Set (BSS)

STA and SoftAP operations under the Distributed Control Function (DCF). Power management is handled with

minimal host interaction to minimize the active-duty period.

The ESP32 Wi-Fi Radio and Baseband support the following features:

• 802.11b/g/n

• 802.11n MCS0-7 in both 20 MHz and 40 MHz bandwidth

• 802.11n MCS32 (RX)

• 802.11n 0.4 µs guard-interval

• up to 150 Mbps of data rate

• Receiving STBC 2×1

• Up to 20.5 dBm of transmitting power

• Adjustable transmitting power

• Antenna diversity

ESP32 supports antenna diversity with an external RF switch. One or more GPIOs control the RF switch

and selects the best antenna to minimize the effects of channel fading.

Espressif Systems

33
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

4 Functional Description

4.6.5 Wi-Fi MAC

The ESP32 Wi-Fi MAC applies low-level protocol functions automatically. They are as follows:

• Four virtual Wi-Fi interfaces

• Simultaneous Infrastructure BSS Station mode/SoftAP mode/Promiscuous mode

• RTS protection, CTS protection, Immediate Block ACK

• Defragmentation

• TX/RX A-MPDU, RX A-MSDU

• TXOP

• WMM

• CCMP (CBC-MAC, counter mode), TKIP (MIC, RC4), WAPI (SMS4), WEP (RC4) and CRC

• Automatic beacon monitoring (hardware TSF)

4.7 Bluetooth

The chip integrates a Bluetooth link controller and Bluetooth baseband, which carry out the baseband

protocols and other low-level link routines, such as modulation/demodulation, packet processing, bit stream

processing, frequency hopping, etc.

4.7.1 Bluetooth Radio and Baseband

The Bluetooth Radio and Baseband support the following features:

• Class-1, class-2 and class-3 transmit output powers, and a dynamic control range of up to 21 dB

• π/4 DQPSK and 8 DPSK modulation

• High performance in NZIF receiver sensitivity with a minimum sensitivity of -94 dBm

• Class-1 operation without external PA

• Internal SRAM allows full-speed data-transfer, mixed voice and data, and full piconet operation

• Logic for forward error correction, header error control, access code correlation, CRC, demodulation,

encryption bit stream generation, whitening and transmit pulse shaping

• ACL, SCO, eSCO, and AFH

• A-law, µ-law, and CVSD digital audio CODEC in PCM interface

• SBC audio CODEC

• Power management for low-power applications

• SMP with 128-bit AES

4.7.2 Bluetooth Interface

• Provides UART HCI interface, up to 4 Mbps

• Provides SDIO/SPI HCI interface

Espressif Systems

34
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

4 Functional Description

• Provides PCM/I2S audio interface

4.7.3 Bluetooth Stack

The Bluetooth stack of the chip is compliant with the Bluetooth v4.2 BR/EDR and Bluetooth LE

specifications.

4.7.4 Bluetooth Link Controller

The link controller operates in three major states: standby, connection and sniff. It enables multiple

connections, and other operations, such as inquiry, page, and secure simple-pairing, and therefore enables

Piconet and Scatternet. Below are the features:

• Classic Bluetooth

– Device Discovery (inquiry, and inquiry scan)

– Connection establishment (page, and page scan)

– Multi-connections

– Asynchronous data reception and transmission

– Synchronous links (SCO/eSCO)

– Master/Slave Switch

– Adaptive Frequency Hopping and Channel assessment

– Broadcast encryption

– Authentication and encryption

– Secure Simple-Pairing

– Multi-point and scatternet management

– Sniff mode

– Connectionless Slave Broadcast (transmitter and receiver)

– Enhanced Power Control

– Ping

• Bluetooth Low Energy

– Advertising

– Scanning

– Simultaneous advertising and scanning

– Multiple connections

– Asynchronous data reception and transmission

– Adaptive Frequency Hopping and Channel assessment

– Connection parameter update

– Data Length Extension

Espressif Systems

35
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

4 Functional Description

– Link Layer Encryption

– LE Ping

Espressif Systems

36
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

4 Functional Description

4.8 Digital Peripherals

4.8.1 General Purpose Input / Output Interface (GPIO)

ESP32 has 34 GPIO pins which can be assigned various functions by programming the appropriate registers.

There are several kinds of GPIOs: digital-only, analog-enabled, capacitive-touch-enabled, etc. Analog-enabled

GPIOs and Capacitive-touch-enabled GPIOs can be configured as digital GPIOs.

Most of the digital GPIOs can be configured as internal pull-up or pull-down, or set to high impedance. When

configured as an input, the input value can be read through the register. The input can also be set to

edge-trigger or level-trigger to generate CPU interrupts. Most of the digital IO pins are bi-directional,

non-inverting and tristate, including input and output buffers with tristate control. These pins can be

multiplexed with other functions, such as the SDIO, UART, SPI, etc. (More details can be found in the

Appendix, Table IO_MUX. ) For low-power operations, the GPIOs can be set to hold their states.

For details, see Section 4.10 Peripheral Pin Configurations, Appendix A –ESP32 Pin Lists and

ESP32 Technical Reference Manual > Chapter IO_MUX and GPIO Matrix.

4.8.2 Serial Peripheral Interface (SPI)

ESP32 integrates four SPI controllers which can be used to communicate with external devices that use the

SPI protocol. Controller SPI0 is used as a buffer for accessing external memory. Controller SPI1 can be used

as a master. Controllers SPI2 and SPI3 can be configured as either a master or a slave.

SPI1, SPI2, and SPI3 use signal buses prefixed with SPI, HSPI, and VSPI, respectively.

Features of General Purpose SPI (GP-SPI)

• Programmable data transfer length, in multiples of 1 byte

• Four-line full-duplex/half-duplex communication and three-line half-duplex communication support

• Master mode and slave mode

• Programmable CPOL and CPHA

• Programmable clock

For details, see ESP32 Technical Reference Manual > Chapter SPI Controller.

Pin Assignment

For SPI, the pins are multiplexed with GPIO6 ~ GPIO11 via the IO MUX. For HSPI, the pins are multiplexed with

GPIO2, GPIO4, GPIO12 ~ GPIO15 via the IO MUX. For VSPI, the pins are multiplexed with GPIO5, GPIO18 ~

GPIO19, GPIO21 ~ GPIO23 via the IO MUX.

For more information about the pin assignment, see Section 4.10 Peripheral Pin Configurations and

ESP32 Technical Reference Manual > Chapter IO_MUX and GPIO Matrix.

4.8.3 Universal Asynchronous Receiver Transmitter (UART)

The UART in the ESP32 chip facilitates the transmission and reception of asynchronous serial data between

the chip and external UART devices. It consists of two UARTs in the main system, and one low-power LP

UART.

Espressif Systems

37
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

4 Functional Description

Feature List

• Programmable baud rates up to 5 MBaud

• RAM shared by TX FIFOs and RX FIFOs

• Supports input baud rate self-check

• Support for various lengths of data bits and stop bits

• Parity bit support

• Asynchronous communication (RS232 and RS485) and IrDA support

• Supports DMA to communicate data in high speed

• Supports UART wake-up

• Supports both software and hardware flow control

For details, see ESP32 Technical Reference Manual > Chapter UART Controller.

Pin Assignment

The pins for UART can be chosen from any GPIOs via the GPIO Matrix.

For more information about the pin assignment, see Section 4.10 Peripheral Pin Configurations and

ESP32 Technical Reference Manual > Chapter IO_MUX and GPIO Matrix.

4.8.4 I2C Interface

ESP32 has two I2C bus interfaces which can serve as I2C master or slave, depending on the user’s

configuration.

Feature List

• Two I2C controllers: one in the main system and one in the low-power system

• Standard mode (100 Kbit/s)

• Fast mode (400 Kbit/s)

• Up to 5 MHz, yet constrained by SDA pull-up strength

• Support for 7-bit and 10-bit addressing, as well as dual address mode

• Supports continuous data transmission with disabled Serial Clock Line (SCL)

• Supports programmable digital noise filter

Users can program command registers to control I2C interfaces, so that they have more flexibility.

For details, see ESP32 Technical Reference Manual > Chapter I2C Controller.

Pin Assignment

For regular I2C, the pins used can be chosen from any GPIOs via the GPIO Matrix.

For more information about the pin assignment, see Section 4.10 Peripheral Pin Configurations and

ESP32 Technical Reference Manual > Chapter IO_MUX and GPIO Matrix.

Espressif Systems

38
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

4 Functional Description

4.8.5 I2S Interface

The I2S Controller in the ESP32 chip provides a flexible communication interface for streaming digital data in

multimedia applications, particularly digital audio applications.

Feature List

• Master mode and slave mode

• Full-duplex and half-duplex communications

• A variety of audio standards supported

• Configurable high-precision output clock

• Supports PDM signal input and output

• Configurable data transmit and receive modes

For details, see ESP32 Technical Reference Manual > Chapter I2S Controller.

Pin Assignment

The pins for the I2S Controller can be chosen from any GPIOs via the GPIO Matrix.

For more information about the pin assignment, see Section 4.10 Peripheral Pin Configurations and

ESP32 Technical Reference Manual > Chapter IO_MUX and GPIO Matrix.

4.8.6 Remote Control Peripheral

The Remote Control Peripheral (RMT) controls the transmission and reception of infrared remote control

signals.

Feature List

• Eight channels for sending and receiving infrared remote control signals

• Independent transmission and reception capabilities for each channel

• Clock divider counter, state machine, and receiver for each RX channel

• Supports various infrared protocols

For details, see ESP32 Technical Reference Manual > Chapter Remote Control Peripheral.

Pin Assignment

The pins for the Remote Control Peripheral can be chosen from any GPIOs via the GPIO Matrix.

For more information about the pin assignment, see Section 4.10 Peripheral Pin Configurations and

ESP32 Technical Reference Manual > Chapter IO_MUX and GPIO Matrix.

4.8.7 Pulse Counter Controller (PCNT)

The pulse counter controller (PCNT) is designed to count input pulses by tracking rising and falling edges of

the input pulse signal.

Espressif Systems

39
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

4 Functional Description

Feature List

• Eight independent pulse counter units

• Each pulse counter unit has a 16-bit signed counter register and two channels

• Counter modes: increment, decrement, or disable

• Glitch filtering for input pulse signals and control signals

• Selection between counting on rising or falling edges of the input pulse signal

For details, see ESP32 Technical Reference Manual > Chapter Pulse Count Controller.

Pin Assignment

The pins for the Pulse Count Controller can be chosen from any GPIOs via the GPIO Matrix.

For more information about the pin assignment, see Section 4.10 Peripheral Pin Configurations and

ESP32 Technical Reference Manual > Chapter IO_MUX and GPIO Matrix.

4.8.8 LED PWM Controller

The LED PWM Controller (LEDC) is designed to generate PWM signals for LED control.

Feature List

• Sixteen independent PWM generators

• Maximum PWM duty cycle resolution of 20 bits

• Eight independent timers with 20-bit counters, configurable fractional clock dividers and counter

overflow values

• Adjustable phase of PWM signal output

• PWM duty cycle dithering

• Automatic duty cycle fading

For details, see ESP32 Technical Reference Manual > Chapter LED PWM Controller.

Pin Assignment

The pins for the LED PWM Controller can be chosen from any GPIOs via the GPIO Matrix.

For more information about the pin assignment, see Section 4.10 Peripheral Pin Configurations and

ESP32 Technical Reference Manual > Chapter IO_MUX and GPIO Matrix.

4.8.9 Motor Control PWM

The Pulse Width Modulation (PWM) controller can be used for driving digital motors and smart lights. The

controller consists of PWM timers, the PWM operator and a dedicated capture sub-module. Each timer

provides timing in synchronous or independent form, and each PWM operator generates a waveform for one

PWM channel. The dedicated capture sub-module can accurately capture events with external timing.

Espressif Systems

40
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

4 Functional Description

Feature List

• Three PWM timers for precise timing and frequency control

– Every PWM timer has a dedicated 8-bit clock prescaler

– The 16-bit counter in the PWM timer can work in count-up mode, count-down mode, or

count-up-down mode

– A hardware sync can trigger a reload on the PWM timer with a phase register. It will also trigger the

prescaler’restart, so that the timer’s clock can also be synced, with selectable hardware

synchronization source

• Three PWM operators for generating waveform pairs

– Six PWM outputs to operate in several topologies

– Configurable dead time on rising and falling edges; each set up independently

– Modulating of PWM output by high-frequency carrier signals, useful when gate drivers are insulated

with a transformer

• Fault Detection module

– Programmable fault handling in both cycle-by-cycle mode and one-shot mode

– A fault condition can force the PWM output to either high or low logic levels

• Capture module for hardware-based signal processing

– Speed measurement of rotating machinery

– Measurement of elapsed time between position sensor pulses

– Period and duty cycle measurement of pulse train signals

– Decoding current or voltage amplitude derived from duty-cycle-encoded signals of current/voltage

sensors

– Three individual capture channels, each of which with a 32-bit time-stamp register

– Selection of edge polarity and prescaling of input capture signals

– The capture timer can sync with a PWM timer or external signals

For details, see ESP32 Technical Reference Manual > Chapter Motor Control PWM.

Pin Assignment

The pins for the Motor Control PWM can be chosen from any GPIOs via the GPIO Matrix.

For more information about the pin assignment, see Section 4.10 Peripheral Pin Configurations and

ESP32 Technical Reference Manual > Chapter IO_MUX and GPIO Matrix.

4.8.10 SD/SDIO/MMC Host Controller

An SD/SDIO/MMC host controller is available on ESP32.

Espressif Systems

41
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

4 Functional Description

Feature List

• Supports two external cards

• Supports SD Memory Card standard: version 3.0 and version 3.01)

• Supports SDIO Version 3.0

• Supports Consumer Electronics Advanced Transport Architecture (CE-ATA Version 1.1)

• Supports Multimedia Cards (MMC version 4.41, eMMC version 4.5 and version 4.51)

The controller allows up to 80 MHz clock output in three different data-bus modes: 1-bit, 4-bit, and 8-bit

modes. It supports two SD/SDIO/MMC4.41 cards in a 4-bit data-bus mode. It also supports one SD card

operating at 1.8 V.

For details, see ESP32 Technical Reference Manual > Chapter SD/MMC Host Controller.

Pin Assignment

The pins for SD/SDIO/MMC Host Controller are multiplexed with GPIO2, GPIO4, GPIO6 ~ GPIO15 via IO

MUX.

For more information about the pin assignment, see Section 4.10 Peripheral Pin Configurations and

ESP32 Technical Reference Manual > Chapter IO_MUX and GPIO Matrix.

4.8.11 SDIO/SPI Slave Controller

ESP32 integrates an SD device interface that conforms to the industry-standard SDIO Card Specification

Version 2.0, and allows a host controller to access the SoC, using the SDIO bus interface and protocol. ESP32

acts as the slave on the SDIO bus. The host can access the SDIO-interface registers directly and can access

shared memory via a DMA engine, thus maximizing performance without engaging the processor cores.

Feature List

The SDIO/SPI slave controller supports the following features:

• SPI, 1-bit SDIO, and 4-bit SDIO transfer modes over the full clock range from 0 to 50 MHz

• Configurable sampling and driving clock edge

• Special registers for direct access by host

• Interrupts to host for initiating data transfer

• Automatic loading of SDIO bus data and automatic discarding of padding data

• Block size of up to 512 bytes

• Interrupt vectors between the host and the slave, allowing both to interrupt each other

• Supports DMA for data transfer

For details, see ESP32 Technical Reference Manual > Chapter SDIO Slave Controller.

Espressif Systems

42
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

4 Functional Description

Pin Assignment

The pins for SDIO/SPI Slave Controller are multiplexed with GPIO2, GPIO4, GPIO6 ~ GPIO15 via IO MUX.

For more information about the pin assignment, see Section 4.10 Peripheral Pin Configurations and

ESP32 Technical Reference Manual > Chapter IO_MUX and GPIO Matrix.

4.8.12 TWAI® Controller

The Two-wire Automotive Interface (TWAI®) is a multi-master, multi-cast communication protocol designed for
automotive applications. The TWAI controller facilitates the communication based on this protocol.

Feature List

• Compatible with ISO 11898-1 protocol (CAN Specification 2.0)

• Standard frame format (11-bit ID) and extended frame format (29-bit ID)

• Bit rates:

– From 25 Kbit/s to 1 Mbit/s in chip revision v0.0/v1.0/v1.1

– From 12.5 Kbit/s to 1 Mbit/s in chip revision v3.0/v3.1

• Multiple modes of operation: Normal, Listen Only, and Self-Test

• 64-byte receive FIFO

• Special transmissions: single-shot transmissions and self reception

• Acceptance filter (single and dual filter modes)

• Error detection and handling: error counters, configurable error interrupt threshold, error code capture,

arbitration lost capture

For details, see ESP32 Technical Reference Manual > Chapter Two-wire Automotive Interface (TWAI).

Pin Assignment

The pins for the Two-wire Automotive Interface can be chosen from any GPIOs via the GPIO Matrix.

For more information about the pin assignment, see Section 4.10 Peripheral Pin Configurations and

ESP32 Technical Reference Manual > Chapter IO_MUX and GPIO Matrix.

4.8.13 Ethernet MAC Interface

An IEEE-802.3-2008-compliant Media Access Controller (MAC) is provided for Ethernet LAN communications.

ESP32 requires an external physical interface device (PHY) to connect to the physical LAN bus (twisted-pair,

fiber, etc.). The PHY is connected to ESP32 through 17 signals of MII or nine signals of RMII.

Feature List

• 10 Mbps and 100 Mbps rates

• Dedicated DMA controller allowing high-speed transfer between the dedicated SRAM and Ethernet MAC

• Tagged MAC frame (VLAN support)

Espressif Systems

43
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

4 Functional Description

• Half-duplex (CSMA/CD) and full-duplex operation

• MAC control sublayer (control frames)

• 32-bit CRC generation and removal

• Several address-filtering modes for physical and multicast address (multicast and group addresses)

• 32-bit status code for each transmitted or received frame

• Internal FIFOs to buffer transmit and receive frames. The transmit FIFO and the receive FIFO are both 512

words (32-bit)

• Hardware PTP (Precision Time Protocol) in accordance with IEEE 1588 2008 (PTP V2)

• 25 MHz/50 MHz clock output

For details, see ESP32 Technical Reference Manual > Chapter Ethernet Media Access Controller (MAC).

Pin Assignment

For information about the pin assignment of Ethernet MAC Interface, see Section 4.10 Peripheral Pin

Configurations and ESP32 Technical Reference Manual > Chapter IO_MUX and GPIO Matrix.

4.9 Analog Peripherals

4.9.1 Analog-to-Digital Converter (ADC)

ESP32 integrates two 12-bit SAR ADCs and supports measurements on 18 channels (analog-enabled pins).

The ULP coprocessor in ESP32 is also designed to measure voltage, while operating in the sleep mode, which

enables low-power consumption. The CPU can be woken up by a threshold setting and/or via other

triggers.

Table 4-3 describes the ADC characteristics.

Table 4-3. ADC Characteristics

Parameter

Description

DNL (Differential nonlinearity)

INL (Integral nonlinearity)

Sampling rate

RTC controller; ADC connected to an

external 100 nF capacitor; DC signal input;

ambient temperature at 25 °C;

Wi-Fi&Bluetooth off

RTC controller

DIG controller

Min Max

Unit

–7

7

LSB

–12

12

LSB

— 200

ksps

—

2 Msps

Notes:

• When atten = 3 and the measurement result is above 3000 (voltage at approx. 2450 mV), the ADC

accuracy will be worse than described in the table above.

• To get better DNL results, users can take multiple sampling tests with a filter, or calculate the average

value.

Espressif Systems

44
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

4 Functional Description

• The input voltage range of GPIO pins within VDD3P3_RTC domain should strictly follow the DC

characteristics provided in Table 5-3. Otherwise, measurement errors may be introduced, and chip

performance may be affected.

By default, there are ±6% differences in measured results between chips. ESP-IDF provides couple of

calibration methods for ADC1. Results after calibration using eFuse Vref value are shown in Table 4-4. For

higher accuracy, users may apply other calibration methods provided in ESP-IDF, or implement their

own.

Table 4-4. ADC Calibration Results

Parameter

Total error

Description
Atten = 0, effective measurement range of 100 ∼ 950 mV
Atten = 1, effective measurement range of 100 ∼ 1250 mV
Atten = 2, effective measurement range of 150 ∼ 1750 mV
Atten = 3, effective measurement range of 150 ∼ 2450 mV

Min Max Unit

–23
–30
–40
–60

23 mV

30 mV

40 mV

60 mV

For details, see ESP32 Technical Reference Manual > Chapter On-Chip Sensors and Analog Signal

Processing.

Pin Assignment

With appropriate settings, the ADCs can be configured to measure voltage on 18 pins maximum. For detailed

information about the pin assignment, see Section 4.10 Peripheral Pin Configurations and

ESP32 Technical Reference Manual > Chapter IO_MUX and GPIO Matrix.

4.9.2 Digital-to-Analog Converter (DAC)

Two 8-bit DAC channels can be used to convert two digital signals into two analog voltage signal outputs. The

design structure is composed of integrated resistor strings and a buffer. This dual DAC supports power supply

as input voltage reference. The two DAC channels can also support independent conversions.

For details, see ESP32 Technical Reference Manual > Chapter On-Chip Sensors and Analog Signal

Processing.

Pin Assignment

The DAC can be configured by GPIO 25 and GPIO 26. For detailed information about the pin assignment, see

Section 4.10 Peripheral Pin Configurations and ESP32 Technical Reference Manual > Chapter IO_MUX and

GPIO Matrix.

4.9.3 Touch Sensor

ESP32 has 10 capacitive-sensing GPIOs, which detect variations induced by touching or approaching the

GPIOs with a finger or other objects. The low-noise nature of the design and the high sensitivity of the circuit

allow relatively small pads to be used. Arrays of pads can also be used, so that a larger area or more points

can be detected.

Espressif Systems

45
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

4 Functional Description

Pin Assignment

The 10 capacitive-sensing GPIOs are listed in Table 4-5.

Table 4-5. Capacitive-Sensing GPIOs Available on ESP32

Capacitive-Sensing Signal Name

Pin Name

T0

T1

T2

T3

T4

T5

T6

T7

T8

T9

GPIO4

GPIO0

GPIO2

MTDO

MTCK

MTDI

MTMS

GPIO27

32K_XN

32K_XP

For details, see ESP32 Technical Reference Manual > Chapter On-Chip Sensors and Analog Signal

Processing.

Note:

ESP32 Touch Sensor has not passed the Conducted Susceptibility (CS) test for now, and thus has limited application

scenarios.

Espressif Systems

46
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

4 Functional Description

4.10 Peripheral Pin Configurations

Interface

ADC

DAC

Touch Sensor

JTAG

Signal

ADC1_CH0

ADC1_CH1

ADC1_CH2

ADC1_CH3

ADC1_CH4

ADC1_CH5

ADC1_CH6

ADC1_CH7

ADC2_CH0

ADC2_CH1

ADC2_CH2

ADC2_CH3

ADC2_CH4

ADC2_CH5

ADC2_CH6

ADC2_CH7

ADC2_CH8

ADC2_CH9

DAC_1

DAC_2

TOUCH0

TOUCH1

TOUCH2

TOUCH3

TOUCH4

TOUCH5

TOUCH6

TOUCH7

TOUCH8

TOUCH9

MTDI

MTCK

MTMS

MTDO

Table 4-6. Peripheral Pin Configurations

Pin

Function

SENSOR_VP

SENSOR_CAPP

SENSOR_CAPN

SENSOR_VN

32K_XP

32K_XN

VDET_1

VDET_2

GPIO4

GPIO0

GPIO2

MTDO

MTCK

MTDI

MTMS

GPIO27

GPIO25

GPIO26

GPIO25

GPIO26

GPIO4

GPIO0

GPIO2

MTDO

MTCK

MTDI

MTMS

GPIO27

32K_XN

32K_XP

MTDI

MTCK

MTMS

MTDO

Two 12-bit SAR ADCs

Two 8-bit DACs

Capacitive touch sensors

JTAG for software debugging

Espressif Systems

47
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

4 Functional Description

Interface

Signal

HS2_CLK

HS2_CMD

SD/SDIO/MMC Host

HS2_DATA0

Controller

Motor PWM

SDIO/SPI Slave

Controller

UART

I2C

HS2_DATA1

HS2_DATA2

HS2_DATA3

PWM0_OUT0~2

PWM1_OUT_IN0~2

PWM0_FLT_IN0~2

PWM1_FLT_IN0~2

PWM0_CAP_IN0~2

PWM1_CAP_IN0~2

PWM0_SYNC_IN0~2

PWM1_SYNC_IN0~2

SD_CLK

SD_CMD

SD_DATA0

SD_DATA1

SD_DATA2

SD_DATA3

U0RXD_in

U0CTS_in

U0DSR_in

U0TXD_out

U0RTS_out

U0DTR_out

U1RXD_in

U1CTS_in

U1TXD_out

U1RTS_out

U2RXD_in

U2CTS_in

U2TXD_out

U2RTS_out

I2CEXT0_SCL_in

I2CEXT0_SDA_in

I2CEXT1_SCL_in

I2CEXT1_SDA_in

I2CEXT0_SCL_out

I2CEXT0_SDA_out

I2CEXT1_SCL_out

I2CEXT1_SDA_out

Function

Supports SD memory card V3.01 standard

Pin

MTMS

MTDO

GPIO2

GPIO4

MTDI

MTCK

Three channels of 16-bit timers generate

PWM waveforms. Each channel has a pair

Any GPIO Pins

of output signals, three fault detection

signals, three event-capture signals, and

three sync signals.

MTMS

MTDO

GPIO2

GPIO4

MTDI

MTCK

SDIO interface that conforms to the

industry standard SDIO 2.0 card

specification

Any GPIO Pins

Three UART devices with hardware

flow-control and DMA

Any GPIO Pins

Two I2C devices in slave or master mode

Espressif Systems

48
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

4 Functional Description

Interface

Signal

Pin

Function

Any GPIO Pins

16 independent channels @80 MHz

clock/RTC CLK. Duty accuracy: 16 bits.

I2S

I2S0O_DATA_out0~23

I2S1I_DATA_in0~15

Any GPIO Pins

LED PWM

ledc_hs_sig_out0~7

ledc_ls_sig_out0~7

I2S0I_DATA_in0~15

I2S0O_BCK_in

I2S0O_WS_in

I2S0I_BCK_in

I2S0I_WS_in

I2S0I_H_SYNC

I2S0I_V_SYNC

I2S0I_H_ENABLE

I2S0O_BCK_out

I2S0O_WS_out

I2S0I_BCK_out

I2S0I_WS_out

I2S1O_BCK_in

I2S1O_WS_in

I2S1I_BCK_in

I2S1I_WS_in

I2S1I_H_SYNC

I2S1I_V_SYNC

I2S1I_H_ENABLE

I2S1O_BCK_out

I2S1O_WS_out

I2S1I_BCK_out

I2S1I_WS_out

I2S1O_DATA_out0~23

I2S0_CLK

I2S1_CLK

RMT_SIG_IN0~7

RMT_SIG_OUT0~7

HSPIQ_in/_out

HSPID_in/_out

HSPICLK_in/_out

HSPI_CS0_in/_out

HSPI_CS1_out

RMT

General Purpose

HSPI_CS2_out

SPI

VSPIQ_in/_out

VSPID_in/_out

VSPICLK_in/_out

VSPI_CS0_in/_out

VSPI_CS1_out

VSPI_CS2_out

Stereo input and output from/to the audio

codec; parallel LCD data output; parallel

camera data input.

Note: I2S0_CLK and I2S1_CLK can only

be mapped to GPIO0, U0RXD (GPIO3), or

U0TXD (GPIO1) via IO MUX by selecting

GPIO functions CLK_OUT1, CLK_OUT2,

and CLK_OUT3. For more information,

see ESP32 Technical Reference Manual >

Chapter IO_MUX and GPIO Matrix > Table

IO MUX Pad Summary.

GPIO0, U0RXD,

or U0TXD

Any GPIO Pins

Any GPIO Pins

Eight channels for an IR transmitter and

receiver of various waveforms

Standard SPI consists of clock,

chip-select, MOSI and MISO. These SPIs

can be connected to LCD and other

external devices. They support the

following features:

• Both master and slave modes;
• Four sub-modes of the SPI transfer

format;

• Configurable SPI frequency;
• Up to 64 bytes of FIFO and DMA.

Espressif Systems

49
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

4 Functional Description

Interface

Parallel QSPI

EMAC

Signal

SPIHD

SPIWP

SPICS0

SPICLK

SPIQ

SPID

HSPICLK

HSPICS0

HSPIQ

HSPID

HSPIHD

HSPIWP

VSPICLK

VSPICS0

VSPIQ

VSPID

VSPIHD

VSPIWP

EMAC_TX_CLK

EMAC_RX_CLK

EMAC_TX_EN

EMAC_TXD0

EMAC_TXD1

EMAC_TXD2

EMAC_TXD3

EMAC_RX_ER

EMAC_RX_DV

EMAC_RXD0

EMAC_RXD1

EMAC_RXD2

EMAC_RXD3

EMAC_CLK_OUT

Pin

Function

SD_DATA_2

SD_DATA_3

SD_CMD

SD_CLK

SD_DATA_0

SD_DATA_1

MTMS

MTDO

MTDI

MTCK

GPIO4

GPIO2

GPIO18

GPIO5

GPIO19

GPIO23

GPIO21

GPIO22

GPIO0

GPIO5

GPIO21

GPIO19

GPIO22

MTMS

MTDI

MTCK

GPIO27

GPIO25

GPIO26

U0TXD

MTDO

GPIO16

Supports Standard SPI, Dual SPI, and

Quad SPI that can be connected to the

external flash and SRAM

Ethernet MAC with MII/RMII interface

EMAC_CLK_OUT_180 GPIO17

EMAC_TX_ER

GPIO4

EMAC_MDC_out

Any GPIO Pins

EMAC_MDI_in

Any GPIO Pins

EMAC_MDO_out

Any GPIO Pins

EMAC_CRS_out

EMAC_COL_out

Any GPIO Pins

Any GPIO Pins

Espressif Systems

50
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

4 Functional Description

Interface

Signal

Pin

Function

pcnt_sig_ch0_in0

pcnt_sig_ch1_in0

pcnt_ctrl_ch0_in0

pcnt_ctrl_ch1_in0

pcnt_sig_ch0_in1

pcnt_sig_ch1_in1

pcnt_ctrl_ch0_in1

pcnt_ctrl_ch1_in1

pcnt_sig_ch0_in2

pcnt_sig_ch1_in2

pcnt_ctrl_ch0_in2

pcnt_ctrl_ch1_in2

pcnt_sig_ch0_in3

pcnt_sig_ch1_in3

pcnt_ctrl_ch0_in3

pcnt_ctrl_ch1_in3

pcnt_sig_ch0_in4

pcnt_sig_ch1_in4

pcnt_ctrl_ch0_in4

pcnt_ctrl_ch1_in4

pcnt_sig_ch0_in5

pcnt_sig_ch1_in5

pcnt_ctrl_ch0_in5

pcnt_ctrl_ch1_in5

pcnt_sig_ch0_in6

pcnt_sig_ch1_in6

pcnt_ctrl_ch0_in6

pcnt_ctrl_ch1_in6

pcnt_sig_ch0_in7

pcnt_sig_ch1_in7

pcnt_ctrl_ch0_in7

pcnt_ctrl_ch1_in7

twai_rx

twai_tx

twai_bus_off_on

twai_clkout

Pulse Counter

TWAI

Operating in seven different modes, the

Any GPIO Pins

pulse counter captures pulse and counts

pulse edges.

Any GPIO Pins

Compatible with ISO 11898-1 protocol

(CAN Specification 2.0)

Espressif Systems

51
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

5 Electrical Characteristics

5 Electrical Characteristics

5.1 Absolute Maximum Ratings

Stresses above those listed in Table 5-1 Absolute Maximum Ratings may cause permanent damage to the

device. These are stress ratings only and normal operation of the device at these or any other conditions

beyond those indicated in Section 5.2 Recommended Power Supply Characteristics is not implied. Exposure

to absolute-maximum-rated conditions for extended periods may affect device reliability.

Table 5-1. Absolute Maximum Ratings

Parameter

Description

Min

Max

Unit

VDDA, VDD3P3, VDD3P3_RTC,

VDD3P3_CPU, VDD_SDIO

Ioutput

1

TST ORE

Allowed input voltage

–0.3

3.6

V

Cumulative IO output current

Storage temperature

—

–40

1200

150

mA

°C

1 The product proved to be fully functional after all its IO pins were pulled high while being connected

to ground for 24 consecutive hours at ambient temperature of 25 °C.

5.2 Recommended Power Supply Characteristics

Table 5-2. Recommended Power Supply Characteristics

Parameter

Description

Min

Typ Max Unit

VDDA, VDD3P3_RTC, VDD3P3,
VDD_SDIO (3.3 V mode) note 1

Voltage applied to power supply

pins per power domain

2.3/3.0 note 2

3.3

3.6

VDD3P3_CPU

Voltage applied to power supply pin

1.8

3.3

3.6

IV DD

T note 3

Current delivered by external power

supply

0.5

—

—

Operating temperature

–40

— 125

°C

V

V

A

1.

• VDD_SDIO works as the power supply for the related IO, and also for an external device. Please refer to the

Appendix IO_MUX of this datasheet for more details.

• VDD_SDIO can be sourced internally by the ESP32 from the VDD3P3_RTC power domain:

– When VDD_SDIO operates at 3.3 V, it is driven directly by VDD3P3_RTC through a 6 Ω resistor, therefore,

there will be some voltage drop from VDD3P3_RTC.

– When VDD_SDIO operates at 1.8 V, it can be generated from ESP32’s internal LDO. The maximum current

this LDO can offer is 40 mA, and the output voltage range is 1.65 V ~ 2.0 V.

• VDD_SDIO can also be driven by an external power supply.
• Please refer to Section 2.5.2 Power Scheme, for more information.

2.

• Chips with a 3.3 V flash or PSRAM in-package: this minimum voltage is 3.0 V;
• Chips with no flash or PSRAM in-package: this minimum voltage is 2.3 V;
• For more information, see Section 1 ESP32 Series Comparison.

3. The operating temperature of ESP32-U4WDH and ESP32-D0WDRH2-V3 ranges from –40 °C to 85 °C, due to the

in-package flash or PSRAM. For other chips that have no in-package flash or PSRAM, their operating temperature is

–40 °C ~ 125 °C.

Espressif Systems

52
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

5 Electrical Characteristics

5.3 DC Characteristics (3.3 V, 25 °C)

Table 5-3. DC Characteristics (3.3 V, 25 °C)

Parameter

Description

CIN

VIH

VIL

IIH

IIL

VOH

VOL

IOH

IOL

RP U

RP D

VIH_nRST

VIL_nRST

Pin capacitance

High-level input voltage

Low-level input voltage

High-level input current

Low-level input current

High-level output voltage

Low-level output voltage

High-level source current
(VDD 1 = 3.3 V,
VOH >= 2.64 V,
output drive strength set

to the maximum)

VDD3P3_CPU
power domain 1, 2
VDD3P3_RTC
power domain 1, 2
VDD_SDIO power
domain 1, 3

Low-level sink current
(VDD 1 = 3.3 V, VOL = 0.495 V,
output drive strength set to the maximum)

Resistance of internal pull-up resistor

Resistance of internal pull-down resistor

Chip reset release voltage (CHIP_PU voltage

is within the specified range)

Low-level input voltage of CHIP_PU

to shut down the chip

Min

—
0.75 × VDD 1
–0.3

—

—
0.8 × VDD 1

—

Typ

2

Max

Unit

— pF

VDD 1 + 0.3
—
— 0.25 × VDD 1

—

—

—

—

50

50

—
0.1 × VDD 1

V

V

nA

nA

V

V

— 40

— mA

— 40

— mA

— 20

— mA

—

28

— mA

—

—

0.75 × VDD 1

45

45

—

VDD 1 + 0.3

—

—

0.6

— kΩ

— kΩ

V

V

1. Please see Table IO_MUX for IO’s power domain. VDD is the I/O voltage for a particular power domain of pins.

2. For VDD3P3_CPU and VDD3P3_RTC power domain, per-pin current sourced in the same domain is gradually

reduced from around 40 mA to around 29 mA, VOH >=2.64 V, as the number of current-source pins increases.

3. For VDD_SDIO power domain, per-pin current sourced in the same domain is gradually reduced from around 30 mA

to around 10 mA, VOH >=2.64 V, as the number of current-source pins increases.

5.4 RF Current Consumption in Active Mode

The current consumption measurements are taken with a 3.3 V supply at 25 °C of ambient temperature at the

RF port. All transmitters’ measurements are based on a 50% duty cycle.

Table 5-4. Current Consumption Depending on RF Modes

Work Mode

Transmit 802.11b, DSSS 1 Mbps, POUT = +19.5 dBm

Transmit 802.11g, OFDM 54 Mbps, POUT = +16 dBm

Transmit 802.11n, OFDM MCS7, POUT = +14 dBm

Receive 802.11b/g/n

Min

Typ

Max Unit

—

—

—

240

190

180

— 95 ~ 100

— mA

— mA

— mA

— mA

Espressif Systems

53
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

5 Electrical Characteristics

Work Mode

Transmit BT/BLE, POUT = 0 dBm

Receive BT/BLE

5.5 Reliability

Min

—

Typ

Max Unit

130

— 95 ~ 100

— mA

— mA

Table 5-5. Reliability Qualifications

Test Item

Test Conditions

HTOL (High Temperature

Operating Life)

ESD (Electro-Static

Discharge Sensitivity)

Latch up

125 °C, 1000 hours

HBM (Human Body Mode) 1 ± 2000 V
CDM (Charge Device Mode) 2 ± 500 V

Current trigger ± 200 mA

Voltage trigger 1.5 × VDDmax

Bake 24 hours @125 °C

Test Standard

JESD22-A108

JS-001

JS-002

JESD78

J-STD-020,

Preconditioning

Moisture soak (level 3: 192 hours @30 °C, 60% RH)

JESD47,

IR reflow solder: 260 + 0 °C, 20 seconds, three

JESD22-A113

times

TCT (Temperature Cycling

Test)

–65 °C / 150 °C, 500 cycles

Autoclave Test

121 °C, 100% RH, 96 hours

130 °C, 85% RH, 96 hours

uHAST

(Highly

Accel-

erated

Stress

Test,

unbiased)

HTSL (High Temperature

Storage Life)

JESD22-A104

JESD22-A102

JESD22-A118

150 °C, 1000 hours

JESD22-A103

1. JEDEC document JEP155 states that 500 V HBM allows safe manufacturing with a standard ESD control process.

2. JEDEC document JEP157 states that 250 V CDM allows safe manufacturing with a standard ESD control process.

5.6 Wi-Fi Radio

Table 5-6. Wi-Fi Radio Characteristics

Parameter
Operating frequency range note1 —
Output impedance note2

-

Description

Min

2412

-

12

Typ

Max Unit

— 2484 MHz

note 2

— Ω

18.5

19.5

20.5

13

14

dBm

dBm

—

—

—

—

–98
–88
–93
–75

— dBm

— dBm

— dBm

— dBm

11n, MCS7

11b mode

11b, 1 Mbps

11b, 11 Mbps

11g, 6 Mbps

11g, 54 Mbps

TX power note3

Sensitivity

Espressif Systems

54
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

5 Electrical Characteristics

Parameter

Adjacent channel rejection

Description

11n, HT20, MCS0

11n, HT20, MCS7

11n, HT40, MCS0

11n, HT40, MCS7

11g, 6 Mbps

11g, 54 Mbps

11n, HT20, MCS0

11n, HT20, MCS7

Min

Typ

Max Unit

—

—

—

—

—

—

—

—

–93
–73
–90
–70

27

13

27

12

— dBm

— dBm

— dBm

— dBm

— dB

— dB

— dB

— dB

1. Device should operate in the frequency range allocated by regional regulatory authorities. Target operating

frequency range is configurable by software.

2. The typical value of the Wi-Fi radio output impedance is different between chips in different QFN packages. For

chips in a QFN 6×6 package, the value is 30+j10 Ω. For chips in a QFN 5×5 package, the value is 35+j10 Ω.

3. Target TX power is configurable based on device or certification requirements.

5.7 Bluetooth Radio

5.7.1 Receiver –Basic Data Rate

Table 5-7. Receiver Characteristics –Basic Data Rate

Parameter

Sensitivity @0.1% BER

Maximum received signal @0.1% BER

Co-channel C/I

Adjacent channel selectivity C/I

Out-of-band blocking performance

Description

—

—

—

F = F0 + 1 MHz
F = F0 –1 MHz

F = F0 + 2 MHz
F = F0 –2 MHz

F = F0 + 3 MHz
F = F0 –3 MHz

30 MHz ~ 2000 MHz

2000 MHz ~ 2400 MHz

2500 MHz ~ 3000 MHz

3000 MHz ~ 12.5 GHz

Intermodulation

—

5.7.2 Transmitter –Basic Data Rate

Min

–90

Typ Max

Unit

–89

–88

dBm

0

—

—

—

—

—

—

—

–10
–27
–27
–10

–36

—

+7

— dBm

— dB

—

–6
–6
—
— –25
— –33
— –25
— –45

dB

dB

dB

dB

dB

dB

—

—

—

—

—

— dBm

— dBm

— dBm

— dBm

— dBm

Table 5-8. Transmitter Characteristics –Basic Data Rate

Parameter
RF transmit power note1

Gain control step

Description

Min

Typ Max

—

—

—

—

0

3

—

—

Unit

dBm

dB

Espressif Systems

55
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

5 Electrical Characteristics

Parameter

RF power control range

+20 dB bandwidth

Adjacent channel transmit power

∆ f 1avg
∆ f 2max
∆ f 2avg/∆ f 1avg
ICFT

Drift rate

Drift (DH1)

Drift (DH5)

Description

—

—

F = F0 ± 2 MHz

F = F0 ± 3 MHz

F = F0 ± > 3 MHz

—

—

—

—

—

—

—

Typ Max

Min

–12

—

—

0.9

— –47
— –55
— –60

133.7

—

— 0.92

—

—

—

—

–7

0.7

6

6

Unit

dBm

MHz

dBm

dBm

dBm

kHz

kHz

—

kHz

+9

—

—

—

—

—

—

—

— kHz/50 µs

—

—

kHz

kHz

—

— 155

1. There are in total eight power levels from level 0 to level 7, with transmit power ranging from –12 dBm to 9 dBm.

When the power level rises by 1, the transmit power increases by 3 dB. Power level 4 is used by default and the

corresponding transmit power is 0 dBm.

5.7.3 Receiver –Enhanced Data Rate

Table 5-9. Receiver Characteristics –Enhanced Data Rate

Parameter

Description

Min

Typ Max

Unit

Sensitivity @0.01% BER

Maximum received signal @0.01% BER

Co-channel C/I

π/4 DQPSK

—

—

—

Adjacent channel selectivity C/I

F = F0 + 1 MHz
F = F0 –1 MHz

F = F0 + 2 MHz
F = F0 –2 MHz

F = F0 + 3 MHz
F = F0 –3 MHz

Sensitivity @0.01% BER

Maximum received signal @0.01% BER

C/I c-channel

8DPSK

—

—

—

Adjacent channel selectivity C/I

5.7.4 Transmitter –Enhanced Data Rate

F = F0 + 1 MHz
F = F0 –1 MHz

F = F0 + 2 MHz
F = F0 –2 MHz

F = F0 + 3 MHz
F = F0 –3 MHz

–90

–89

–88

dBm

—

—

0

11

—

–7
–7
—
— –25
— –35
— –25
— –45

— dBm

— dB

— dB

— dB

— dB

— dB

— dB

— dB

–84

–83

–82

dBm

—

—

—

–5

18

2

—
2
— –25
— –25
— –25
— –38

— dBm

— dB

— dB

— dB

— dB

— dB

— dB

— dB

Espressif Systems

56
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

5 Electrical Characteristics

Table 5-10. Transmitter Characteristics –Enhanced Data Rate

Parameter

Description

Min

Typ Max

Unit

RF transmit power (see note under Table 5-10) —

Gain control step

RF power control range

π/4 DQPSK max w0

π/4 DQPSK max wi

π/4 DQPSK max |wi + w0|

8DPSK max w0

8DPSK max wi

8DPSK max |wi + w0|

π/4 DQPSK modulation accuracy

8 DPSK modulation accuracy

In-band spurious emissions

—

—

—

—

—

—

—

—

RMS DEVM

99% DEVM

Peak DEVM

RMS DEVM

99% DEVM

Peak DEVM

F = F0 ± 1 MHz

F = F0 ± 2 MHz

F = F0 ± 3 MHz
F = F0 +/–> 3 MHz

EDR differential phase coding

—

5.8 Bluetooth LE Radio

5.8.1 Receiver

—

—

–12

0

3

—

— dBm

— dB

+9

dBm

— kHz

— kHz

— kHz

— kHz

— kHz

— kHz

— %

— %

— %

— %

— %

— %

— dBm

— dBm

— dBm

— –0.72

—

–6

— –7.42

—

—

—

0.7

–9.6

–10

— 4.28

100

13.3

5.8

100

14

–46
–40
–46

—

—

—

—

—

—

—

—

—

—

— –53

dBm

100

— %

Table 5-11. Receiver Characteristics –Bluetooth LE

Parameter

Sensitivity @30.8% PER

Maximum received signal @30.8% PER

Co-channel C/I

Adjacent channel selectivity C/I

Out-of-band blocking performance

Description

—

—

—

F = F0 + 1 MHz
F = F0 –1 MHz

F = F0 + 2 MHz
F = F0 –2 MHz

F = F0 + 3 MHz
F = F0 –3 MHz

Min

–94

0

Typ Max

Unit

–93

–92

dBm

—

— dBm

— +10

—

–5
–5
—
— –25
— –35
— –25
— –45

— dB

— dB

— dB

— dB

— dB

— dB

— dB

— dBm

— dBm

— dBm

30 MHz ~ 2000 MHz

2000 MHz ~ 2400

–10
–27

MHz

2500 MHz ~ 3000

–27

—

—

—

MHz

Espressif Systems

57
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

5 Electrical Characteristics

Parameter

Description

3000 MHz ~ 12.5 GHz

Intermodulation

—

Min
–10

–36

Typ Max

Unit

—

—

— dBm

— dBm

Espressif Systems

58
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

5 Electrical Characteristics

5.8.2 Transmitter

Table 5-12. Transmitter Characteristics –Bluetooth LE

Parameter

Description

Min

Typ Max

RF transmit power (see note under Table 5-8) —

Gain control step

RF power control range

Adjacent channel transmit power

∆ f 1avg
∆ f 2max
∆ f 2avg/∆ f 1avg
ICFT

Drift rate

Drift

—

—

F = F0 ± 2 MHz

F = F0 ± 3 MHz

F = F0 ± > 3 MHz

—

—

—

—

—

—

—

—

–12

0

3

—

— –52
— –58
— –60

—

—

+9

—

—

—

—

247

— 265

—

—

—

—

— 0.92

—

—

—

–10

0.7

2

Unit

dBm

dB

dBm

dBm

dBm

dBm

kHz

kHz

—

kHz

— kHz/50 µs

—

kHz

Espressif Systems

59
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

6 Packaging

6 Packaging

• For information about tape, reel, and chip marking, please refer to ESP32 Chip Packaging Information.

• The pins of the chip are numbered in anti-clockwise order starting from Pin 1 in the top view. For pin

numbers and pin names, see also pin layout figures in Section 2.1 Pin Layout.

Figure 6-1. QFN48 (6×6 mm) Package

Figure 6-2. QFN48 (5×5 mm) Package

Espressif Systems

60
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

Pin 1Pin 2Pin 3Pin 1Pin 2Pin 3D PIN #1 DDT L BY MARKING L 48L SLP L E e_f (5x5r1r1) 0 a.a.a. C 0 a.a.a. C TOP VIEw 1//lccclCI SIDE VIEw D2 + bl 48X PIN #1 ID C0.350 E2 REF. A A1 A3 D E 02 E2 b b1 L e Dimensional Ref. Min. Norn. Max. 0.800 0.850 0.900 0.000 --0.050 0.203 Ref. 4.950 5.000 5.050 4.950 5.000 5.050 3.650 3.700 3.750 3.650 3.700 3.750 0.130 0.180 0.230 0.070 0.120 0.170 0.300 0.350 0.400 0.350 BSC Tol. of Form&Position aaa 0.10 BOTTOM VIEw bbb 0.10 CCC 0.10 ddd 0.05 eee 0.08 fff 0.10 Notes 1.All DIMENSIONS ARE IN MILLIMETERS,2, DIMENSIONING AND T□LERANCING PER JEDEC M□-220, in 1Pn 2Related Documentation and Resources

Related Documentation and Resources

Related Documentation

• ESP32 Technical Reference Manual – Detailed information on how to use the ESP32 memory and peripherals.
• ESP32 Hardware Design Guidelines – Guidelines on how to integrate the ESP32 into your hardware product.
• ESP32 ECO and Workarounds for Bugs – Correction of ESP32 design errors.
• ESP32 Series SoC Errata – Descriptions of known errors in ESP32 series of SoCs.
• Certificates

https://espressif.com/en/support/documents/certificates

• ESP32 Product/Process Change Notifications (PCN)
https://espressif.com/en/support/documents/pcns

• ESP32 Advisories – Information on security, bugs, compatibility, component reliability.

https://espressif.com/en/support/documents/advisories

• Documentation Updates and Update Notification Subscription

https://espressif.com/en/support/download/documents

Developer Zone

• ESP-IDF Programming Guide for ESP32 – Extensive documentation for the ESP-IDF development framework.
• ESP-IDF and other development frameworks on GitHub.

https://github.com/espressif

• ESP32 BBS Forum – Engineer-to-Engineer (E2E) Community for Espressif products where you can post questions,

share knowledge, explore ideas, and help solve problems with fellow engineers.

https://esp32.com/

• The ESP Journal – Best Practices, Articles, and Notes from Espressif folks.

https://blog.espressif.com/

• See the tabs SDKs and Demos, Apps, Tools, AT Firmware.
https://espressif.com/en/support/download/sdks-demos

Products

• ESP32 Series SoCs – Browse through all ESP32 SoCs.
https://espressif.com/en/products/socs?id=ESP32

• ESP32 Series Modules – Browse through all ESP32-based modules.

https://espressif.com/en/products/modules?id=ESP32

• ESP32 Series DevKits – Browse through all ESP32-based devkits.

https://espressif.com/en/products/devkits?id=ESP32

• ESP Product Selector – Find an Espressif hardware product suitable for your needs by comparing or applying filters.

https://products.espressif.com/#/product-selector?language=en

Contact Us

• See the tabs Sales Questions, Technical Enquiries, Circuit Schematic & PCB Design Review, Get Samples

(Online stores), Become Our Supplier, Comments & Suggestions.

https://espressif.com/en/contact-us/sales-questions

Espressif Systems

61
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

Appendix A

Appendix A –ESP32 Pin Lists

A.1. Notes on ESP32 Pin Lists

Table 6-1. Notes on ESP32 Pin Lists

No.

Description

1

2

3

4

5

6

In Table IO_MUX, the boxes highlighted in yellow indicate the GPIO pins that are input-only.

Please see the following note for further details.

GPIO pins 34-39 are input-only. These pins do not feature an output driver or internal pull-

up/pull-down circuitry. The pin names are: SENSOR_VP (GPIO36), SENSOR_CAPP (GPIO37),

SENSOR_CAPN (GPIO38), SENSOR_VN (GPIO39), VDET_1 (GPIO34), VDET_2 (GPIO35).

The pins are grouped into four power domains: VDDA (analog power supply), VDD3P3_RTC

(RTC power supply), VDD3P3_CPU (power supply of digital IOs and CPU cores), VDD_SDIO

(power supply of SDIO IOs). VDD_SDIO is the output of the internal SDIO-LDO. The voltage of

SDIO-LDO can be configured at 1.8 V or be the same as that of VDD3P3_RTC. The strapping

pin and eFuse bits determine the default voltage of the SDIO-LDO. Software can change

the voltage of the SDIO-LDO by configuring register bits. For details, please see the column

“Power Domain”in Table IO_MUX.

The functional pins in the VDD3P3_RTC domain are those with analog functions, including

the 32 kHz crystal oscillator, ADC, DAC, and the capacitive touch sensor. Please see columns

“Analog Function 0 ~ 2” in Table IO_MUX.

These VDD3P3_RTC pins support the RTC function, and can work during Deep-sleep. For

example, an RTC-GPIO can be used for waking up the chip from Deep-sleep.

The GPIO pins support up to six digital functions, as shown in columns “Function 0 ~ 5” In
Table IO_MUX. The function selection registers will be set as “N”, where N is the function

number. Below are some definitions:

• SD_* is for signals of the SDIO slave.
• HS1_* is for Port 1 signals of the SDIO host.
• HS2_* is for Port 2 signals of the SDIO host.
• MT* is for signals of the JTAG.
• U0* is for signals of the UART0 module.
• U1* is for signals of the UART1 module.
• U2* is for signals of the UART2 module.
• SPI* is for signals of the SPI01 module.
• HSPI* is for signals of the SPI2 module.
• VSPI* is for signals of the SPI3 module.

Espressif Systems

62
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

Appendix A

No.

Description

Each column about digital “Function” is accompanied by a column about “Type”. Please
see the following explanations for the meanings of “type” with respect to each “function”
they are associated with. For each “Function-N”, “type” signifies:

If a function other than “Function-N” is assigned, the input signal of

• I: input only.
“Function-N” is still from this pin.
• I1: input only. If a function other than “Function-N” is assigned, the input signal of
“Function-N” is always “1”.
• I0: input only. If a function other than “Function-N” is assigned, the input signal of
“Function-N” is always “0”.
• O: output only.
• T: high-impedance.
• I/O/T: combinations of input, output, and high-impedance according to the function

signal.

• I1/O/T: combinations of input, output, and high-impedance, according to the function

signal. If a function is not selected, the input signal of the function is “1”.

For example, pin 30 can function as HS1_CMD or SD_CMD, where HS1_CMD is of an“I1/O/T”

type. If pin 30 is selected as HS1_CMD, this pin’s input and output are controlled by the SDIO
host. If pin 30 is not selected as HS1_CMD, the input signal of the SDIO host is always “1”.

Each digital output pin is associated with its configurable drive strength. Column “Drive

Strength” in Table IO_MUX lists the default values. The drive strength of the digital output

pins can be configured into one of the following four options:

• 0: ~5 mA
• 1: ~10 mA
• 2: ~20 mA
• 3: ~40 mA
The default value is 2.
The drive strength of the internal pull-up (wpu) and pull-down (wpd) is ~75 µA.

7

8

Column“At Reset” in Table IO_MUX lists the status of each pin during reset, including input-

9

enable (ie=1), internal pull-up (wpu) and internal pull-down (wpd). During reset, all pins are

10

11

12

output-disabled.

Column “After Reset” in Table IO_MUX lists the status of each pin immediately after reset,

including input-enable (ie=1), internal pull-up (wpu) and internal pull-down (wpd). After reset,
each pin is set to “Function 0”. The output-enable is controlled by digital Function 0.

Table Ethernet_MAC is about the signal mapping inside Ethernet MAC. The Ethernet MAC

supports MII and RMII interfaces, and supports both the internal PLL clock and the external

clock source. For the MII interface, the Ethernet MAC is with/without the TX_ERR signal.

MDC, MDIO, CRS and COL are slow signals, and can be mapped onto any GPIO pin through

the GPIO-Matrix.

Table GPIO Matrix is for the GPIO-Matrix. The signals of the on-chip functional modules can

be mapped onto any GPIO pin. Some signals can be mapped onto a pin by both IO-MUX
and GPIO-Matrix, as shown in the column tagged as“Same input signal from IO_MUX core”

in Table GPIO Matrix.

Espressif Systems

63
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

Appendix A

No.

Description

*In Table GPIO_Matrix，the column “Default Value if unassigned”records the default value

13

of the an input signal if no GPIO is assigned to it. The actual value is determined by register

GPIO_FUNCm_IN_INV_SEL and GPIO_FUNCm_IN_SEL. (The value of m ranges from 1 to

255.)

A.2. GPIO_Matrix

Signal

No.

Input Signals

0

1

2

3

4

5

6

7

8

9

10

11

12

13

14

15

16

17

18

23

24

25

26

27

28

29

30

31

32

33

SPICLK_in

SPIQ_in

SPID_in

SPIHD_in

SPIWP_in

SPICS0_in

SPICS1_in

SPICS2_in

HSPICLK_in

HSPIQ_in

HSPID_in

HSPICS0_in

HSPIHD_in

HSPIWP_in

U0RXD_in

U0CTS_in

U0DSR_in

U1RXD_in

U1CTS_in

I2S0O_BCK_in

I2S1O_BCK_in

I2S0O_WS_in

I2S1O_WS_in

I2S0I_BCK_in

I2S0I_WS_in

I2CEXT0_SCL_in

I2CEXT0_SDA_in

pwm0_sync0_in

pwm0_sync1_in

pwm0_sync2_in

Table 6-2. GPIO_Matrix

Same Input Signal

from IO_MUX Core Output Signals

Output Enable of

Output Signals

Default

Value If

Unassigned*

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

1

1

0

0

0

yes

yes

yes

yes

yes

yes

no

no

yes

yes

yes

yes

yes

yes

yes

yes

no

yes

yes

no

no

no

no

no

no

no

no

no

no

no

SPICLK_out

SPIQ_out

SPID_out

SPIHD_out

SPIWP_out

SPICS0_out

SPICS1_out

SPICS2_out

HSPICLK_out

HSPIQ_out

HSPID_out

HSPICS0_out

HSPIHD_out

HSPIWP_out

U0TXD_out

U0RTS_out

U0DTR_out

U1TXD_out

U1RTS_out

I2S0O_BCK_out

I2S1O_BCK_out

I2S0O_WS_out

I2S1O_WS_out

I2S0I_BCK_out

I2S0I_WS_out

I2CEXT0_SCL_out

I2CEXT0_SDA_out

sdio_tohost_int_out

pwm0_out0a

pwm0_out0b

SPICLK_oe

SPIQ_oe

SPID_oe

SPIHD_oe

SPIWP_oe

SPICS0_oe

SPICS1_oe

SPICS2_oe

HSPICLK_oe

HSPIQ_oe

HSPID_oe

HSPICS0_oe

HSPIHD_oe

HSPIWP_oe

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

Espressif Systems

64
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

Appendix A

Signal

No.

Input Signals

34

35

36

37

39

40

41

42

43

44

45

46

47

48

49

50

51

52

53

54

55

56

57

58

61

62

63

64

65

66

67

68

69

70

71

72

73

74

75

76

pwm0_f0_in

pwm0_f1_in

pwm0_f2_in

—

pcnt_sig_ch0_in0

pcnt_sig_ch1_in0

pcnt_ctrl_ch0_in0

pcnt_ctrl_ch1_in0

pcnt_sig_ch0_in1

pcnt_sig_ch1_in1

pcnt_ctrl_ch0_in1

pcnt_ctrl_ch1_in1

pcnt_sig_ch0_in2

pcnt_sig_ch1_in2

pcnt_ctrl_ch0_in2

pcnt_ctrl_ch1_in2

pcnt_sig_ch0_in3

pcnt_sig_ch1_in3

pcnt_ctrl_ch0_in3

pcnt_ctrl_ch1_in3

pcnt_sig_ch0_in4

pcnt_sig_ch1_in4

pcnt_ctrl_ch0_in4

pcnt_ctrl_ch1_in4

HSPICS1_in

HSPICS2_in

VSPICLK_in

VSPIQ_in

VSPID_in

VSPIHD_in

VSPIWP_in

VSPICS0_in

VSPICS1_in

VSPICS2_in

pcnt_sig_ch0_in5

pcnt_sig_ch1_in5

pcnt_ctrl_ch0_in5

pcnt_ctrl_ch1_in5

pcnt_sig_ch0_in6

pcnt_sig_ch1_in6

Same Input Signal

from IO_MUX Core Output Signals

Output Enable of

Output Signals

Default

Value If

Unassigned*

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

no

no

no

no

no

no

no

no

no

no

no

no

no

no

no

no

no

no

no

no

no

no

no

no

no

no

yes

yes

yes

yes

yes

yes

no

no

no

no

no

no

no

no

pwm0_out1a

pwm0_out1b

pwm0_out2a

pwm0_out2b

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

HSPICS1_out

HSPICS2_out

VSPICLK_out_mux

VSPIQ_out

VSPID_out

VSPIHD_out

VSPIWP_out

VSPICS0_out

VSPICS1_out

VSPICS2_out

ledc_hs_sig_out0

ledc_hs_sig_out1

ledc_hs_sig_out2

ledc_hs_sig_out3

ledc_hs_sig_out4

ledc_hs_sig_out5

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

HSPICS1_oe

HSPICS2_oe

VSPICLK_oe

VSPIQ_oe

VSPID_oe

VSPIHD_oe

VSPIWP_oe

VSPICS0_oe

VSPICS1_oe

VSPICS2_oe

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

Espressif Systems

65
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

Appendix A

Signal

No.

Input Signals

Same Input Signal

from IO_MUX Core Output Signals

Output Enable of

Output Signals

Default

Value If

Unassigned*

77

78

79

80

81

82

83

84

85

86

87

88

89

90

91

92

94

95

96

97

98

99

100

101

102

103

104

105

106

107

108

109

110

111

112

113

114

115

116

117

pcnt_ctrl_ch0_in6

pcnt_ctrl_ch1_in6

pcnt_sig_ch0_in7

pcnt_sig_ch1_in7

pcnt_ctrl_ch0_in7

pcnt_ctrl_ch1_in7

rmt_sig_in0

rmt_sig_in1

rmt_sig_in2

rmt_sig_in3

rmt_sig_in4

rmt_sig_in5

rmt_sig_in6

rmt_sig_in7

—

—

twai_rx

I2CEXT1_SCL_in

I2CEXT1_SDA_in

host_card_detect_n_1

host_card_detect_n_2

host_card_write_prt_1

host_card_write_prt_2

host_card_int_n_1

host_card_int_n_2

pwm1_sync0_in

pwm1_sync1_in

pwm1_sync2_in

pwm1_f0_in

pwm1_f1_in

pwm1_f2_in

pwm0_cap0_in

pwm0_cap1_in

pwm0_cap2_in

pwm1_cap0_in

pwm1_cap1_in

pwm1_cap2_in

pwm2_flta

pwm2_fltb

pwm2_cap1_in

0

0

0

0

0

0

0

0

0

0

0

0

0

0

—

—

1

1

1

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

1

1

0

no

no

no

no

no

no

no

no

no

no

no

no

no

no

—

—

no

no

no

no

no

no

no

no

no

no

no

no

no

no

no

no

no

no

no

no

no

no

no

no

ledc_hs_sig_out6

ledc_hs_sig_out7

ledc_ls_sig_out0

ledc_ls_sig_out1

ledc_ls_sig_out2

ledc_ls_sig_out3

ledc_ls_sig_out4

ledc_ls_sig_out5

ledc_ls_sig_out6

ledc_ls_sig_out7

rmt_sig_out0

rmt_sig_out1

rmt_sig_out2

rmt_sig_out3

rmt_sig_out4

rmt_sig_out6

rmt_sig_out7

I2CEXT1_SCL_out

I2CEXT1_SDA_out

host_ccmd_od_pullup_en_n

host_rst_n_1

host_rst_n_2

gpio_sd0_out

gpio_sd1_out

gpio_sd2_out

gpio_sd3_out

gpio_sd4_out

gpio_sd5_out

gpio_sd6_out

gpio_sd7_out

pwm1_out0a

pwm1_out0b

pwm1_out1a

pwm1_out1b

pwm1_out2a

pwm1_out2b

pwm2_out1h

pwm2_out1l

pwm2_out2h

pwm2_out2l

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

Espressif Systems

66
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

Appendix A

Signal

No.

Input Signals

118

119

120

121

122

123

124

125

140

141

142

143

144

145

146

147

148

149

150

151

152

153

154

155

156

157

158

159

160

161

162

163

164

165

166

167

168

169

170

171

pwm2_cap2_in

pwm2_cap3_in

pwm3_flta

pwm3_fltb

pwm3_cap1_in

pwm3_cap2_in

pwm3_cap3_in

—

I2S0I_DATA_in0

I2S0I_DATA_in1

I2S0I_DATA_in2

I2S0I_DATA_in3

I2S0I_DATA_in4

I2S0I_DATA_in5

I2S0I_DATA_in6

I2S0I_DATA_in7

I2S0I_DATA_in8

I2S0I_DATA_in9

I2S0I_DATA_in10

I2S0I_DATA_in11

I2S0I_DATA_in12

I2S0I_DATA_in13

I2S0I_DATA_in14

I2S0I_DATA_in15

—

—

—

—

—

—

—

—

I2S1I_BCK_in

I2S1I_WS_in

I2S1I_DATA_in0

I2S1I_DATA_in1

I2S1I_DATA_in2

I2S1I_DATA_in3

I2S1I_DATA_in4

I2S1I_DATA_in5

Same Input Signal

from IO_MUX Core Output Signals

Output Enable of

Output Signals

Default

Value If

Unassigned*

0

0

1

1

0

0

0

—

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

0

—

—

—

—

—

—

—

—

0

0

0

0

0

0

0

0

no

no

no

no

no

no

no

—

no

no

no

no

no

no

no

no

no

no

no

no

no

no

no

no

—

—

—

—

—

—

—

—

no

no

no

no

no

no

no

no

pwm2_out3h

pwm2_out3l

pwm2_out4h

pwm2_out4l

—

twai_tx

twai_bus_off_on

twai_clkout

I2S0O_DATA_out0

I2S0O_DATA_out1

I2S0O_DATA_out2

I2S0O_DATA_out3

I2S0O_DATA_out4

I2S0O_DATA_out5

I2S0O_DATA_out6

I2S0O_DATA_out7

I2S0O_DATA_out8

I2S0O_DATA_out9

I2S0O_DATA_out10

I2S0O_DATA_out11

I2S0O_DATA_out12

I2S0O_DATA_out13

I2S0O_DATA_out14

I2S0O_DATA_out15

I2S0O_DATA_out16

I2S0O_DATA_out17

I2S0O_DATA_out18

I2S0O_DATA_out19

I2S0O_DATA_out20

I2S0O_DATA_out21

I2S0O_DATA_out22

I2S0O_DATA_out23

I2S1I_BCK_out

I2S1I_WS_out

I2S1O_DATA_out0

I2S1O_DATA_out1

I2S1O_DATA_out2

I2S1O_DATA_out3

I2S1O_DATA_out4

I2S1O_DATA_out5

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

Espressif Systems

67
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

Appendix A

Signal

No.

Input Signals

172

173

174

175

176

177

178

179

180

181

182

183

184

185

186

187

188

189

190

191

192

193

194

195

196

197

198

199

200

201

202

203

204

205

206

207

208

209

210

211

I2S1I_DATA_in6

I2S1I_DATA_in7

I2S1I_DATA_in8

I2S1I_DATA_in9

I2S1I_DATA_in10

I2S1I_DATA_in11

I2S1I_DATA_in12

I2S1I_DATA_in13

I2S1I_DATA_in14

I2S1I_DATA_in15

—

—

—

—

—

—

—

—

I2S0I_H_SYNC

I2S0I_V_SYNC

I2S0I_H_ENABLE

I2S1I_H_SYNC

I2S1I_V_SYNC

I2S1I_H_ENABLE

—

—

U2RXD_in

U2CTS_in

emac_mdc_i

emac_mdi_i

emac_crs_i

emac_col_i

pcmfsync_in

pcmclk_in

pcmdin

—

—

—

—

—

Same Input Signal

from IO_MUX Core Output Signals

Output Enable of

Output Signals

Default

Value If

Unassigned*

0

0

0

0

0

0

0

0

0

0

—

—

—

—

—

—

—

—

0

0

0

0

0

0

—

—

0

0

0

0

0

0

0

0

0

—

—

—

—

—

no

no

no

no

no

no

no

no

no

no

—

—

—

—

—

—

—

—

no

no

no

no

no

no

—

—

yes

yes

no

no

no

no

no

no

no

—

—

—

—

—

I2S1O_DATA_out6

I2S1O_DATA_out7

I2S1O_DATA_out8

I2S1O_DATA_out9

I2S1O_DATA_out10

I2S1O_DATA_out11

I2S1O_DATA_out12

I2S1O_DATA_out13

I2S1O_DATA_out14

I2S1O_DATA_out15

I2S1O_DATA_out16

I2S1O_DATA_out17

I2S1O_DATA_out18

I2S1O_DATA_out19

I2S1O_DATA_out20

I2S1O_DATA_out21

I2S1O_DATA_out22

I2S1O_DATA_out23

pwm3_out1h

pwm3_out1l

pwm3_out2h

pwm3_out2l

pwm3_out3h

pwm3_out3l

pwm3_out4h

pwm3_out4l

U2TXD_out

U2RTS_out

emac_mdc_o

emac_mdo_o

emac_crs_o

emac_col_o

bt_audio0_irq

bt_audio1_irq

bt_audio2_irq

ble_audio0_irq

ble_audio1_irq

ble_audio2_irq

pcmfsync_out

pcmclk_out

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

emac_mdc_oe

emac_mdo_o_e

emac_crs_oe

emac_col_oe

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

pcmfsync_en

pcmclk_en

Espressif Systems

68
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

Appendix A

Signal

No.

Input Signals

212

213

214

215

224

225

226

227

228

—

—

—

—

—

—

—

—

—

Default

Value If

Unassigned*

—

—

—

—

—

—

—

—

—

Same Input Signal

from IO_MUX Core Output Signals

Output Enable of

Output Signals

—

—

—

—

—

—

—

—

—

pcmdout

ble_audio_sync0_p

ble_audio_sync1_p

ble_audio_sync2_p

sig_in_func224

sig_in_func225

sig_in_func226

sig_in_func227

sig_in_func228

pcmdout_en

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

1’d1

A.3. Ethernet_MAC

Table 6-3. Ethernet_MAC

Pin Name

Function6

MII (int_osc)

MII (ext_osc)

RMII (int_osc)

RMII (ext_osc)

TX_CLK (I)

CLK_OUT(O)

EXT_OSC_CLK(I)

RX_CLK (I)

—

GPIO0

GPIO5

GPIO21

GPIO19

GPIO22

MTMS

MTDI

MTCK

GPIO27

GPIO25

GPIO26

U0TXD

MTDO

GPIO16

GPIO17

GPIO4

EMAC_TX_CLK

EMAC_RX_CLK

EMAC_TX_EN

EMAC_TXD0

EMAC_TXD1

EMAC_TXD2

EMAC_TXD3

EMAC_RX_ER

EMAC_RX_DV

EMAC_RXD0

EMAC_RXD1

EMAC_RXD2

EMAC_RXD3

TX_CLK (I)

RX_CLK (I)

TX_EN(O)

TXD[0](O)

TXD[1](O)

TXD[2](O)

TXD[3](O)

RX_ER(I)

RX_DV(I)

RXD[0](I)

RXD[1](I)

RXD[2](I)

RXD[3](I)

TX_EN(O)

TXD[0](O)

TXD[1](O)

TXD[2](O)

TXD[3](O)

RX_ER(I)

RX_DV(I)

RXD[0](I)

RXD[1](I)

RXD[2](I)

RXD[3](I)

TX_EN(O)

TXD[0](O)

TXD[1](O)

—

—

—

RXD[0](I)

RXD[1](I)

—

—

EMAC_CLK_OUT

CLK_OUT(O)

—

CLK_OUT(O)

EMAC_CLK_OUT_180 CLK_OUT_180(O) —

CLK_OUT_180(O) —

EMAC_TX_ER

TX_ERR(O)*

TX_ERR(O)*

—

CRS_DV(I)

CRS_DV(I)

—

TX_EN(O)

TXD[0](O)

TXD[1](O)

—

—

—

RXD[0](I)

RXD[1](I)

—

—

—

—

MDC(O)

MDIO(IO)

—

—

In GPIO Matrix*

In GPIO Matrix*

In GPIO Matrix*

In GPIO Matrix*

—

—

—

—

MDC(O)

MDIO(IO)

CRS(I)

COL(I)

MDC(O)

MDIO(IO)

CRS(I)

COL(I)

MDC(O)

MDIO(IO)

—

—

*Notes: 1. The GPIO Matrix can be any GPIO. 2. The TX_ERR (O) is optional.

A.4. IO_MUX

For the list of IO_MUX pins, please see the next page.

Espressif Systems

69
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

A
p
p
e
n
d
x
A

i

S
u
b
m

i
t

D
o
c
u
m
e
n
t
a
t
i
o
n
F
e
e
d
b
a
c
k

E
s
p
r
e
s
s
i
f
S
y
s
t
e
m
s

7
0

E
S
P
3
2
S
e
r
i
e
s
D
a
t
a
s
h
e
e
t

v
5
.
2

IO_MUXPin No.Power Supply PinAnalog PinDigital PinPower DomainAnalog Function0Analog Function1Analog Function2RTC Function0RTC Function1Function0TypeFunction1TypeFunction2TypeFunction3TypeFunction4TypeFunction5TypeDrive Strength (2’d2: 20 mA)At ResetAfter Reset1VDDAVDDA supply in2LNA_INVDD3P33VDD3P3VDD3P3 supply in4VDD3P3VDD3P3 supply in5SENSOR_VPVDD3P3_RTCADC1_CH0RTC_GPIO0GPIO36IGPIO36Ioe=0, ie=0oe=0, ie=06SENSOR_CAPPVDD3P3_RTCADC1_CH1RTC_GPIO1GPIO37IGPIO37Ioe=0, ie=0oe=0, ie=07SENSOR_CAPNVDD3P3_RTCADC1_CH2RTC_GPIO2GPIO38IGPIO38Ioe=0, ie=0oe=0, ie=08SENSOR_VNVDD3P3_RTCADC1_CH3RTC_GPIO3GPIO39IGPIO39Ioe=0, ie=0oe=0, ie=09CHIP_PUVDD3P3_RTC10VDET_1VDD3P3_RTCADC1_CH6RTC_GPIO4GPIO34IGPIO34Ioe=0, ie=0oe=0, ie=011VDET_2VDD3P3_RTCADC1_CH7RTC_GPIO5GPIO35IGPIO35Ioe=0, ie=0oe=0, ie=01232K_XPVDD3P3_RTCXTAL_32K_PADC1_CH4TOUCH9RTC_GPIO9GPIO32I/O/TGPIO32I/O/T2'd2oe=0, ie=0oe=0, ie=01332K_XNVDD3P3_RTCXTAL_32K_NADC1_CH5TOUCH8RTC_GPIO8GPIO33I/O/TGPIO33I/O/T2'd2oe=0, ie=0oe=0, ie=014GPIO25VDD3P3_RTCDAC_1ADC2_CH8RTC_GPIO6GPIO25I/O/TGPIO25I/O/TEMAC_RXD0I2'd2oe=0, ie=0oe=0, ie=015GPIO26VDD3P3_RTCDAC_2ADC2_CH9RTC_GPIO7GPIO26I/O/TGPIO26I/O/TEMAC_RXD1I2'd2oe=0, ie=0oe=0, ie=016GPIO27VDD3P3_RTCADC2_CH7TOUCH7RTC_GPIO17GPIO27I/O/TGPIO27I/O/TEMAC_RX_DVI2'd2oe=0, ie=0oe=0, ie=017MTMSVDD3P3_RTCADC2_CH6TOUCH6RTC_GPIO16MTMSI0HSPICLKI/O/TGPIO14I/O/THS2_CLKOSD_CLKI0EMAC_TXD2O2'd2oe=0, ie=0oe=0, ie=1, wpu18MTDIVDD3P3_RTCADC2_CH5TOUCH5RTC_GPIO15MTDII1HSPIQI/O/TGPIO12I/O/THS2_DATA2I1/O/TSD_DATA2I1/O/TEMAC_TXD3O2'd2oe=0, ie=1, wpdoe=0, ie=1, wpd19VDD3P3_RTCVDD3P3_RTC supply in20MTCKVDD3P3_RTCADC2_CH4TOUCH4RTC_GPIO14MTCKI1HSPIDI/O/TGPIO13I/O/THS2_DATA3I1/O/TSD_DATA3I1/O/TEMAC_RX_ERI2'd2oe=0, ie=0oe=0, ie=1, wpd21MTDOVDD3P3_RTCADC2_CH3TOUCH3RTC_GPIO13I2C_SDAMTDOO/THSPICS0I/O/TGPIO15I/O/THS2_CMDI1/O/TSD_CMDI1/O/TEMAC_RXD3I2'd2oe=0, ie=1, wpuoe=0, ie=1, wpu22GPIO2VDD3P3_RTCADC2_CH2TOUCH2RTC_GPIO12I2C_SCLGPIO2I/O/THSPIWPI/O/TGPIO2I/O/THS2_DATA0I1/O/TSD_DATA0I1/O/T2'd2oe=0, ie=1, wpdoe=0, ie=1, wpd23GPIO0VDD3P3_RTCADC2_CH1TOUCH1RTC_GPIO11I2C_SDAGPIO0I/O/TCLK_OUT1OGPIO0I/O/TEMAC_TX_CLKI2'd2oe=0, ie=1, wpuoe=0, ie=1, wpu24GPIO4VDD3P3_RTCADC2_CH0TOUCH0RTC_GPIO10I2C_SCLGPIO4I/O/THSPIHDI/O/TGPIO4I/O/THS2_DATA1I1/O/TSD_DATA1I1/O/TEMAC_TX_ERO2'd2oe=0, ie=1, wpdoe=0, ie=1, wpd25GPIO16VDD_SDIOGPIO16I/O/TGPIO16I/O/THS1_DATA4I1/O/TU2RXDI1EMAC_CLK_OUTO2'd2oe=0, ie=0oe=0, ie=126VDD_SDIOVDD_SDIO supply out/in27GPIO17VDD_SDIOGPIO17I/O/TGPIO17I/O/THS1_DATA5I1/O/TU2TXDOEMAC_CLK_OUT_180O2'd2oe=0, ie=0oe=0, ie=128SD_DATA_2VDD_SDIOSD_DATA2I1/O/TSPIHDI/O/TGPIO9I/O/THS1_DATA2I1/O/TU1RXDI12'd2oe=0, ie=1, wpuoe=0, ie=1, wpu29SD_DATA_3VDD_SDIOSD_DATA3I0/O/TSPIWPI/O/TGPIO10I/O/THS1_DATA3I1/O/TU1TXDO2'd2oe=0, ie=1, wpuoe=0, ie=1, wpu30SD_CMDVDD_SDIOSD_CMDI1/O/TSPICS0I/O/TGPIO11I/O/THS1_CMDI1/O/TU1RTSO2'd2oe=0, ie=1, wpuoe=0, ie=1, wpu31SD_CLKVDD_SDIOSD_CLKI0SPICLKI/O/TGPIO6I/O/THS1_CLKOU1CTSI12'd2oe=0, ie=1, wpuoe=0, ie=1, wpu32SD_DATA_0VDD_SDIOSD_DATA0I1/O/TSPIQI/O/TGPIO7I/O/THS1_DATA0I1/O/TU2RTSO2'd2oe=0, ie=1, wpuoe=0, ie=1, wpu33SD_DATA_1VDD_SDIOSD_DATA1I1/O/TSPIDI/O/TGPIO8I/O/THS1_DATA1I1/O/TU2CTSI12'd2oe=0, ie=1, wpuoe=0, ie=1, wpu34GPIO5VDD3P3_CPUGPIO5I/O/TVSPICS0I/O/TGPIO5I/O/THS1_DATA6I1/O/TEMAC_RX_CLKI2'd2oe=0, ie=1, wpuoe=0, ie=1, wpu35GPIO18VDD3P3_CPUGPIO18I/O/TVSPICLKI/O/TGPIO18I/O/THS1_DATA7I1/O/T2'd2oe=0, ie=0oe=0, ie=136GPIO23VDD3P3_CPUGPIO23I/O/TVSPIDI/O/TGPIO23I/O/THS1_STROBEI02'd2oe=0, ie=0oe=0, ie=137VDD3P3_CPUVDD3P3_CPU supply in38GPIO19VDD3P3_CPUGPIO19I/O/TVSPIQI/O/TGPIO19I/O/TU0CTSI1EMAC_TXD0O2'd2oe=0, ie=0oe=0, ie=139GPIO22VDD3P3_CPUGPIO22I/O/TVSPIWPI/O/TGPIO22I/O/TU0RTSOEMAC_TXD1O2'd2oe=0, ie=0oe=0, ie=140U0RXDVDD3P3_CPUU0RXDI1CLK_OUT2OGPIO3I/O/T2'd2oe=0, ie=1, wpuoe=0, ie=1, wpu41U0TXDVDD3P3_CPUU0TXDOCLK_OUT3OGPIO1I/O/TEMAC_RXD2I2'd2oe=0, ie=1, wpuoe=0, ie=1, wpu42GPIO21VDD3P3_CPUGPIO21I/O/TVSPIHDI/O/TGPIO21I/O/TEMAC_TX_ENO2'd2oe=0, ie=0oe=0, ie=143VDDAVDDA supply in44XTAL_NVDDA45XTAL_PVDDA46VDDAVDDA supply in47CAP2VDDA48CAP1VDDATotal Number81426Notes: •wpu: weak pull-up; •wpd: weak pull-down; •ie: input enable; •oe: output enable; •Please see Table: Notes on ESP32 Pin Lists for more information.（请参考表：管脚清单说明。）Revision History

Revision History

Date

Version

Release notes

2025.11

v5.2

• ESP32-D0WDR2-V3 is end of life and upgraded to ESP32-D0WDRH2-V3
• Table 1-1 Comparison: Updated ”Ordering Code” to ”Part Number”

• Section 4.8.3 Universal Asynchronous Receiver Transmitter (UART): Added

description ”Programmable baud rates up to 5 MBaud”

2025.10

v5.1

• Section 3 Boot Configurations: Fixed the typo about Internal LDO (VDD_SDIO)

Voltage Control
• Fixed other typos

2025.08

v5.0

• Table 2-3 Power Pins: Added power pin 1 VDDA
• Updated Figure 3-1 Visualization of Timing Parameters for the Strapping Pins
• Table 5-3 DC Characteristics (3.3 V, 25 °C): Added VIH_nRST

2025.04

v4.9

• Section CPU and Memory: Improved CoreMark scores
• Section 3.1 Chip Boot Mode Control: Modified the description from ”valid only
for ESP32 ECO V3” to ”valid only for ESP32 chip revisions v3.0 and higher”
• Section 4.8.2 Serial Peripheral Interface (SPI): Added information about SPI
• Table 2-2 Analog Pins: Fixed typos about pin numbers

• Section 3 Boot Configurations: Fixed the typo about JTAG signal source

control

2025.01

v4.8

• Section 2.2 Pin Overview: Added a note about JTAG interface signals
• Table 2-5 Pin Mapping Between Chip and Flash/PSRAM: Modified a note

about VDD_SDIO

• Table 5-2 Recommended Power Supply Characteristics: Deleted a note about

VDD3P3_RTC limitation

2024.09

v4.7

• Section 4.1.1 CPU: Fixed the link to Cadence Xtensa ISA Summary
• Section 4.8.7 Pulse Counter Controller (PCNT): Fixed the typo in the Feature

List

2024.08

v4.6

Improved the formatting, structure, and wording in the following sections:

• Section 2 Pins
• Section 3 Boot Configurations (used to be named as ”Strapping Pins”)
• Section 4 Functional Description

Cont’d on next page

Espressif Systems

71
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

Revision History

Date

Version

Release notes

Cont’d from previous page

2024.02

v4.5

Section 2.5.3 Chip Power-up and Reset: Updated the link to the VDD_SDIO 1.8 V

circuit design to ESP32 Hardware Design Guidelines

2023.12

v4.4

Table 1-1 Comparison: Added information about flash under the table

2023.07

v4.3

• Updated formatting throughout the document
• Updated wording in some sections
• Added a new section 2.3.1 Restrictions for GPIOs and RTC_GPIOs
• Added a new section 4.1.5 Cache

2023.01

v4.2

• Removed contents about hall sensor according to PCN20221202
• Section 4.9.3 Touch Sensor: Added a note about limited applications of touch

sensor

• Section 4.1.1 CPU: Added link to Xtensa® Instruction Set Architecture (ISA)

2022.12

v4.1

Summary

• Table 1-1 Comparison: Updated the description about chip revision upgrade

2022.10

v4.0

2022.03

v3.9

• Section Product Overview: Updated the description
• Table 2-6 Pin Mapping Between Chip and Flash/PSRAM: Added two notes

below the table

• Section 2.5.2 Power Scheme: Added a new item to “Notes on power supply”
• Updated Figure 1-1 ESP32 Series Nomenclature
• Table 1-1 Comparison: Added a new column “VDD_SDIO Voltage”
• Section 4.8.12 TWAI® Controller: Updated the bit rates
• Added Not Recommended for New Designs (NRND) label to ESP32-S0WD

• Added a new chip variant ESP32-D0WDR2-V3
• Added Table 2-5 Pin Mapping Between Chip and Flash/PSRAM and Table 2-6

Pin Mapping Between Chip and Flash/PSRAM
• Updated Figure 6-2 QFN48 (5×5 mm) Package
• Updated Appendix IO_MUX
• Updated Table 4-6 Peripheral Pin Configurations
• Section 3.1 Chip Boot Mode Control: Added links to ESP32 Technical

Reference Manual

Cont’d on next page

Espressif Systems

72
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

Revision History

Date

Version

Release notes

Cont’d from previous page

2021.10

v3.8

• Upgraded ESP32-U4WDH variant from single-core to dual-core, see

PCN-2021-021. The single-core version coexists with the new dual-core

version around December 2, 2021. The physical product is subject to batch

tracking.

• Section CPU and Memory: Added CoreMark® score
• Section 4.8.12 TWAI® Controller: Updated the description
• Added Not Recommended for New Designs (NRND) label to the

ESP32-D0WDQ6-V3 variant

• Section 6 Packaging: Provided a link to Espressif Chip Package Information
• Updated Section Bluetooth

2021.07

v3.7

• Removed ESP32-D2WD variant
• Section 4.7.1 Bluetooth Radio and Baseband: Updated wording
• Updated pin function numbers starting from Function0
• Added Not Recommended for New Designs (NRND) label to ESP32-D0WD and

ESP32-D0WDQ6 variants

2021.03

V3.6

• Updated Figure Block Diagram
• Updated Table 5-5 Reliability
• Updated Figure 2-3 ESP32 Power Scheme
• Updated Table 5-2 Recommended Power Supply Characteristics
• Updated the notes below Table 2-4 Description of Timing Parameters for

Power-up and Reset

• Table 4-1, 4-6, Section 4.8.12 TWAI® Controller: Added more information about

TWAI®

2021.01

V3.5

• Table 2-1 Pin Overview: Updated the description for CAP2 from 3 nF to 3.3 nF
• Section Advanced Peripheral Interfaces: Added TWAI®
• Updated Figure Block Diagram
• Appendix IO_MUX: Updated the reset values for MTCK, MTMS, GPIO27

2020.04

V3.4

• Added one chip variant: ESP32-U4WDH
• Updated some figures in Table 4-2, 5-6, 5-7, 5-9, 5-11, 5-12
• Table 5-7 Receiver –Basic Data Rate: Added a note under the table

Cont’d on next page

Espressif Systems

73
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

Revision History

Date

Version

Release notes

Cont’d from previous page

2020.01

V3.3

• Added two chip variants: ESP32-D0WD-V3 and ESP32-D0WDQ6-V3.
• Added a note under Table 4-3 Analog-to-Digital Converter (ADC)

2019.10

V3.2

• Updated Figure 2-4 Visualization of Timing Parameters for Power-up and Reset

• Table 2-1 Pin Overview: Added pin-pin mapping between ESP32-D2WD and

2019.07

V3.1

the in-package flash under the table

• Updated Figure 1-1 ESP32 Series Nomenclature

2019.04

V3.0

• Section 3 Boot Configurations (used to be named as ”Strapping Pins”): Added

information about the setup and hold times for the strapping pins

2019.02

V2.9

• Table 2-1 Pin Overview: Applied new formatting
• Table 4-6 Peripheral Pin Configurations: Fixed typos with respect to the ADC1

channel mappings

• Changed the RF power control range in Table 5-7, 5-10, and 5-12 from –12 ~

2019.01

V2.8

+12 to –12 ~ +9 dBm;

• Small text changes

2018.11

V2.7

• Updated Section Applications
• Table IO_MUX: Updated pin statuses at reset and after reset

2018.10

V2.6

• Section 6 Packaging: Updated QFN package drawings

2018.08

V2.5

• Table 5-1 Absolute Maximum Ratings: Added ”Cumulative IO output current”
• Table 5-3 DC Characteristics (3.3 V, 25 °C): Added more parameters
• Appendix IO_MUX: Changed the power domain names to be consistent with

the pin names

Cont’d on next page

Espressif Systems

74
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

Revision History

Date

Version

Release notes

Cont’d from previous page

2018.07

V2.4

• Deleted information on Packet Traffic Arbitration (PTA);
• Added Figure 2-4 Visualization of Timing Parameters for Power-up and Reset
• Table 4-2 Power Management Unit (PMU): Added the current consumption

figures for dual-core SoCs

• Updated Section 4.9.1 Analog-to-Digital Converter (ADC)

2018.06

V2.3

• Table 4-2 Power Management Unit (PMU): Added the current consumption

figures at CPU frequency of 160 MHz

2018.05

V2.2

• Table 2-1 Pin Overview: Changed the voltage range of VDD3P3_RTC from

1.8-3.6 V to 2.3-3.6 V

• Updated Section 2.5.2 Power Scheme
• Updated Section 4.1.3 External Flash and RAM
• Updated Table 4-2 Power Management Unit (PMU)
• Removed content about temperature sensor;

Changes to electrical characteristics:

• Updated Table 5-1 Absolute Maximum Ratings
• Added Table 5-2 Recommended Power Supply Characteristics
• Added Table 5-3 DC Characteristics (3.3 V, 25 °C)
• Added Table 5-5 Reliability
• Table 5-7 Receiver –Basic Data Rate: Updated the values of ”Gain control step”

and ”Adjacent channel transmit power”

• Table 5-10 Transmitter –Enhanced Data Rate: Updated the values of ”Gain
control step”, ”π/4 DQPSK modulation accuracy”, ”8 DPSK modulation

accuracy”, and ”In-band spurious emissions”

• Table 5-12 Transmitter: Updated the values of ”Gain control step” and

”Adjacent channel transmit power”

2018.01

V2.1

• Deleted software-specific features;
• Deleted information on LNA pre-amplifier;
• Specified the CPU speed and flash speed of ESP32-D2WD;
• Section 2.5.2 Power Scheme: Added notes

2017.12

V2.0

• Section 6 Packaging: Added a note on the sequence of pin number

Cont’d on next page

Espressif Systems

75
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

Revision History

Date

Version

Release notes

Cont’d from previous page

2017.10

V1.9

• Table 2-1 Pin Overview: Updated the description of pin CHIP_PU
• Section 2.5.2 Power Scheme: Added a note
• Section 3 Boot Configurations (used to be named as ”Strapping Pins”):

Updated the description of the chip’s system reset

• Section 4.6.4 Wi-Fi Radio and Baseband: Added a description of antenna

diversity and selection

• Table 4-2 Power Management Unit (PMU): Deleted ”Association sleep pattern”,

added notes to Active sleep and Modem-sleep

2017.08

V1.8

• Added Table 4-6 Peripheral Pin Configurations
• Figure Block Diagram: Corrected a typo

2017.08

V1.7

• Section Bluetooth: Changed the transmitting power to +12 dBm; the sensitivity

of NZIF receiver to –97 dBm

• Table 2-1 Pin Overview: Added a note
• section 4.1.1 CPU: Added 160 MHz clock frequency
• Section 4.6.4 Wi-Fi Radio and Baseband: Changed the transmitting power

from 21 dBm to 20.5 dBm

• Section 4.7.1 Bluetooth Radio and Baseband: Changed the dynamic control
range of class-1, class-2 and class-3 transmit output powers to ”up to 24

dBm”; changed the dynamic range of NZIF receiver sensitivity to ”over 97 dB”

• Table 4-2 Power Management Unit (PMU): Added two notes
• Updated Section 4.8.1 General Purpose Input / Output Interface (GPIO)
• Updated Section 4.8.11 SDIO/SPI Slave Controller
• Updated Table 5-1 Absolute Maximum Ratings
• Table 5.4 RF Current Consumption in Active Mode: Changed the duty cycle on

which the transmitters’measurements are based to 50%.
• Table 5-6 Wi-Fi Radio: Added a note on “Output impedance”
• Table 5-7, 5-9, 5-11: Updated parameter ”Sensitivity”
• Table 5-7, 5-10, 5-12: Updated parameters ”RF transmit power” and ”RF power

control range”; added parameter ”Gain control step”

• Deleted Chapters: ”Touch Sensor” and ”Code Examples”;
• Added a link to certification download.

2017.06

V1.6

• Section Complete Integration Solution: Changed the number of external

components to 20

• Section 4.8.1 General Purpose Input / Output Interface (GPIO): Changed the

number of GPIO pins to 34

Espressif Systems

76
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

Cont’d on next page

Revision History

Date

Version

Release notes

Cont’d from previous page

2017.06

V1.5

• Section CPU and Memory: Changed the power supply range
• Section 2.5.2 Power Scheme: Updated the note
• Updated Table 5-1 Absolute Maximum Ratings
• Table Notes on ESP32 Pin Lists: Changed the drive strength values of the

digital output pins in Note 8

• Added the option to subscribe for notifications of documentation changes

• Section Clocks and Timers: Added a note to the frequency of the external

crystal oscillator

• Section 3 Boot Configurations (used to be named as ”Strapping Pins”): Added

a note

2017.05

V1.4

• Updated Section 4.3 RTC and Low-power Management
• Table 5-1 Absolute Maximum Ratings: Changed the maximum driving capability

from 12 mA to 80 mA

• Table 5-6 Wi-Fi Radio: Changed the input impedance value of 50Ω to output

impedance value of 30+j10 Ω

• Table Notes on ESP32 Pin Lists: Added a note to No.8
• Table IO_MUX: Deleted GPIO20

2017.04

V1.3

• Added Appendix Notes on ESP32 Pin Lists
• Updated Table 5-6 Wi-Fi Radio
• Updated Figure 2-2 ESP32 Pin Layout (QFN 5*5, Top View)

2017.03

V1.2

• Table 2-1 Pin Overview: Added a note
• Section 4.1.2 Internal Memory: Updated the note

2017.02

V1.1

• Added Section 1 ESP32 Series Comparison
• Updated Section MCU and Advanced Features
• Updated Section Block Diagram
• Updated Section 2 Pins
• Updated Section CPU and Memory
• Updated Section 4.2.3 Audio PLL Clock
• Updated Section 5.1 Absolute Maximum Ratings
• Updated Section 6 Packaging
• Updated Section Related Documentation and Resources

2016.08

V1.0

First release.

Espressif Systems

77
Submit Documentation Feedback

ESP32 Series Datasheet v5.2

Disclaimer and Copyright Notice
Information in this document, including URL references, is subject to change without notice.

ALL THIRD PARTY’S INFORMATION IN THIS DOCUMENT IS PROVIDED AS IS WITH NO WARRANTIES TO ITS AUTHENTICITY AND
ACCURACY.

NO WARRANTY IS PROVIDED TO THIS DOCUMENT FOR ITS MERCHANTABILITY, NON-INFRINGEMENT, FITNESS FOR ANY PARTICULAR
PURPOSE, NOR DOES ANY WARRANTY OTHERWISE ARISING OUT OF ANY PROPOSAL, SPECIFICATION OR SAMPLE.

All liability, including liability for infringement of any proprietary rights, relating to use of information in this document is disclaimed. No
licenses express or implied, by estoppel or otherwise, to any intellectual property rights are granted herein.

The Wi-Fi Alliance Member logo is a trademark of the Wi-Fi Alliance. The Bluetooth logo is a registered trademark of Bluetooth SIG.

All trade names, trademarks and registered trademarks mentioned in this document are property of their respective owners, and are
hereby acknowledged.

Copyright © 2025 Espressif Systems (Shanghai) Co., Ltd. All rights reserved.

www.espressif.com

