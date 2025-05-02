#!/bin/bash

n_values=(1 5 10 15 100)
c_values=(1 5 10 15 100)
s_values=(1 5 10 15 100)
w_value=5
d_value=100

./setup_benchmark_venv.sh

for n in "${n_values[@]}"; do
	for c in "${c_values[@]}"; do
		for s in "${s_values[@]}"; do
			./run_benchmark.sh -n "$n" -c "$c" -s "$s" -w "$w_value" -d "$d_value"
		done
	done
done