@echo off
masm -Mx -t -W1 -Ic:\winddk\3790\inc\ddk\w2k sample.asm;
link sample;
exe2bin sample.exe sample.com
copy sample.com c:\
