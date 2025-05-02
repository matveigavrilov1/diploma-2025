import pandas as pd
import matplotlib.pyplot as plt
import re
from datetime import datetime
import sys
import os

def parse_filename(filename):
	basename = os.path.basename(filename)
	pattern = r'(\d{4}-\d{2}-\d{2}_\d{2}-\d{2}-\d{2})_threads_(\d+)_coro_(\d+)_shared_(\d+)_target_(\w+)_dump_(\d+)_worktime_(\d+)\.csv'
	match = re.match(pattern, basename)
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

def plot_benchmark_results(filename):
	params = parse_filename(filename)
	if not params:
		print("Failed to parse parameters from filename")
		return

	df = pd.read_csv(filename, header=None)

	timestamps = pd.to_datetime(df.iloc[:, 0], format='%H:%M:%S.%f')
	counters = df.iloc[:, 1:-1]
	total = df.iloc[:, -1]

	plt.figure(figsize=(12, 8))

	for i in range(counters.shape[1]):
		plt.plot(timestamps, counters.iloc[:, i], label=f'Counter {i+1}')

	plt.plot(timestamps, total, label='Total', color='black', linewidth=2, linestyle='--')

	target_name = "std::mutex" if params['target'] == "m" else "coroMutex"

	plt.title(
		f"Benchmark {target_name}\n"
		f"Threads: {params['threads']}, Coroutines: {params['coroutines']}, "
		f"Shared objects: {params['shared_objects']}\n"
		f"Dump interval: {params['dump_interval_ms']}ms, "
		f"Work time: {params['worktime_sec']}sec"
	)
	plt.xlabel('Time')
	plt.ylabel('Counter values')
	plt.legend()
	plt.grid(True)

	plt.gca().xaxis.set_major_formatter(plt.matplotlib.dates.DateFormatter('%H:%M:%S'))
	plt.gcf().autofmt_xdate()

	plt.tight_layout()

	# Save the plot to a file instead of showing it
	output_filename = filename.replace('.csv', '.png')
	plt.savefig(output_filename)
	print(f"Plot saved to {output_filename}")
	plt.close()

if __name__ == "__main__":
	if len(sys.argv) != 2:
		print("Usage: python gen_bench_graphic.py <input_filename.csv>")
		sys.exit(1)

	input_filename = sys.argv[1]
	plot_benchmark_results(input_filename)