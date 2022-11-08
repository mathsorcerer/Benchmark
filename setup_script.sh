#!/bin/bash

# Clone benchmark source code
# git clone https://github.com/tunglevt/Benchmark

# Update system 
sudo apt-get update

# Install building tools
sudo apt-get install -y build-essential

# Install python3 
sudo apt-get install -y python3

# Install EMP-Toolkit
wget https://raw.githubusercontent.com/emp-toolkit/emp-readme/master/scripts/install.py

python3 install.py --install --tool --ot --agmpc

# Install MP-SPDZ
sudo apt-get install -y automake build-essential cmake git libboost-dev libboost-thread-dev libntl-dev libsodium-dev libssl-dev libtool m4 python3 texinfo yasm

wget https://github.com/data61/MP-SPDZ/archive/refs/tags/v0.3.3.tar.gz

tar -zxvf v0.3.3.tar.gz

# Replace Makefile MP-SPDZ
cp ./Makefile MP-SPDZ-0.3.3/

# Go to folder and build
cd MP-SPDZ-0.3.3
git clone https://github.com/simd-everywhere/simde deps/simde
make -j 8 tldr

# Build Fake-Offline protocol
rm -f ./Utils/Fake-Offline.cpp
cp ../Benchmark/Speed_Z/Fake-Offline.cpp ./Utils/
make Fake-Offline.x 

# Jump out MP-SPDZ
cd ../

# Install ZeroMQ
sudo apt-get install -y libzmq3-dev

# Build Global Garbled Circuit source code
cd Benchmark/Garbled_Circuit/Server
make

# Jump out 
cd ../../..

# Other works ...

