#/bin/bash

rm *.o
rm minls

make minls

./minls $@
