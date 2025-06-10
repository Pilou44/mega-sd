### Tableau 1 : Signaux entre la Mega Drive (Port Cartouche) et le STM32

| Signal Mega Drive | Broche Port Cartouche | Description                                          | Sens        | GPIO STM32 | Nom du Buffer Traversé |
|-------------------|-----------------------|------------------------------------------------------|-------------|------------|------------------------|
| **Signaux d'Adresse (A01-A23)** |                       |                                                      |             |            |                        |
| A08               | A03                   | Address 8                                            | MD -> STM32 | PD9        | ADDRESS_3              |
| A11               | A04                   | Address 11                                           | MD -> STM32 | PD2        | ADDRESS_3              |
| A07               | A05                   | Address 7                                            | MD -> STM32 | PD7        | ADDRESS_3              |
| A12               | A06                   | Address 12                                           | MD -> STM32 | PD6        | ADDRESS_3              |
| A06               | A07                   | Address 6                                            | MD -> STM32 | PD0        | ADDRESS_2              |
| A13               | A08                   | Address 13                                           | MD -> STM32 | PC11       | ADDRESS_2              |
| A05               | A09                   | Address 5                                            | MD -> STM32 | PC9        | ADDRESS_2              |
| A14               | A10                   | Address 14                                           | MD -> STM32 | PC7        | ADDRESS_2              |
| A04               | A11                   | Address 4                                            | MD -> STM32 | PC8        | ADDRESS_1              |
| A15               | A12                   | Address 15                                           | MD -> STM32 | PD15       | ADDRESS_1              |
| A03               | A13                   | Address 3                                            | MD -> STM32 | PD14       | ADDRESS_1              |
| A16               | A14                   | Address 16                                           | MD -> STM32 | PD13       | ADDRESS_1              |
| A02               | A15                   | Address 2                                            | MD -> STM32 | PD12       | ADDRESS_1              |
| A17               | A16                   | Address 17                                           | MD -> STM32 | PD11       | ADDRESS_1              |
| A01               | A17                   | Address 1                                            | MD -> STM32 | PD10       | ADDRESS_1              |
| A09               | B04                   | Address 09                                           | MD -> STM32 | PD3        | ADDRESS_3              |
| A10               | B05                   | Address 10                                           | MD -> STM32 | PD5        | ADDRESS_3              |
| A18               | B06                   | Address 18                                           | MD -> STM32 | PD4        | ADDRESS_3              |
| A19               | B07                   | Address 19                                           | MD -> STM32 | PD1        | ADDRESS_2              |
| A20               | B08                   | Address 20                                           | MD -> STM32 | PC12       | ADDRESS_2              |
| A21               | B09                   | Address 21                                           | MD -> STM32 | PC10       | ADDRESS_2              |
| A22               | B10                   | Address 22                                           | MD -> STM32 | PC13       | ADDRESS_2              |
| A23               | B11                   | Address 23                                           | MD -> STM32 | PC6        | ADDRESS_1              |
| **Signaux de Données (D00-D15)** |                       |                                                      |             |            |                        |
| D07               | A19                   | Data 7                                               | MD <-> STM32 | PE12       | DATA_2                 |
| D00               | A20                   | Data 0                                               | MD <-> STM32 | PE14       | DATA_2                 |
| D08               | A21                   | Data 8                                               | MD <-> STM32 | PE10       | DATA_2                 |
| D06               | A22                   | Data 6                                               | MD <-> STM32 | PE11       | DATA_2                 |
| D01               | A23                   | Data 1                                               | MD <-> STM32 | PE8        | DATA_2                 |
| D09               | A24                   | Data 9                                               | MD <-> STM32 | PE7        | DATA_1                 |
| D05               | A25                   | Data 5                                               | MD <-> STM32 | PE1        | DATA_1                 |
| D02               | A26                   | Data 2                                               | MD <-> STM32 | PE0        | DATA_1                 |
| D10               | A27                   | Data 10                                              | MD <-> STM32 | PE9        | DATA_2                 |
| D04               | A28                   | Data 4                                               | MD <-> STM32 | PE4        | DATA_1                 |
| D03               | A29                   | Data 3                                               | MD <-> STM32 | PE6        | DATA_1                 |
| D11               | A30                   | Data 11                                              | MD <-> STM32 | PE2        | DATA_1                 |
| D15               | B22                   | Data 15                                              | MD <-> STM32 | PE13       | DATA_2                 |
| D14               | B23                   | Data 14                                              | MD <-> STM32 | PE15       | DATA_2                 |
| D13               | B24                   | Data 13                                              | MD <-> STM32 | PE3        | DATA_1                 |
| D12               | B25                   | Data 12                                              | MD <-> STM32 | PE5        | DATA_1                 |
| **Signaux de Contrôle et Autres** |                       |                                                      |             |            |                        |
| SL1               | B01                   | Left Audio                                           | MD -> STM32 | PC0        | CTRL_IN                |
| /MRES             | B02                   | Master reset                                         | MD -> STM32 | PC1        | CTRL_IN                |
| SR1               | B03                   | Right Audio                                          | MD -> STM32 | PC2        | CTRL_IN                |
| /YS               | B12                   | VDP is currently drawing the backdrop colour         | N/A         | N/A        | N/A                    |
| /VSYNC            | B13                   | Vertical sync pulse                                  | N/A         | N/A        | N/A                    |
| /HSYNC            | B14                   | Horizonal sync pulse                                 | N/A         | N/A        | N/A                    |
| EDCLK             | B15                   | External Dot Clock (~13.4 or 10.7 MHz)               | N/A         | N/A        | N/A                    |
| /CAS0             | B16                   | Read or Write on $000000-$DFFFFF region              | MD -> STM32 | PC3        | CTRL_IN                |
| /CEO              | B17                   | Chip Enable                                          | MD -> STM32 | PC4        | CTRL_IN                |
| /AS               | B18                   | Address strobe                                       | MD -> STM32 | PC5        | CTRL_IN                |
| VCLK              | B19                   | 68K Clock                                            | N/A         | N/A        | N/A                    |
| /DTACK            | B20                   | Data acknowledge to 68K                              | STM32 -> MD | PA1        | CTRL_OUT               |
| /CAS2             | B21                   | Read or Write on $E00000-$FFFFFF region, maybe       | N/A         | N/A        | N/A                    |
| /ASEL             | B26                   | Read or Write on $000000-$7FFFFF region              | N/A         | N/A        | N/A                    |
| /VRES             | B27                   | System reset                                         | MD -> STM32 | N/A        | CTRL_IN (Note 1)       |
| /LWR              | B28                   | -                                                    | MD -> STM32 | N/A        | CTRL_IN (Note 1)       |
| /UWR              | B29                   | -                                                    | MD -> STM32 | N/A        | CTRL_IN (Note 1)       |
| /M3               | B30                   | Master System mode                                   | STM32 -> MD | PA9        | CTRL_OUT               |
| /TIME             | B31                   | RTC Chip Enable / SRAM CS (e.g. Sonic 3)             | MD -> STM32 | N/A        | CTRL_IN (Note 1)       |
| /CART             | B32                   | Cartridge Presence                                   | STM32 -> MD | PA8        | CTRL_OUT               |

