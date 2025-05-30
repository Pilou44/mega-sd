# Sommaire / Table of contents

- [Version Française](#version-francaise)
- [English Version](#english-version)

# Version Française

## Introduction
Le but de ce projet est de créer une extension permettant d'exécuter des jeux **Mega Drive** et **Master System** sur une Mega Drive depuis une carte SD, et dans un second temps des jeux MD+ / MSU-MD, voir Mega-CD également.

Beaucoup de produits faisant cela existent déjà dans le commerce. Le but de ce projet n'est pas de faire mieux, mais d'apprendre.

## Première version
#### Hardware 1.0 ([Release hw_1.0](https://github.com/Pilou44/mega-sd/releases/tag/hw_1.0))
Pour débuter, je suis parti sur une extension se branchant à la Mega Drive via le port d'extension, comme un Mega-CD. L'avantage de ce port par rapport au port cartouche est qu'il dispose de wait-states qui font que l'on a de plus de temps pour répondre aux requêtes de la Mega Drive.

Le projet est architecturé autour d'une carte de test STM32 de chez DiyMore ([lien](https://www.diymore.cc/products/stm32f4-discovery-stm32f407vgt6-microcontroller-32bit-flash-mcu-arm-cortex-m4-core-development-board?_pos=33&_sid=8834dc3dc&_ss=r)).\
Les bus d'adresses et de données, ainsi que les signaux /ROM, /LWR et /UWR, sont transmis au STM32 via des émetteurs-récepteurs SN74LVC245 afin de convertir le 5V en 3.3V. Ils arrivent sur les ports PB, PC et PD.\
Une alimentation externe 5V est utilisée, et elle est pilotée par un transistor de façon à ce que le projet ne soit actif que lorsque le 5V de la Mega Drive est détecté.\
La carte SD est connectée via le SPI1 (ports PA).\
Les ports PE sont dirigés vers un connecteur pour pouvoir éventuellement ajouter une extension comme un FPGA.

Ce hardware présente quelques problèmes nécessitant des patches :
- Il y a un problème au niveau du switch gérant le mode de boot du STM32. La LED ne fonctionne pas et il est impossible de changer ce mode de boot sans l'alimentation externe.
- Il y a un problème de masse : elle est séparée en 2, et c'est le STM32 qui fait la jonction entre ces deux parties.
- La PIN 8 du STM32 n'est pas branchée aux autres PIN +3.3V du STM32.
- Il manque un port UART pour le debug (la carte n'est pas compatible ST-link).
- Le lecteur de carte SD peut avoir besoin d'être alimenté en 5V.

Un autre problème (non patchable) est que la largeur des pistes +3.3V du STM32 et +5V de l'USB ne sont pas bonnes.

Cette première version a un gros défaut : les SN74LVC245 sur le bus de données sont activés automatiquement par le connecteur de la Mega Drive. Cela implique un conflit potentiel si une cartouche est insérée. **Il ne faut donc jamais le brancher en même temps qu'une cartouche.**

Un autre défaut (mineur) est que les lignes pour envoyer du son vers la Mega Drive (SL2 et SR2) ne sont pas branchées au DAC du STM32. Elles sont cependant sorties sur les broches d'un connecteur pour une utilisation optionnelle. Cela empêche de profiter des jeux MD+ et MSU-MD.

Une première limitation à l'utilisation du port d'extension plutôt que du port cartouche est que le reset n'est pas reçu. Les jeux qui ont besoin de le détecter ne fonctionneront donc pas.

Une autre grosse limitation technique est apparue : le bus d'adresses n'a que 19 lignes sur le port d'extension (A1 à A19) contre 23 sur le port cartouche (A0 ne sort sur aucun port, il est toujours à la masse). Il est donc impossible de lancer des jeux de plus de 1 Mo.

Enfin, une dernière limitaiton (mais qui n'est pas liée à l'utilisation du port d'extension) est que le STM32 ne commute pas assez vite le sens de ses GPIO, il est donc impossible d'écrire de la console vers le STM32. **Celà implique qu'il ne faut pas lancer un jeu avec un système de sauvegarde.** Au moment de la sauvegarde, la console et le STM32 se retrouveraient en même temps en écriture sur le bus de données.

Pour toutes ces raisons, le hardware 1.0 servira aux premiers tests pour afficher un menu et lancer des jeux de 1 Mo maximum sans sauvegarde, mais sera probablement abandonné rapidement. Une version 1.1 sera conçue pour corriger tout ce qui a été patché. Le hardware 2.0 utilisera probablement le port cartouche, ce qui imposera de grosses modifications hardware.

### Conclusion sur cette version
Après des tests, les buffers choisis ne vont pas du tout. Les signaux 5V ne sont pas parfaitement isolés du 3.3V, entrainant des fuites qui font monter le +3.3V de la carte à +3.9V. Il n'existe pas dáutre buffers compatibles pin à pin. Il faudrait faire des adaptateurs pour en utiliser d'autres, et celà imposerait de nouveaux patches sur un PCB qui en a déjà trop.\
D'autre part, les wait-states du port d'extension sont très mal documentés. Impossible de trouver une documentation sur comment ils fonctionnent ou comment les utiliser. En revanche, le port cartouche dispose d'une pin /DTACK permettant de piloter des wait-states.\
Devant ces problèmes, la version 1 est abandonnée. Le projet va être entièrement repensé dans une V2 connectée au port cartouche.

## Deuxième version

Pour cette version, nous partons sur l'utilisation de port cartouche, interfacé avec le même STM32 que dans la précédente version. On prévoit des extensions possibles pour le décodage audio ou autre.

### Buffers
#### Choix
Au niveau des signaux nécessaires et des buffers adaptés, voila ce qui a été choisi :
- Bus de Données (D0-D15, bidir, MD 5V <-> STM32 3.3V) :
2 x 74LVC8T245 (SOIC sur adaptateur). VCCA=3.3V, VCCB=5V.
- Bus d'Adresses (A1-A23, MD 5V -> STM32 3.3V) :
3 x 74LVC8T245 (SOIC sur adaptateur). VCCA=3.3V, VCCB=5V, DIR fixé B vers A.
- Signaux de Contrôle (MD 5V -> STM32 3.3V, ~7 signaux) :
1 x 74LVC8T245 (SOIC sur adaptateur). VCCA=3.3V, VCCB=5V, DIR fixé B vers A.
- Signaux de Contrôle (STM32 3.3V -> MD 5V, 3 signaux : /DTACK, /M3, /CART) :
1 x 74HCT125 (Quad buffer, DIP ou SOIC). Alimenté en 5V. Entrées depuis STM32, sorties vers MD. Broches /OE liées à GND.

#### Connexion
**1. Buffers de Données (2 x 74LVC8T245 pour D0-D15)**

-   **VCCA :**  Connecté au 3.3V (alimentation du STM32).
-   **VCCB :**  Connecté au 5V (alimentation de la Mega Drive).
-   **GND :**  Connecté à la masse commune.
-   **Broches A (A1-A8 sur chaque CI) :**  Connectées aux GPIOs du STM32 pour D0-D15.
-   **Broches B (B1-B8 sur chaque CI) :**  Connectées aux broches D0-D15 du port cartouche de la Mega Drive.
-   **`/OE`  (Output Enable, actif BAS) :**
    -   Les deux broches  `/OE`  des deux 74LVC8T245 peuvent être connectées ensemble et pilotées par un seul signal du STM32.
    -   Ce signal OE_DATA (nommons-le ainsi) doit être  **BAS**  lorsque la Mega Drive s'adresse à la cartouche et qu'un transfert de données est en cours.
    -   Logique pour OE_DATA (sortie STM32, active BAS) :  `OE_DATA = NOT( /CEO_low AND /AS_low )`
        -   `/CEO`  (B17 du port cartouche, actif bas) : Indique que la cartouche est sélectionnée.
        -   `/AS`  (B18 du port cartouche, actif bas) : Indique qu'un cycle de bus est en cours.
        -   Donc, si  `/CEO`  est bas ET  `/AS`  est bas, alors OE_DATA doit être bas (buffers actifs). Sinon, OE_DATA est haut (buffers en haute impédance).
    -   Le STM32 lira  `/CEO`  et  `/AS`  (après translation de niveau 5V->3.3V) et générera le signal OE_DATA.
-   **`DIR`  (Direction) :**
    -   Les deux broches  `DIR`  des deux 74LVC8T245 peuvent être connectées ensemble et pilotées par un seul signal du STM32.
    -   Ce signal DIR_DATA (nommons-le ainsi) détermine le sens du transfert. La convention pour le 74LVC8T245 est souvent :
        -   DIR = HAUT (3.3V) => B vers A (Mega Drive vers STM32 - pour une lecture par le STM32, ou une écriture de la MD vers la SRAM émulée).
        -   DIR = BAS (0V) => A vers B (STM32 vers Mega Drive - pour une écriture du STM32, ou une lecture de la ROM par la MD).
    -   Logique pour DIR_DATA (sortie STM32) : Le STM32 doit déterminer si la Mega Drive effectue un cycle de lecture de la ROM/SRAM ou un cycle d'écriture vers la SRAM. On utilise la logique R/_W déduite :
        -   `is_md_write_cycle = (AS_low && (UWR_low || LWR_low))`
        -   `is_md_read_cycle = (AS_low && UWR_high && LWR_high)`  Donc :  `DIR_DATA = is_md_write_cycle`  (Si la MD écrit, on met DIR pour B->A. Si la MD lit, on met DIR pour A->B). Il faut vérifier la datasheet du 74LVC8T245 pour la polarité exacte de DIR (souvent DIR=1 => B vers A).

**2. Buffers d'Adresses (3 x 74LVC8T245 pour A1-A23)**

-   **VCCA :**  3.3V
-   **VCCB :**  5V
-   **GND :**  Masse
-   **Broches B (côté MD) :**  Connectées aux lignes A1-A23 du port cartouche.
-   **Broches A (côté STM32) :**  Connectées aux GPIOs du STM32 pour lire les adresses.
-   **`/OE`  :**
    -   Ces buffers doivent être actifs lorsque la console pilote le bus d'adresse et que le STM32 a besoin de lire cette adresse. En pratique, ils peuvent être actifs la plupart du temps où la cartouche est sélectionnée.
    -   Tu peux lier les  `/OE`  de ces 3 buffers ensemble.
    -   `OE_ADDR = NOT( /CEO_low AND /AS_low )`  (similaire à OE_DATA, ou simplement lié à la masse si tu veux qu'ils soient toujours actifs quand la carte est alimentée et que  `/CEO`  est géré par la Mega Drive). Pour plus de sécurité, le lier à  `/CEO`  (ou  `NOT /CEO`  si /OE est actif bas) est une bonne idée. Ou même le lier à la masse pour qu'ils soient toujours actifs, car ce sont des entrées pour le STM32, donc elles ne "piloteront" rien de manière conflictuelle.
    -   **Plus simple : Lier  `/OE`  à GND.**  Les buffers seront toujours actifs. Le STM32 lira simplement ce qui est sur le bus d'adresse de la MD.
-   **`DIR`  :**
    -   Doit être fixé pour la direction  **B vers A**  (Mega Drive vers STM32).
    -   Si DIR=1 => B vers A, alors lier les broches DIR de ces 3 CI à  **3.3V**  (VCCA).

**3. Buffer de Contrôle (MD 5V -> STM32 3.3V, 1 x 74LVC8T245 pour ~6 signaux)** * Signaux : `/AS`, `/LWR`, `/UWR`, `/CEO`, `/MRES`, `/VRES`, `/TIME`.

-   **VCCA :**  3.3V
-   **VCCB :**  5V
-   **GND :**  Masse
-   **Broches B (côté MD) :**  Connectées aux signaux de contrôle correspondants du port cartouche.
-   **Broches A (côté STM32) :**  Connectées aux GPIOs du STM32.
-   **`/OE`  : Lier à GND**  (toujours actif).
-   **`DIR`  : Lier à 3.3V**  (VCCA) (pour B vers A).
-   **Entrées inutilisées côté B :**  Lier à GND ou +5V sur le PCB.

**4. Buffer de Contrôle (STM32 3.3V -> MD 5V, 1 x 74HCT125 pour 3 signaux)** * Signaux : `/DTACK`, `/M3`, `/CART`.

-   **VCC :**  **5V**
-   **GND :**  Masse
-   **Entrées A (côté STM32) :**  Connectées aux GPIOs du STM32.
-   **Sorties Y (côté MD) :**  Connectées aux signaux  `/DTACK`,  `/M3`,  `/CART`  du port cartouche.
-   **`/OE`  (Output Enable, actif BAS) :**  Pour ces signaux, tu veux qu'ils soient toujours pilotés par le STM32 lorsque le buffer est alimenté. Donc,  **lier les broches  `/OE`  correspondantes à GND.**
-   **Entrée A inutilisée (s'il y en a une sur les 4 buffers) :**  Lier à GND ou 3.3V sur le PCB (via une résistance de pull-down/pull-up si tu préfères). La sortie Y correspondante sera alors définie mais non connectée. Son  `/OE`  peut être mis à HAUT (5V) pour désactiver cette sortie inutilisée.

**Attribution des GPIOs STM32 (en gardant Port B libre) :**

-   **SPI1 SD Card :** `PA4 (CS_SD), PA5 (SCK), PA6 (MISO), PA7 (MOSI)` - **OK (4 pins)**
    
-   **LED :** `PA0` - **OK (1 pin)**
    
-   **UART Debug :** `PA2 (TX), PA3 (RX)` - **OK (2 pins)**
    
-   **Bus Adresse (A1-A23, MD->STM32, 23 pins) :**
    
    -   `PC0-PC13`  (14 pins pour A1-A14) -  **OK.**
    -   `PD0-PD8`  (9 pins pour A15-A23) -  **OK.**
-   **Bus Données (D0-D15, MD<->STM32, 16 pins) :**
    
    -   `PE0-PE15`  -  **OK.**
-   **Contrôle (MD->STM32, 7 pins) :**
    
    -   `/CEO`  ->  `PD9`
    -   `/AS`  ->  `PD10`
    -   `/LWR`  ->  `PD11`
    -   `/UWR`  ->  `PD12`
    -   `/MRES`  ->  `PD13`
    -   `/VRES`  ->  `PD14`
    -   `/TIME`  ->  `PD15`
    -   **OK.** 
-   **Contrôle (STM32->MD, 3 pins) :**
    
    -   `/DTACK_OUT`  ->  `PA1`  (par exemple)
    -   `/M3_OUT`  ->  `PA8`  (par exemple)
    -   `/CART_OUT`  ->  `PA9`  (par exemple)
    -   **OK.**  (3 pins sur PA)
-   **Contrôle Buffers de Données (STM32->Buffers, 2 pins) :**
    
    -   `nOE_DATA_BUFFERS_OUT`  ->  `PA10`  (par exemple)
    -   `DIR_DATA_BUFFERS_OUT`  ->  `PA11`  (par exemple)
    -   **OK.**  (2 pins sur PA)

# English Version

## Introduction
The goal of this project is to create an extension that allows you to run **Mega Drive** and **Master System** games on a Mega Drive from an SD card, and later also Mega-CD games.

Many commercial products already do this. The purpose of this project is not to make something better, but to learn.

## First Version
### Hardware 1.0  ([Release hw_1.0](https://github.com/Pilou44/mega-sd/releases/tag/hw_1.0))
To start, I chose to make an extension that connects to the Mega Drive via the expansion port, like a Mega-CD.

The project is based on a DiyMore STM32 test board ([link](https://www.diymore.cc/products/stm32f4-discovery-stm32f407vgt6-microcontroller-32bit-flash-mcu-arm-cortex-m4-core-development-board?_pos=33&_sid=8834dc3dc&_ss=r)).\
The address and data buses, as well as the /ROM, /LWR and /UWR signals, are sent to the STM32 via SN74LVC245 transceivers to convert 5V to 3.3V. They arrive on the PB, PC, and PD ports.\
An external 5V power supply is used and is controlled by a transistor so that the project is only active when the 5V from the Mega Drive is detected.\
The SD card is connected via SPI1 (PA ports).\
The PE ports are routed to a connector for potential future expansion with an FPGA.

This hardware presents a few issues requiring patches:
- There is an issue with the switch managing the STM32 boot mode. The LED does not work, and it is impossible to change the boot mode without the external power supply.
- There is a ground problem: the ground plane is split in two, and only the STM32 bridges these two areas.
- Pin 8 of the STM32 is not connected to the other +3.3V pins of the STM32.

Another (unpatchable) issue is that the track width for the +3.3V STM32 and the USB +5V are not adequate.

This first version has a major flaw: the SN74LVC245 on the data bus are automatically enabled by the Mega Drive connector. This creates a potential conflict if a cartridge is inserted. **So you must never connect this in parallel with a cartridge.**

Another (minor) flaw is that the lines used to send sound to the Mega Drive (SL2 and SR2) are not connected (they are however available on a connector for optional use). This prevents using MD+ and MSU games.

A first limitation of using the expansion port instead of the cartridge port is that the reset signal is not received. Games that need to detect reset will not work correctly.

Finally, another major technical limitation appeared: the address bus only has 19 lines on the expansion port (A1 to A19) compared to 23 on the cartridge port. It is therefore impossible to run games larger than 1MB.

For all these reasons, hardware 1.0 will serve for initial tests to display a menu and run games up to 1MB in size, but will probably be abandoned quickly. Hardware 2.0 will likely use the cartridge port.
