#!/bin/sh
echo "Installing dependencies"
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get -yq install cmake build-essential git qtbase5-dev
echo "Cloning caffa"
git clone https://github.com/lindkvis/caffa.git
cd caffa
echo "Initialising Submodules"
git submodule init
git submodule update
mkdir build
cd build
echo "Configuring"
cmake ..
echo "Building"
make