---

### Tableau 2 : Configuration des Broches DIR et /OE des Buffers

| Nom du Buffer (Suggestion) | CI Utilisé (Suggestion) | Signal(s) Géré(s)                                  | Broche DIR Connectée à                                                      | Broche /OE Connectée à                                                              |
| :------------------------- | :---------------------- | :------------------------------------------------- | :-------------------------------------------------------------------------- | :-------------------------------------------------------------------------------- |
| ADDRESS_BUF_1              | 74LVC8T245              | A01-A08 (partiel)                                  | GND  (Fixe pour B->A)                                                | GND (Toujours actif)                                                              |
| ADDRESS_BUF_2              | 74LVC8T245              | A09-A16 (partiel)                                  | GND  (Fixe pour B->A)                                                | GND (Toujours actif)                                                              |
| ADDRESS_BUF_3              | 74LVC8T245              | A17-A23 (7 bits)                                   | GND  (Fixe pour B->A)                                                | GND (Toujours actif)                                                              |
| DATA_BUF_1                 | 74LVC8T245              | D00-D07                                            | `STM32_DIR_DATA_BUFFERS_OUT` (ex: PA11)                                     | `STM32_nOE_DATA_BUFFERS_OUT` (ex: PA10, actif BAS)                                |
| DATA_BUF_2                 | 74LVC8T245              | D08-D15                                            | `STM32_DIR_DATA_BUFFERS_OUT` (ex: PA11)                                     | `STM32_nOE_DATA_BUFFERS_OUT` (ex: PA10, actif BAS)                                |
| CTRL_IN_BUF                | 74LVC8T245              | /CEO, /AS, /LWR, /UWR, /MRES, /VRES, /TIME (7 bits) | GND  (Fixe pour B->A)                                                | GND (Toujours actif)                                                              |
| CTRL_OUT_BUF               | 74HCT125                | /DTACK, /M3, /CART (3 bits)                        | N/A (Unidirectionnel A->Y)                                                  | GND (pour les 3 buffers utilisés ; `/OE` du 4ème buffer inutilisé à +5V)          |

