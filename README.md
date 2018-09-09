# MVRunner [![Build Status](https://travis-ci.org/Tardo/MVRunner.svg?branch=master)](https://travis-ci.org/Tardo/MVRunner) [![Build status](https://ci.appveyor.com/api/projects/status/5xiw82neaa8ihqkv?svg=true)](https://ci.appveyor.com/project/Tardo/mvrunner)
_A game inspired in Teeworlds and DDRace_

# **UNDER DEVELOPMENT**

## - DEPENDENCIES
- SFML v2.5 (Not Included)
- ZPG (Not Included)
- TMXParser (Included)
- TinyXML2 (Included)
- LiquidFun (Included)

# COMPILATION PROCESS (LINUX)
#### Install SFML v2.5 (Static Mode)
```
sudo apt-get install cmake zlib1g-dev libfreetype6-dev libx11-dev libxrandr-dev libxcb1-dev libx11-xcb-dev libxcb-randr0-dev libxcb-image0-dev libflac-dev libogg-dev libvorbis-dev libvorbisenc2 libvorbisfile3 libopenal-dev libudev-dev libsndfile1-dev libglew-dev
wget -O ~/sfmlv2.5.0.tar.gz https://github.com/SFML/SFML/archive/2.5.0.tar.gz
tar -zxvf ~/sfmlv2.5.0.tar.gz -C ~/
cd ~/SFML-2.5.0
cmake -DBUILD_SHARED_LIBS:BOOL=FALSE -DCMAKE_INSTALL_PREFIX:PATH=/usr .
make
sudo make install
```
#### Install ZPG
```
wget -O ~/zpgv1.0.0.zip https://gitlab.com/Tardo/Zpg/-/archive/master/Zpg-master.zip
unzip ~/zpgv1.0.0.zip -d ~/
cd ~/Zpg-master
cmake -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_INSTALL_PREFIX:PATH=/usr .
make Zpg
make zpg_packer
sudo make install
```
### Compile Game
```
git clone https://github.com/Tardo/MVRunner.git --depth=1
cd MVRunner/
mkdir build-release/
cd build-release/
cmake -DCMAKE_BUILD_TYPE:STRING=Release ..
make
```
-  ### Create ZPG Assets Package
  ```
  make create_zpg
  ```
-  ### Create Distribution Packages (Optional)
  ```
  cpack --debug --verbose -C CPackConfig.cmake
  ```
