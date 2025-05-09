# Port d'extension / Expansion port

[Source](https://gamesx.com/wiki/doku.php?id=schematics:megadriveexpport)

|   pin   |   Name   |   Description   |   pin   |   Name   |   Description   | 
|:-:    |:-:    |:-:    |:-:    |:-:    |:-:    |
| A01 | GND | Ground | B01 | GND | Ground |
| A02 | A19 | Address 19 | B02 | /DISK | [(1)](######(1)) | 
| A03 | /CAS0 | - | B03 | A01 | Address 01 | 
| A04 | D00 | Data 00 | B04 | A02 | Address 02 | 
| A05 | D01 | Data 01 | B05 | A03 | Address 03 | 
| A06 | D02 | Data 02 | B06 | A04 | Address 04 | 
| A07 | D03 | Data 03 | B07 | A05 | Address 05 | 
| A08 | D04 | Data 04 | B08 | A06 | Address 06 | 
| A09 | D05 | Data 05 | B09 | A07 | Address 07 | 
| A10 | D06 | Data 06 | B10 | A08 | Address 08 | 
| A11 | D07 | Data 07 | B11 | A09 | Address 09 | 
| A12 | GND | Ground | B12 | GND | Ground | 
| A13 | D08 | Data 08 | B13 | A10 | Address 10 | 
| A14 | D09 | Data 09 | B14 | A11 | Address 11 | 
| A15 | D10 | Data 10 | B15 | A12 | Address 12 | 
| A16 | D11 | Data 11 | B16 | A13 | Address 13 | 
| A17 | D12 | Data 12 | B17 | A14 | Address 14 | 
| A18 | D13 | Data 13 | B18 | A15 | Address 15 | 
| A19 | D14 | Data 14 | B19 | A16 | Address 16 | 
| A20 | D15 | Data 15 | B20 | A17 | Address 17 | 
| A21 | /ROM | - | B21 | /LWR | (P-SRAM) | 
| A22 | /ASEL | (/LO_MEM) | - | B22 | /UWR | (P-SRAM) | 
| A23 | /RAS2 | - | B23 | /CAS2 |  | 
| A24 | /FDC | - | B24 | FDWR | [(2)](######(2)) | 
| A25 | FRES | - | B25 | A18 | Address 18 | 
| A26 | +5v | +5v DC | B26 | +5v | +5v DC | 
| A27 | SR3 | Audio Out (R\) [(4)](######(4)) | B27 | SL3 | Audio Out (L) [(4)](######(4)) | 
| A28 | VCC OUT | [(3)](######(3)) | B28 | VCC OUT | [(3)](######(3)) | 
| A29 | SR2 | Audio In (R\) [(5)](######(5)) | B29 | SL2 | Audio In (L) [(5)](######(5)) |  
| A30 | GND | Ground | B30 | GND | Ground | 

###### (1) Presumably signalling the presence of a CD?
###### (2) Function unknown
###### (3) VCC OUT - Power from Mega Drive/Genesis is routed through this pin to the Mega/Sega-CD and switches it on; detailed info [here](https://nfggames.com/forum2/index.php?topic=7145.msg48200#msg48200)
###### (4) Mega Drive to Mega-CD. Not connected on Mega Drive 1
###### (5) Mega-CD to Mega Drive