**Notes pour les entrées inutilisées des buffers :**
*   **ADDRESS_BUF_3 :** L'entrée B8 (côté MD, 5V) est inutilisée. À lier à GND ou +5V sur le PCB.
*   **CTRL_IN_BUF :** L'entrée B8 (côté MD, 5V) est inutilisée. À lier à GND ou +5V sur le PCB.
*   **CTRL_OUT_BUF (74HCT125) :** L'entrée A du 4ème buffer inutilisé doit être liée à GND ou 3.3V. Sa broche `/OE` correspondante doit être liée à +5V (VCC du 74HCT125).

---

### Tableau 3 : Liste des Signaux de la Mega Drive (Port Cartouche) à Router vers un Header d'Extension (Optionnel)

| Broche Port Cartouche | Nom Signal | Description                                     | Commentaire pour l'Extension                                                                                              |
| :-------------------- | :--------- | :---------------------------------------------- | :------------------------------------------------------------------------------------------------------------------------ |
| B01                   | SL1        | Left Audio In                                   | Signal audio analogique. Router directement vers le header (ne pas bufferiser avec un CI logique).                       |
| B03                   | SR1        | Right Audio In                                  | Signal audio analogique. Router directement vers le header.                                                               |
| B12                   | /YS        | VDP is currently drawing the backdrop colour    | Signal logique 5V. L'extension devra prévoir une translation si besoin.                                                   |
| B13                   | /VSYNC     | Vertical sync pulse                             | Signal logique 5V.                                                                                                        |
| B14                   | /HSYNC     | Horizonal sync pulse                            | Signal logique 5V.                                                                                                        |
| B15                   | EDCLK      | External Dot Clock (~13.4 or 10.7 MHz)          | Signal logique 5V (horloge).                                                                                              |
| B16                   | /CAS0      | Read or Write on $000000-$DFFFFF region         | Signal logique 5V.                                                                                                        |
| B19                   | VCLK       | 68K Clock                                       | Signal logique 5V (horloge).                                                                                              |
| B21                   | /CAS2      | Read or Write on $E00000-$FFFFFF region, maybe | Signal logique 5V.                                                                                                        |
| B26                   | /ASEL      | Read or Write on $000000-$7FFFFF region         | Signal logique 5V.                                                                                                        |
| A02, A31              | +5V        | +5V DC                                          | Alimentation à fournir sur le header.                                                                                     |
| A01, A18, A32         | GND        | Ground                                          | Masse à fournir sur le header.                                                                                            |
| _N/A_                 | +3.3V      | (Généré sur ton PCB par le LF33CV)              | Alimentation 3.3V à fournir sur le header.                                                                                |
| _N/A_                 | +5V_EXT    | (Si tu as une alimentation externe pour ton PCB)  | Alimentation 5V (externe) à fournir sur le header si différente du +5V de la MD.                                        |
