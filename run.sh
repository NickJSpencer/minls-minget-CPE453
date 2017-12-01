#/bin/bash

clear
rm *.o
rm minls

make minget

echo "Running..."
./minget $@
