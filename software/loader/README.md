Pour compiler :
clownassembler_asm68k.exe /p main.asm,rom.bin

On utilise l'outil de SGDK ou un autre fix-checksum :
java -jar C:/Users/guill/sgdk/bin/sizebnd.jar rom.bin -sizealign 2048 -checksum
