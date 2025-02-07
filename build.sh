#!/bin/bash
###
 # @Author: Ynt
 # @Date: 2024-11-16 17:54:23
 # @LastEditTime: 2024-11-20 10:46:32
 # @Description: 
### 

set -e

if [ ! -d `pwd`/build ]; then
    sudo mkdir `pwd`/build
fi

sudo rm -rf `pwd`/build/*

cd `pwd`/build &&
    cmake .. &&
    make

cd ..

if [ ! -d /usr/include/mymuduo ]; then
    sudo mkdir /usr/include/mymuduo
fi

for header in  `ls *.h`
do 
    sudo cp $header /usr/include/mymuduo
done

sudo cp `pwd`/lib/libmymuduo.so /usr/lib

sudo ldconfig