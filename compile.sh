#!/bin/bash

## libuc
cd libuc
make clean
./autogen.sh
./configure --prefix=/opt/libuc
make
sudo make install

export PKG_CONFIG_PATH=/opt/libuc/lib/pkgconfig

## libuvideo
cd ../
cd libuvideo
./autogen.sh
./configure --prefix=/opt/libuc
make
sudo make install

## libtpm
cd ../
cd libtpm
./autogen.sh
./configure --prefix=/opt/libuc
make
sudo make install

## libtxt
cd ../
cd libtxt
./autogen.sh
./configure --prefix=/opt/libuc
make
sudo make install

## tloader
cd ../
cd tloader
./autogen.sh
./configure --prefix=/opt/libuc
make
