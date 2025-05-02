#!/bin/bash

./run_benchmark.sh -c 1 -d 100 -s 1 -n 1 -w 5

./run_benchmark.sh -c 5 -d 100 -s 1 -n 1 -w 5
./run_benchmark.sh -c 5 -d 100 -s 1 -n 10 -w 5
./run_benchmark.sh -c 10 -d 100 -s 1 -n 10 -w 5
./run_benchmark.sh -c 15 -d 100 -s 1 -n 10 -w 5

./run_benchmark.sh -c 5 -d 100 -s 5 -n 1 -w 5
./run_benchmark.sh -c 5 -d 100 -s 5 -n 10 -w 5
./run_benchmark.sh -c 10 -d 100 -s 5 -n 10 -w 5
./run_benchmark.sh -c 15 -d 100 -s 5 -n 10 -w 5

./run_benchmark.sh -c 5 -d 100 -s 10 -n 1 -w 5
./run_benchmark.sh -c 5 -d 100 -s 10 -n 10 -w 5
./run_benchmark.sh -c 10 -d 100 -s 10 -n 10 -w 5
./run_benchmark.sh -c 15 -d 100 -s 10 -n 10 -w 5

./run_benchmark.sh -c 5 -d 100 -s 15 -n 1 -w 5
./run_benchmark.sh -c 5 -d 100 -s 15 -n 10 -w 5
./run_benchmark.sh -c 10 -d 100 -s 15 -n 10 -w 5
./run_benchmark.sh -c 15 -d 100 -s 15 -n 10 -w 5