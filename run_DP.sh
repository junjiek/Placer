#!/bin/bash
if [[ $# != 2 ]]; then
    echo "Usage: ./run_DP.sh <algorithm> <benchmarkname>"
    exit 0
fi
./FastPlace3.0_Linux32_DP -legalize ../benchmark/ISPD05/$2 /$2.aux ./ /$2-$1.sol.pl
