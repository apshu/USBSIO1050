/* This small code implements the bootloader jumps, imitates latency */ 
asm("\tpsect myreset,abs,ovrld,class=CODE,delta=2,keep,pure,reloc=0");
asm("\torg 0");
asm("\tmovlp high 0x200");
asm("\tgoto	0x200");
asm("\tdw 0x3fff");
asm("\tdw 0x3fff");
asm("\tmovlp high 0x204");
asm("\tgoto	0x204");

