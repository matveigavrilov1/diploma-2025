import pandas as pd
import matplotlib.pyplot as plt
import re
from datetime import datetime
import sys
import os

def parse_filename(filename):
	pattern = r'(\d{4}-\d{2}-\d{2}_\d{2}-\d{2}-\d{2})_threads_(\d+)_coro_(\d+)_shared_(\d+)_target_(\w+)_dump_(\d+)_worktime_(\d+)\.csv'
	match = re.match(pattern, filename)
	if not match:
		return None

	params = {
		'timestamp': match.group(1).replace('_', ' '),
		'threads': int(match.group(2)),
		'coroutines': int(match.group(3)),
		'shared_objects': int(match.group(4)),
		'target': match.group(5),
		'dump_interval_ms': int(match.group(6)),
		'worktime_sec': int(match.group(7))
	}
	return params

def find_pair_file(filename):
	params = parse_filename(filename)
	if not params:
		return None

	# Determine the opposite target
	if params['target'] == 'm':
		pair_target = 'cm'
	elif params['target'] == 'cm':
		pair_target = 'm'
	else:
		return None

	# Extract directory and prepare regex pattern (ignoring timestamp)
	dirname = os.path.dirname(filename) or '.'  # Current dir if no directory in path
	pattern = re.compile(
		r"^.*_threads_" + re.escape(str(params['threads'])) + 
		r"_coro_" + re.escape(str(params['coroutines'])) +
		r"_shared_" + re.escape(str(params['shared_objects'])) +
		r"_target_" + re.escape(pair_target) +
		r"_dump_" + re.escape(str(params['dump_interval_ms'])) +
		r"_worktime_" + re.escape(str(params['worktime_sec'])) + 
		r"\.csv$",
		re.IGNORECASE
	)

	# Search for the pair file in the directory
	for file in os.listdir(dirname):
		if pattern.match(file):
			return os.path.join(dirname, file)

	return None

def plot_benchmark_results(filename):
	params = parse_filename(filename)
	if not params:
		print("Failed to parse parameters from filename")
		return

	# Find pair file
	pair_filename = find_pair_file(filename)
	if not pair_filename:
		print(f"Error: Could not find pair file for {filename}")
		return

	# Read both files
	df1 = pd.read_csv(filename, header=None)
	df2 = pd.read_csv(pair_filename, header=None)

	# Extract data
	timestamps1 = pd.to_datetime(df1.iloc[:, 0], format='%H:%M:%S.%f')
	total1 = df1.iloc[:, -1]

	timestamps2 = pd.to_datetime(df2.iloc[:, 0], format='%H:%M:%S.%f')
	total2 = df2.iloc[:, -1]

	# Create plot
	plt.figure(figsize=(12, 8))

	# Plot both totals
	plt.plot(timestamps1, total1, label='std::mutex' if params['target'] == 'm' else 'coroMutex', linewidth=2)
	plt.plot(timestamps2, total2, label='std::mutex' if params['target'] != 'm' else 'coroMutex', linewidth=2)

	# Set plot details
	plt.title(
		f"Benchmark Comparison\n"
		f"Threads: {params['threads']}, Coroutines: {params['coroutines']}, "
		f"Shared objects: {params['shared_objects']}\n"
		f"Dump interval: {params['dump_interval_ms']}ms, "
		f"Work time: {params['worktime_sec']}sec"
	)
	plt.xlabel('Time')
	plt.ylabel('Total counter value')
	plt.legend()
	plt.grid(True)

	# Format x-axis
	plt.gca().xaxis.set_major_formatter(plt.matplotlib.dates.DateFormatter('%H:%M:%S'))
	plt.gcf().autofmt_xdate()

	plt.tight_layout()

	# Save the plot
	output_filename = filename.replace('.csv', '_comparison.png')
	plt.savefig(output_filename)
	print(f"Comparison plot saved to {output_filename}")
	plt.close()

if __name__ == "__main__":
	if len(sys.argv) != 2:
		print("Usage: python gen_bench_graphic_comparison.py <input_filename.csv>")
		sys.exit(1)

	input_filename = sys.argv[1]
	plot_benchmark_results(input_filename)