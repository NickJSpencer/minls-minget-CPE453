#/bin/bash
clear

rm *.o
rm minls

make minls

./minls $@
