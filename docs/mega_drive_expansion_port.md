Expansion port

Address:
B03	A01	Address 01
B04	A02	Address 02
B05	A03	Address 03
B06	A04	Address 04
B07	A05	Address 05
B08	A06	Address 06
B09	A07	Address 07
B10	A08	Address 08
B11	A09	Address 09
B13	A10	Address 10
B14	A11	Address 11
B15	A12	Address 12
B16	A13	Address 13
B17	A14	Address 14
B18	A15	Address 15
B19	A16	Address 16
B20	A17	Address 17
B25	A18 Address 18
A02	A19	Address 19

Data:
A04	D00	Data 00
A05	D01	Data 01
A06	D02	Data 02
A07	D03	Data 03
A08	D04	Data 04
A09	D05	Data 05
A10	D06	Data 06
A11	D07	Data 07
A13	D08	Data 08
A14	D09	Data 09
A15	D10	Data 10
A16	D11	Data 11
A17	D12	Data 12
A18	D13	Data 13
A19	D14	Data 14
A20	D15	Data 15

Control:
B02	/DISK (Presumably signalling the presence of a CD?)
A21	/ROM
B21	/LWR	(P-SRAM)
A03	/CAS0
A22	/ASEL	(/LO_MEM)
B22	/UWR	(P-SRAM)
A23	/RAS2
B23	/CAS2
A24	/FDC
B24	FDWR (Function unknown)
A25	FRES

Ground:
A01	GND	Ground
B01	GND	Ground
A12	GND	Ground
B12	GND	Ground
A30	GND	Ground
B30	GND	Ground

+5V:
A26	+5v	+5v DC
B26	+5v	+5v DC

VCC: (Power from Mega Drive/Genesis is routed through this pin to the Mega/Sega-CD and switches it on; detailed info https://nfggames.com/forum2/index.php?topic=7145.msg48200#msg48200)
A28	VCC OUT
B28	VCC OUT

Audio:
A27	SR3	Audio Out (R)
B27	SL3	Audio Out (L)
A29	SR2	Audio In (R)
B29	SL2	Audio In (L)
