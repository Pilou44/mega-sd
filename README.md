# Sommaire / Table of contents

- [Version Française](#version-francaise)
- [English Version](#english-version)

# Version Française

## Introduction
Le but de ce projet est de créer une extension permettant d'exécuter des jeux **Mega Drive** et **Master System** sur une Mega Drive depuis une carte SD, et plus tard des jeux Mega-CD également.

Beaucoup de produits faisant cela existent déjà dans le commerce. Le but de ce projet n'est pas de faire mieux, mais d'apprendre.

## Première version
#### Hardware 1.0 ([Release hw_1.0](https://github.com/Pilou44/mega-sd/releases/tag/hw_1.0))
Pour débuter, je suis parti sur une extension se branchant à la Mega Drive via le port d'extension, comme un Mega-CD.

Le projet est architecturé autour d'une carte de test STM32 de chez DiyMore ([lien](https://www.diymore.cc/products/stm32f4-discovery-stm32f407vgt6-microcontroller-32bit-flash-mcu-arm-cortex-m4-core-development-board?_pos=33&_sid=8834dc3dc&_ss=r)).\
Les bus d'adresses et de données, ainsi que les signaux /ROM, /LWR et /UWR sont transmis au STM32 via des émetteurs-récepteurs SN74LVC245 afin de convertir le 5V en 3.3V. Ils arrivent sur les ports PB, PC et PD.\
Une alimentation externe 5V est utilisée, et elle est pilotée par un transistor de façon à ce que le projet ne soit actif que lorsque le 5V de la Mega Drive est détecté.\
La carte SD est connectée via le SPI1 (ports PA).\
Les ports PE sont dirigés vers un connecteur pour pouvoir ajouter une extension comme un FPGA.

Ce hardware présente quelques problèmes nécessitant des patches :
- Il y a un problème au niveau du switch gérant le mode de boot du STM32. La LED ne fonctionne pas et il est impossible de changer ce mode de boot sans l'alimentation externe.
- Il y a un problème de masse : elle est séparée en 2, et c'est le STM32 qui fait la jonction entre ces deux parties.
- La PIN 8 du STM32 n'est pas branchée aux autres PIN +3.3V du STM32.\
Un autre problème (non patchable) est que la largeur des pistes +3.3V du STM32 et +5V de l'USB ne sont pas bonnes.

Cette première version a un gros défaut : les SN74LVC245 sur le bus de données sont activés automatiquement par le connecteur de la Mega Drive. Cela implique un conflit potentiel si une cartouche est insérée. **Il ne faut donc jamais le brancher en même temps qu'une cartouche.**

Un autre défaut (mineur) est que les lignes pour envoyer du son vers la Mega Drive (SL2 et SR2) ne sont pas branchées (elles sont cependant sorties sur les broches d'un connecteur pour une utilisation optionnelle). Cela empêche de profiter des jeux MD+ et MSU-MD.

Une première limitation à l'utilisation du port d'extension plutôt que du port cartouche est que le reset n'est pas reçu. Les jeux qui ont besoin de le détecter ne fonctionneront donc pas.

Enfin, une autre grosse limitation technique est apparue : le bus d'adresses n'a que 19 lignes sur le port d'extension (A1 à A19) contre 23 sur le port cartouche. Il est donc impossible de lancer des jeux de plus de 1 Mo.

Pour toutes ces raisons, le hardware 1.0 servira aux premiers tests pour afficher un menu et lancer des jeux de 1 Mo maximum, mais sera probablement abandonné rapidement. Le hardware 2.0 utilisera probablement le port cartouche.

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
- Pin 8 of the STM32 is not connected to the other +3.3V pins of the STM32.\
Another (unpatchable) issue is that the track width for the +3.3V STM32 and the USB +5V are not adequate.

This first version has a major flaw: the SN74LVC245 on the data bus are automatically enabled by the Mega Drive connector. This creates a potential conflict if a cartridge is inserted. **So you must never connect this in parallel with a cartridge.**

Another (minor) flaw is that the lines used to send sound to the Mega Drive (SL2 and SR2) are not connected (they are however available on a connector for optional use). This prevents using MD+ and MSU games.

A first limitation of using the expansion port instead of the cartridge port is that the reset signal is not received. Games that need to detect reset will not work correctly.

Finally, another major technical limitation appeared: the address bus only has 19 lines on the expansion port (A1 to A19) compared to 23 on the cartridge port. It is therefore impossible to run games larger than 1MB.

For all these reasons, hardware 1.0 will serve for initial tests to display a menu and run games up to 1MB in size, but will probably be abandoned quickly. Hardware 2.0 will likely use the cartridge port.
