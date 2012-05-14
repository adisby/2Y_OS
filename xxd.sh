#!/bin/sh
echo "sector map:"
xxd -u -a -g l -c 16 -s 0x9E0400 -l 512 80m.img
#echo "/:"
#xxd -u -a -g l -c 16 -s 0xA21800 -l 512 80m.img
echo "pwd:"
xxd -u -a -g l -c 16 -s 0xBD3E00 -l 512 80m.img
