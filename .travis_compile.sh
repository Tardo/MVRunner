#!/bin/bash

cd $TRAVIS_BUILD_DIR
mkdir build-$CMAKE_BUILD_TYPE/ && cd build-$CMAKE_BUILD_TYPE/
cmake -DCMAKE_BUILD_TYPE:STRING=$CMAKE_BUILD_TYPE .. && make && make create_zpg
cpack --debug --verbose -C CPackConfig.cmake