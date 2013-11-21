#/bin/bash

HULK_SRC_DIR=$(pwd)
HULK_BUILD_DIR=$1/build
HULK_INSTALL_DIR=$1/install

mkdir -p $HULK_BUILD_DIR/core/debug
cd $HULK_BUILD_DIR
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$HULK_INSTALL_DIR/core/debug $HULK_SRC_DIR
make all install
