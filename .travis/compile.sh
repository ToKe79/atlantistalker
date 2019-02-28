#!/bin/bash
export MOREWARN=1
make
make libs
cd misc/poseidon
./compile.sh
cd ../passcheck
./compile.sh
cd ../passhash
./compile.sh
