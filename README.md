# Introduction

Le but de ce projet est de créer une extension permettant d'exécuter des jeux Megadrive et Master System sur une Megadrive depuis une carte SD, et plus tard des jeux Mega-CD également. 
Beaucoup de produits faisaint cela existent déjà dans le commerce. Le but de ce projet n'est pas de faire mieux, mais d'apprendre.

# Première version

### Hardware 1.0 (https://github.com/Pilou44/mega-sd/releases/tag/hw_1.0)

Pour débuter, je suis parti sur une extension se branchant à la Megadrive via le port d'extension, comme un Mega-CD. Le projet est architecturé autour d'une carte de test STM32 de chez DiyMore (https://www.diymore.cc/products/stm32f4-discovery-stm32f407vgt6-microcontroller-32bit-flash-mcu-arm-cortex-m4-core-development-board?_pos=33&_sid=8834dc3dc&_ss=r). 
Les bus d'adresse et et données, ainsi que les signaux /ROM, /LWR et /UWR sont transmises au STM32 via des émetteur-récepteur SN74LVC245 afin de convertir le 5V en 3.3V. Ils arrivent sur les ports PB, PC et PD.
Une alimentation externe 5V est là, et elle est pilotée par un transistor de façon à ce que le projet ne soit actif que quand le 5V de la Megadrive est détectée.
La carte SD est connectée via le SPI1 (ports PA).
Les ports PE sont dirigés vers un connecteur pour pouvoir ajouter une extension comme un FPGA.

Cette première version a un gros défaut : les SN74LVC245 sur le bus de données sont activés automatiquement par le connecteur de la Megadrive. Cela implique un conflit potentiel si une cartouche est insérée. Il ne faut donc jamais le brancher en même temps qu'une cartouche.
Un autre défaut (mineur) est que les lignes pour envoyer du son vers la Megadrive (SL2 et SR2) ne sont pas branchées (elle sont cependant sorties sur les broches d'un connecteur pour une utilisation optionelle). Cela empêche de profiter des jeux MD+ et MSU.

Une première limitation à l'utlisation du port d'extension plutot que le port cartouche est que le reset n'est pas reçu. Les jeux qui ont besoin de le détecter ne marcheront donc pas.
Enfin une autre grosse limitation technique est apparue : le bus d'adresse n'a que 19 lignes sur le port d'extension (A1 à A19) contre 23 sur le port cartouche. Il est donc impossible de lancer des jeux de plus de 1Mo.

Pour toutes ces raisons, le hardware 1.0 servira aux premiers tests pour afficher un menu et lancer des jeux de 1Mo maximum, mais sera probablement abandonné rapidement. Le hardware 2.0 utilisera probablement le port cartouche.
