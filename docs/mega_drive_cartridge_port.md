# Port cartouche / Cartridge port

[Source](https://melaircraft.net/2019/01/17/sega-megadrive-genesis-cartridge-pinout/)

| pin | Direction | Name | Description | pin | Direction | Name   | Description                    | 
|:-:  |:-:        |:-:   |:-:          |:-:  |:-:        |:-:     |:-:                             |
| A01 | O         | GND  | Ground      | B01 | I         | SL1    | Left Audio                     |
| A02 | O         | +5V  | +5V DC      | B02 | O         | /MRES  | Master reset [Note 1](#note-1) | 
| A03 | O         | A08  | Address 8   | B03 | I         | SR1    | Right Audio                    |
| A04 | O         | A11  | Address 11  | B04 | O         | A09    | Address 09                     |
| A05 | O         | A07  | Address 7   | B05 | O         | A10    | Address 10                     |
| A06 | O         | A12  | Address 12  | B06 | O         | A18    | Address 18                     |
| A07 | O         | A06  | Address 6   | B07 | O         | A19    | Address 19                     |
| A08 | O         | A13  | Address 13  | B08 | O         | A20    | Address 20                     |
| A09 | O         | A05  | Address 5   | B09 | O         | A21    | Address 21                     |
| A10 | O         | A14  | Address 14  | B10 | O         | A22    | Address 22                     |
| A11 | O         | A04  | Address 4   | B11 | O         | A23    | Address 23                     |
| A12 | O         | A15  | Address 15  | B12 | O         | /YS    | VDP is currently drawing the backdrop colour |
| A13 | O         | A03  | Address 3   | B13 | O         | /VSYNC | Vertical sync pulse            |
| A14 | O         | A16  | Address 16  | B14 | O         | /HSYNC | Horizonal sync pulse           |
| A15 | O         | A02  | Address 2   | B15 | O         | EDCLK  | External Dot Clock (~13.4 or 10.7 MHz) |
| A16 | O         | A17  | Address 17  | B16 | O         | /CAS0  | Read or Write on $000000-$DFFFFF region |
| A17 | O         | A01  | Address 1   | B17 | O         | /CEO   | Chip Enable [Note 2](#note-2)  |
| A18 | O         | GND  | Ground      | B18 | O         | /AS    | Address strobe [Note 3](#note-3) |
| A19 | O         | D07  | Data 7      | B19 | O         | VCLK   | 68K Clock                      |
| A20 | O / I     | D00  | Data 0      | B20 | I         | /DTACK | Data acknowledge to 68K        |
| A21 | O / I     | D08  | Data 8      | B21 | O         | /CAS2  | Read or Write on $E00000-$FFFFFF region, maybe (Upper 2MB) |
| A22 | O / I     | D06  | Data 6      | B22 | O / I     | D15    | Data 15                        |
| A23 | O / I     | D01  | Data 1      | B23 | O / I     | D14    | Data 14                        |
| A24 | O / I     | D09  | Data 9      | B24 | O / I     | D13    | Data 13                        |
| A25 | O / I     | D05  | Data 5      | B25 | O / I     | D12    | Data 12                        |
| A26 | O / I     | D02  | Data 2      | B26 | O         | /ASEL  | Read or Write on $000000-$7FFFFF region |
| A27 | O / I     | D10  | Data 10     | B27 | O         | /VRES  | System reset [Note 4](#note-4) |
| A28 | O / I     | D04  | Data 4      | B28 | O         | /LWR   | -                              |
| A29 | O / I     | D03  | Data 3      | B29 | O         | /UWR   | -                              |
| A30 | O / I     | D11  | Data 11     | B30 | I         | /M3    | Master System mode [Note 5](#note-5) |
| A31 | O         | +5V  | +5V DC      | B31 | O         | /TIME  | [Note 6](#note-6)              |
| A32 | O         | GND  | Ground      | B32 | I         | /CART  | [Note 7](#note-7)              |

###### Note 1
Master reset, from system start up.
###### Note 2
Chip Enable for the cartridge.\
Normally low when accessing $000000-$3FFFFF region.\
When expension unit is present then low when accessing $400000-$7FFFFF.
###### Note 3
Address strobe, address on bus is currently valid and not changing.
###### Note 4
System reset, from front panel switch.
###### Note 5
Pulled high in console, cartridge shorts to ground to indicate console should run in Mark 3 (Master System) mode.
###### Note 6
Set for r/w at/to $A13000-$A130FF, given the name suggests it might be for a real time clock in the cartridge.\
This would enable usage as a chip enable on a RTC, using fewer address lines or logic to that RTC. Used in Sonic 3 for SRAM.
###### Note 7
Pulled high in console, cartridge shorts to ground to indicate presence.
