#!/bin/bash
rm ./contiki_cell.nativewithshm
make clean TARGET=nativewithshm
cp ./Makefile1 ./Makefile
make  TARGET=nativewithshm
mv ./contiki_cell.nativewithshm ../../../contiki_controller/Default/contikiint
rm ./contiki_cell.nativewithshm
make clean TARGET=nativewithshm
cp ./Makefile2 ./Makefile
make  TARGET=nativewithshm
mv ./contiki_cell.nativewithshm ../../../contiki_controller/Default/mcontikiint
