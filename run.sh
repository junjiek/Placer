#!/bin/bash
if [[ $# != 2 ]]; then
    echo "Usage: ./run.sh <algorithm> <benchmarkname>"
    exit 0
fi
# make
./placer-$1 ~/Project/benchmark/ISPD05/$2/$2.aux $2-$1.sol.pl
mv Upper-Bound-Place_HPWL ../log/$2/$2-$1-Upper-Bound-Place_HPWL
mv Lower-Bound-Place_HPWL ../log/$2/$2-$1-Lower-Bound-Place_HPWL
mv Upper-Bound-Choice ../log/$2/$2-$1-Upper-Bound-Choice
# mv Upper-Bound-MaxDensity ../log/$2/$2-$1-Upper-Bound-MaxDensity
# mv Lower-Bound-MaxDensity ../log/$2/$2-$1-Lower-Bound-MaxDensity
# mv MoveDist ../log/$2/$2-$1-MoveDist
echo $2 $1 'finished!'
