@echo off
  as68k q1.src
  cc /9 q

  link68k -otest.abs q1.o q.o cc68help.lib
  dl68k /otest.bin /ccode;data;string test.abs

echo .
echo running simulator
echo .
  sim68 /d /M16 test.bin