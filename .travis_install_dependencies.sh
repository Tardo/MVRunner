#!/bin/bash

# Install necessary dependencies
sudo apt-get install -y cmake zlib1g-dev libfreetype6-dev libx11-dev libxrandr-dev libxcb1-dev libx11-xcb-dev libxcb-randr0-dev libxcb-image0-dev libflac-dev libogg-dev libvorbis-dev libvorbisenc2 libvorbisfile3 libopenal-dev libudev-dev libsndfile1-dev libglew-dev

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

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then

    # Install some custom requirements on OS X
    # e.g. brew install pyenv-virtualenv

    case "${TOXENV}" in
        py32)
            # Install some custom Python 3.2 requirements on OS X
            ;;
        py33)
            # Install some custom Python 3.3 requirements on OS X
            ;;
    esac
else
    # Install some custom requirements on Linux
fi
