#!/bin/bash

# sources:
# https://stackoverflow.com/questions/6431451/how-do-i-check-if-a-path-is-set-and-if-not-set-it-from-an-argument

deref() {
    echo "${!1}";
}

add_to_path () {
    case ":$(deref "$1"):" in
      *"$2"*) :;;
      *) echo "export $1=\$$1:$2" >> "$3"
    esac
}

FILE="$HOME/.bashrc"

git clone https://github.com/pmelsted/bifrost.git

XSMASH_PATH=$(pwd)
add_to_path "C_INCLUDE_PATH" "$XSMASH_PATH/include/" "$FILE"
add_to_path "CPLUS_INCLUDE_PATH" "$XSMASH_PATH/include/" "$FILE"
add_to_path "LD_LIBRARY_PATH" "$XSMASH_PATH/lib/" "$FILE"
add_to_path "LIBRARY_PATH" "$XSMASH_PATH/lib/" "$FILE"
add_to_path "PATH" "$XSMASH_PATH/lib" "$FILE"

source "$HOME/.bashrc"

cd bifrost
git reset --hard a05730f6285f5a4a7fc579508c5edbea661050e2
mkdir build && cd build || exit
cmake -DCMAKE_INSTALL_PREFIX="$XSMASH_PATH" ..
make
make install

cd $XSMASH_PATH
rm -rf bifrost
make
