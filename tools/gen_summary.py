import os
import re
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from datetime import datetime

def parse_directory_name(dir_name):
	pattern = r'threads_(\d+)_coro_(\d+)_shared_(\d+)_dump_(\d+)_worktime_(\d+)'
	match = re.search(pattern, dir_name)
	if match:
		return {
			'threads': int(match.group(1)),
			'coro': int(match.group(2)),
			'shared': int(match.group(3)),
			'dump_interval': int(match.group(4)),
			'worktime': int(match.group(5))
		}
	return None

def get_last_sum_from_csv(file_path):
	try:
		df = pd.read_csv(file_path, header=None)
		return df.iloc[-1].iloc[-1]  # Last column of last row
	except Exception as e:
		print(f"Error processing {file_path}: {e}")
		return None

def process_runs_directory(base_dir="runs"):
	results = []

	for dir_name in os.listdir(base_dir):
		dir_path = os.path.join(base_dir, dir_name)
		if not os.path.isdir(dir_path):
			continue
			
		params = parse_directory_name(dir_name)
		if not params:
			continue
			
		res_dir = os.path.join(dir_path, "res")
		if not os.path.exists(res_dir):
			continue
			
		for file in os.listdir(res_dir):
			if file.endswith(".csv"):
				file_path = os.path.join(res_dir, file)
				target = "cm" if "target_cm" in file else "m" if "target_m" in file else None
				if not target:
					continue
					
				last_sum = get_last_sum_from_csv(file_path)
				if last_sum is not None:
					result = params.copy()
					result['target'] = target
					result['last_sum'] = last_sum
					results.append(result)

	return pd.DataFrame(results)

def create_summary_table(df):
	if df.empty:
		return pd.DataFrame()

	# Create pivot table
	pivot_df = df.pivot_table(
		index=['threads', 'coro', 'shared', 'dump_interval', 'worktime'],
		columns='target',
		values='last_sum',
		aggfunc='first'
	).reset_index()

	# Clean up and calculate metrics
	pivot_df.columns.name = None
	pivot_df = pivot_df.rename(columns={'m': 'mutex_sum', 'cm': 'coro_mutex_sum'})

	if 'mutex_sum' in pivot_df.columns and 'coro_mutex_sum' in pivot_df.columns:
		pivot_df['difference'] = pivot_df['coro_mutex_sum'] - pivot_df['mutex_sum']
		pivot_df['ratio'] = pivot_df['coro_mutex_sum'] / pivot_df['mutex_sum']

	return pivot_df.sort_values(['threads', 'coro', 'shared'])

def create_plots(summary_df, output_dir="plots"):
	if not os.path.exists(output_dir):
		os.makedirs(output_dir)

	# 1. Performance comparison plot
	plt.figure(figsize=(12, 6))
	sns.barplot(
		x='threads', 
		y='value', 
		hue='variable',
		data=pd.melt(
			summary_df, 
			id_vars=['threads', 'coro', 'shared'], 
			value_vars=['mutex_sum', 'coro_mutex_sum']
		)
	)
	plt.title('Performance: std::mutex vs coroMutex')
	plt.ylabel('Total operations')
	plt.savefig(f"{output_dir}/performance_comparison.png", bbox_inches='tight')
	plt.close()

	# 2. Ratio by threads
	if 'ratio' in summary_df.columns:
		plt.figure(figsize=(10, 5))
		sns.lineplot(x='threads', y='ratio', hue='shared', data=summary_df, marker='o')
		plt.axhline(1, color='red', linestyle='--', alpha=0.5)
		plt.title('Performance Ratio (coroMutex/std::mutex)')
		plt.ylabel('Ratio (higher = better for coroMutex)')
		plt.savefig(f"{output_dir}/performance_ratio.png", bbox_inches='tight')
		plt.close()

def main():
	print("Processing benchmark data...")
	df = process_runs_directory()

	if df.empty:
		print("No valid benchmark data found.")
		return

	# Create and save summary table
	summary_df = create_summary_table(df)
	timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
	summary_file = f"summary_table_{timestamp}.csv"
	summary_df.to_csv(summary_file, index=False)

	# Create plots
	create_plots(summary_df)

	print("\nSummary table saved to:", summary_file)
	print("Plots saved to: plots/ directory")
	print("\nSummary table preview:")
	print(summary_df.head().to_string(index=False))

if __name__ == "__main__":
	main()