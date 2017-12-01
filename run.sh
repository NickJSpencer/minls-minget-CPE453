#/bin/bash

rm *.o
rm minls

make minget

./minget $@
