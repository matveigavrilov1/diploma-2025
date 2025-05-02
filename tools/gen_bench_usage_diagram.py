import pandas as pd
import matplotlib.pyplot as plt
import re
import sys
import os

def parse_filename(filename):
	basename = os.path.basename(filename)
	pattern = r'(\d{4}-\d{2}-\d{2}_\d{2}-\d{2}-\d{2})_threads_(\d+)_coro_(\d+)_shared_(\d+)_target_(\w+)_dump_(\d+)_worktime_(\d+)\.usage'
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

def parse_usage_file(filename):
	with open(filename, 'r') as file:
		content = file.read()

	wall_time = user_time = re.search(r'Wall Time \(μs\): (\d+)', content)
	user_time = re.search(r'User Time \(μs\): (\d+)', content)
	system_time = re.search(r'System Time \(μs\): (\d+)', content)

	if not wall_time or not user_time or not system_time:
		return None

	return {
		'wall_time': int(wall_time.group(1)),
		'user_time': int(user_time.group(1)),
		'system_time': int(system_time.group(1))
	}

def plot_usage_results(filename):
	run_params = parse_filename(filename)
	if not run_params:
		print("Failed to parse parameters from filename")
		return

	usage_data = parse_usage_file(filename)
	if not usage_data:
		print("Failed to parse usage data from file")
		return

	df = pd.DataFrame({
		'Metric': ['Wall Time', 'User Time', 'System Time'],
		'Time (μs)': [usage_data['wall_time'], usage_data['user_time'], usage_data['system_time']]
	})

	plt.figure(figsize=(10, 6))

	bars = plt.bar(df['Metric'], df['Time (μs)'], color=['blue', 'orange'])

	for bar in bars:
		height = bar.get_height()
		plt.text(bar.get_x() + bar.get_width()/2., height,
					f'{height:,} μs',
					ha='center', va='bottom')

	target_name = "std::mutex" if run_params['target'] == "m" else "coroMutex"

	title = (
		f"Resource Usage ({target_name})\n"
		f"Threads: {run_params['threads']}, Coroutines: {run_params['coroutines']}, "
		f"Shared objects: {run_params['shared_objects']}\n"
		f"Dump interval: {run_params['dump_interval_ms']}ms, "
		f"Work time: {run_params['worktime_sec']}sec"
	)
	plt.title(title)
	plt.ylabel('Time (microseconds)')
	plt.grid(True, axis='y')

	plt.tight_layout()

	output_filename = filename.replace('.usage', '_usage.png')
	plt.savefig(output_filename)
	print(f"Plot saved to {output_filename}")
	plt.close()

if __name__ == "__main__":
	if len(sys.argv) != 2:
		print("Usage: python gen_bench_usage_diagram.py <input_filename.usage>")
		sys.exit(1)

	input_filename = sys.argv[1]
	plot_usage_results(input_filename)