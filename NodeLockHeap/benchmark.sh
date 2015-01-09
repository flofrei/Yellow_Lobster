#!/bin/sh
rm -f output/*.dat

./benchmark_CPQ
./benchmark_STL
./benchmark_Intel
