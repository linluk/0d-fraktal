#!/bin/bash

echo Benchmark $(date)  -  $(git show -s --pretty=format:'%h') >> benchmark.txt
echo $ 0d-fraktal -o benchmark.ppm -d 1920x1920 -c 0+0i -b 2 -i 1024 -t 4 >> benchmark.txt
{ time ./0d-fraktal -o benchmark.ppm -d 1920x1920 -c 0+0i -b 2 -i 1024 -t 4; } 2>> benchmark.txt
echo >> benchmark.txt

