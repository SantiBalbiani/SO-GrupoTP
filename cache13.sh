#!/bin/bash

git clone https://github.com/sisoputnfrba/cache13-scripts.git
git clone https://github.com/sisoputnfrba/so-commons-library.git

cd so-commons-library
sudo make install
cd ../tp-2015-2c-los-simuladores/sharedLibrary/
make clean
make all
sudo make install
 
cd ../Swap/
make clean
make all

cd ../Adm/
make clean
make all

cd ../Cpu/
make clean
make all

cd ../Planificador/
make clean
make all














