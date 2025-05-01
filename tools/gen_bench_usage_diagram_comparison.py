import pandas as pd
import matplotlib.pyplot as plt
import re
import sys
import os

def parse_filename(filename):
	pattern = r'(\d{4}-\d{2}-\d{2}_\d{2}-\d{2}-\d{2})_threads_(\d+)_coro_(\d+)_shared_(\d+)_target_(\w+)_dump_(\d+)_worktime_(\d+)\.usage'
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

	pair_target = 'cm' if params['target'] == 'm' else 'm' if params['target'] == 'cm' else None
	if not pair_target:
		return None

	dirname = os.path.dirname(filename) or '.'
	target_params = {
		'threads': params['threads'],
		'coroutines': params['coroutines'],
		'shared_objects': params['shared_objects'],
		'target': pair_target,
		'dump_interval_ms': params['dump_interval_ms'],
		'worktime_sec': params['worktime_sec'],
	}

	for file in os.listdir(dirname):
		if not file.endswith('.usage'):
			continue

		file_params = parse_filename(file)
		if not file_params:
			continue

		if all(file_params.get(k) == v for k, v in target_params.items()):
			return os.path.join(dirname, file)

	return None

def parse_usage_file(filename):
	with open(filename, 'r') as file:
		content = file.read()

	user_time = re.search(r'User Time \(μs\): (\d+)', content)
	system_time = re.search(r'System Time \(μs\): (\d+)', content)

	if not user_time or not system_time:
		return None

	return {
		'user_time': int(user_time.group(1)),
		'system_time': int(system_time.group(1)),
		'total_time': int(user_time.group(1)) + int(system_time.group(1))
	}

def plot_usage_comparison(filename):
	run_params = parse_filename(filename)
	if not run_params:
		print("Failed to parse parameters from filename")
		return

	pair_filename = find_pair_file(filename)
	if not pair_filename:
		print(f"Error: Could not find pair file for {filename}")
		return

	main_data = parse_usage_file(filename)
	pair_data = parse_usage_file(pair_filename)

	if not main_data or not pair_data:
		print("Failed to parse usage data from one of the files")
		return

	metrics = ['User Time', 'System Time', 'Total Time']
	main_values = [main_data['user_time'], main_data['system_time'], main_data['total_time']]
	pair_values = [pair_data['user_time'], pair_data['system_time'], pair_data['total_time']]

	# Determine which is which
	main_target = "std::mutex" if run_params['target'] == "m" else "coroMutex"
	pair_target = "coroMutex" if run_params['target'] == "m" else "std::mutex"

	# Create figure
	plt.figure(figsize=(12, 8))

	# Set bar width and positions
	bar_width = 0.35
	index = range(len(metrics))

	# Plot bars
	bars1 = plt.bar(index, main_values, bar_width, label=main_target, color='blue')
	bars2 = plt.bar([i + bar_width for i in index], pair_values, bar_width, label=pair_target, color='orange')

	# Add value labels
	for bars in [bars1, bars2]:
		for bar in bars:
			height = bar.get_height()
			plt.text(bar.get_x() + bar.get_width()/2., height,
					f'{height:,} μs',
					ha='center', va='bottom', fontsize=9)

	# Customize plot
	plt.title(
		f"Resource Usage Comparison\n"
		f"Threads: {run_params['threads']}, Coroutines: {run_params['coroutines']}, "
		f"Shared objects: {run_params['shared_objects']}\n"
		f"Dump interval: {run_params['dump_interval_ms']}ms, "
		f"Work time: {run_params['worktime_sec']}sec"
	)
	plt.xlabel('Metric')
	plt.ylabel('Time (microseconds)')
	plt.xticks([i + bar_width/2 for i in index], metrics)
	plt.legend()
	plt.grid(True, axis='y')

	plt.tight_layout()

	# Save plot
	output_filename = filename.replace('.usage', '_usage_comparison.png')
	plt.savefig(output_filename)
	print(f"Comparison plot saved to {output_filename}")
	plt.close()

if __name__ == "__main__":
	if len(sys.argv) != 2:
		print("Usage: python gen_bench_usage_diagram_comparison.py <input_filename.usage>")
		sys.exit(1)

	input_filename = sys.argv[1]
	plot_usage_comparison(input_filename)