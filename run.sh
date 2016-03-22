#!/bin/bash
if [[ $# != 2 ]]; then
    echo "Usage: ./run.sh <algorithm> <benchmarkname>"
    exit 0
fi
# make
./placer-$1 ~/Project/benchmark/ISPD05/$2/$2.aux $2.sol.pl
