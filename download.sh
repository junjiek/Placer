#!/bin/bash
if [[ $# != 2 ]]; then
    echo "Usage: ./download.sh <benchmarkname> <suffix>"
    exit 0
fi

scp edalab:Project/Placer/Upper-Bound-Place_HPWL ../log/$1/Upper-Bound-Place_HPWL-$2
scp edalab:Project/Placer/Lower-Bound-Place_HPWL ../log/$1/Lower-Bound-Place_HPWL-$2
