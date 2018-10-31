#!/bin/bash

# The SFML available in system repositories is older than v2.5.1...
if [ ! -d ~/SFML-$SFML_VER ]; then
	wget -O ~/sfmlv$SFML_VER.tar.gz https://github.com/SFML/SFML/archive/$SFML_vER.tar.gz
	tar -zxvf ~/sfmlv$SFML_VER.tar.gz -C ~/
fi
# ZPG Library is not available from system repositories
wget -O ~/zpgv1.0.0.zip https://gitlab.com/Tardo/Zpg/-/archive/master/Zpg-master.zip && unzip ~/zpgv1.0.0.zip -d ~/

# Compile & Install SFML and ZPG
cd ~/SFML-$SFML_VER; cmake -DBUILD_SHARED_LIBS:BOOL=FALSE -DCMAKE_INSTALL_PREFIX:PATH=/usr . && make && sudo make install
cd ~/Zpg-master; cmake -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_INSTALL_PREFIX:PATH=/usr . && make Zpg && make zpg_packer && sudo make install
