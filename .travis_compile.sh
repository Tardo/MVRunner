#!/bin/bash

cd $TRAVIS_BUILD_DIR

if [[ $TRAVIS_OS_NAME == 'linux' ]]; then
	mkdir build-$CMAKE_BUILD_TYPE/ && cd build-$CMAKE_BUILD_TYPE/
	cmake -DCMAKE_BUILD_TYPE:STRING=$CMAKE_BUILD_TYPE -DZPG_PACKER_BIN="$ZPG_PACKER_BIN" -DCMAKE_CXX_FLAGS="-m$TARGET_BITS" -DCMAKE_CXX_FLAGS="-m$TARGET_BITS" -DCMAKE_C_FLAGS=-m$TARGET_BITS -DCMAKE_SIZEOF_VOID_P=$((TARGET_BITS/8)) ..
	make && make create_zpg
	cpack --debug --verbose -C CPackConfig.cmake
elif [[ $TRAVIS_OS_NAME == 'windows' ]]; then
	mkdir build/ && cd build/
    cmake -Werror=dev -G "$CMAKE_GENERATOR" -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE -DZLIB_ROOT="$ZLIB_ROOT" -DZPG_PACKER_BIN="$ZPG_PACKER_BIN" -DCMAKE_SIZEOF_VOID_P=$((TARGET_BITS/8)) ..
    cd ..
    cmake --build build --config $CMAKE_BUILD_TYPE --target MVRunner
    cmake --build build --config $CMAKE_BUILD_TYPE --target create_zpg
    cmake --build build --config $CMAKE_BUILD_TYPE --target package
fi