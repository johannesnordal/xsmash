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

rm -rf local

git clone https://github.com/pmelsted/bifrost.git

XSMASH_PATH=$(pwd)
mkdir local
add_to_path "C_INCLUDE_PATH" "$XSMASH_PATH/local/include/" "$FILE"
add_to_path "CPLUS_INCLUDE_PATH" "$XSMASH_PATH/local/include/" "$FILE"
add_to_path "LD_LIBRARY_PATH" "$XSMASH_PATH/local/lib/" "$FILE"
add_to_path "LIBRARY_PATH" "$XSMASH_PATH/local/lib/" "$FILE"
add_to_path "PATH" "$XSMASH_PATH/local/lib" "$FILE"

source "$HOME/.bashrc"

cd bifrost || exit
git reset --hard a05730f6285f5a4a7fc579508c5edbea661050e2
mkdir build && cd build || exit
cmake -DCMAKE_INSTALL_PREFIX="$XSMASH_PATH/local" ..
make
make install

cd "$XSMASH_PATH" || exit
rm -rf bifrost
make
