#!/bin/bash

cd $TRAVIS_BUILD_DIR
mkdir build-$BUILD_TYPE/ && cd build-$BUILD_TYPE/
cmake -DCMAKE_BUILD_TYPE:STRING=$BUILD_TYPE .. && make && make create_zpg
cpack --debug --verbose -C CPackConfig.cmake