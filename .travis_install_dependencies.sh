#!/bin/bash

if [[ $TRAVIS_OS_NAME == 'trusty' ]]; then
	cd $TRAVIS_BUILD_DIR && cd ..
	# The SFML available in system repositories is older than v2.5.1...
	if [ ! -d $TRAVIS_BUILD_DIR/../SFML-$SFML_VER ]; then
		wget -O ~/sfmlv$SFML_VER.tar.gz https://github.com/SFML/SFML/archive/$SFML_VER.tar.gz
		tar -zxvf ~/sfmlv$SFML_VER.tar.gz -C ~/
	fi
	
	# ZPG Library is not available from system repositories
	if [ ! -d $TRAVIS_BUILD_DIR/../Zpg-master ]; then
		wget -O ~/zpgv1.0.0.zip https://gitlab.com/Tardo/Zpg/-/archive/master/Zpg-master.zip && unzip ~/zpgv1.0.0.zip
	fi
	
	# Compile & Install SFML and ZPG
	cd $TRAVIS_BUILD_DIR/../SFML-$SFML_VER; cmake -DCMAKE_BUILD_TYPE:STRING=$CMAKE_BUILD_TYPE -DBUILD_SHARED_LIBS:BOOL=FALSE -DCMAKE_INSTALL_PREFIX:PATH=/usr . && make && sudo make install
	cd $TRAVIS_BUILD_DIR/../Zpg-master; cmake -DCMAKE_BUILD_TYPE:STRING=$CMAKE_BUILD_TYPE -DCMAKE_INSTALL_PREFIX:PATH=/usr . && make Zpg && make zpg_packer && sudo make install
elif [[ $TRAVIS_OS_NAME == 'windows' ]]; then
	if [ ! -d $SFML_ROOT ]; then
		cd $TRAVIS_BUILD_DIR && cd ..
		curl -LfsS -o sfml.zip https://www.sfml-dev.org/files/$SFML_PACKAGE
	    7z x sfml.zip
	    mv SFML-$SFML_VER $SFML_ROOT
	fi
	
	if [ ! -d $ZLIB_ROOT ]; then
		cd $TRAVIS_BUILD_DIR && cd ..
		curl -LfsS -o zlib-1.2.11.tar.gz http://zlib.net/zlib-1.2.11.tar.gz
	    7z x zlib-1.2.11.tar.gz
	    7z x zlib-1.2.11.tar
	    cd zlib-1.2.11
	    mkdir build & cd build
	    cmake -Werror=dev -G$CMAKE_GENERATOR ..
	    cd ..
	    cmake --build build --config $CMAKE_BUILD_TYPE --target install
	fi
	
	if [ ! -d $ZPG_ROOT ]; then
		cd $TRAVIS_BUILD_DIR && cd ..
		curl -LfsS -o libzpg.zip https://gitlab.com/Tardo/Zpg/-/archive/master/Zpg-master.zip
	    7z x libzpg.zip
	    cd Zpg-master
	    mkdir build && cd build
	    cmake -Werror=dev -G$CMAKE_GENERATOR -DZLIB_ROOT=$ZLIB_ROOT -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE ..
	    cd ..
	    cmake --build build --config $CMAKE_BUILD_TYPE --target Zpg
	    cmake --build build --config $CMAKE_BUILD_TYPE --target zpg_packer
	    cmake --build build --config $CMAKE_BUILD_TYPE --target install
	fi
fi
