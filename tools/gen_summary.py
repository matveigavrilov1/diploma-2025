import os
import re
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from datetime import datetime

def parse_directory_name(dir_name):
	"""Extract parameters from directory name"""
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
	"""Get the last sum value from a CSV file"""
	try:
		df = pd.read_csv(file_path, header=None)
		return df.iloc[-1].iloc[-1]  # Last column of last row
	except Exception as e:
		print(f"Error processing {file_path}: {e}")
		return None

def parse_usage_file(file_path):
	"""Parse .usage file to get User Time and System Time"""
	try:
		with open(file_path, 'r') as f:
			content = f.read()
			user_time = int(re.search(r'User Time \(μs\): (\d+)', content).group(1))
			system_time = int(re.search(r'System Time \(μs\): (\d+)', content).group(1))
			return user_time, system_time
	except Exception as e:
		print(f"Error processing {file_path}: {e}")
		return None, None

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
			
		# Initialize data storage for this directory
		dir_data = {
			'm': {'last_sum': None, 'user_time': None, 'system_time': None},
			'cm': {'last_sum': None, 'user_time': None, 'system_time': None}
		}
			
		# Process all files in res directory
		for file in os.listdir(res_dir):
			file_path = os.path.join(res_dir, file)
			
			# Process CSV files
			if file.endswith(".csv"):
				target = "cm" if "target_cm" in file else "m" if "target_m" in file else None
				if target:
					last_sum = get_last_sum_from_csv(file_path)
					if last_sum is not None:
						dir_data[target]['last_sum'] = last_sum
			
			# Process .usage files
			elif file.endswith(".usage"):
				target = "cm" if "target_cm" in file else "m" if "target_m" in file else None
				if target:
					user_time, system_time = parse_usage_file(file_path)
					if user_time is not None:
						dir_data[target]['user_time'] = user_time
						dir_data[target]['system_time'] = system_time
		
		# Add collected data to results
		for target in ['m', 'cm']:
			if dir_data[target]['last_sum'] is not None or dir_data[target]['user_time'] is not None:
				result = params.copy()
				result['target'] = target
				result.update(dir_data[target])
				results.append(result)

	return pd.DataFrame(results)

def create_summary_table(df):
	"""Create summary table comparing mutex types"""
	if df.empty:
		return pd.DataFrame()

	# Create pivot table for all metrics
	pivot_df = df.pivot_table(
		index=['threads', 'coro', 'shared', 'dump_interval', 'worktime'],
		columns='target',
		values=['last_sum', 'user_time', 'system_time'],
		aggfunc='first'
	).reset_index()

	# Flatten multi-level columns
	pivot_df.columns = ['_'.join(col).strip('_') for col in pivot_df.columns.values]

	# Clean up column names
	pivot_df = pivot_df.rename(columns={
		'threads': 'threads',
		'coro': 'coro',
		'shared': 'shared',
		'dump_interval': 'dump_interval',
		'worktime': 'worktime',
		'last_sum_m': 'mutex_sum',
		'last_sum_cm': 'coro_mutex_sum',
		'user_time_m': 'mutex_user_time',
		'user_time_cm': 'coro_user_time',
		'system_time_m': 'mutex_system_time',
		'system_time_cm': 'coro_system_time'
	})

	# Calculate performance metrics if both mutex types are available
	if 'mutex_sum' in pivot_df.columns and 'coro_mutex_sum' in pivot_df.columns:
		pivot_df['difference'] = pivot_df['coro_mutex_sum'] - pivot_df['mutex_sum']
		pivot_df['ratio'] = pivot_df['coro_mutex_sum'] / pivot_df['mutex_sum']

	# Calculate time differences if available
	if 'mutex_user_time' in pivot_df.columns and 'coro_user_time' in pivot_df.columns:
		pivot_df['user_time_diff'] = pivot_df['coro_user_time'] - pivot_df['mutex_user_time']
		pivot_df['user_time_ratio'] = pivot_df['coro_user_time'] / pivot_df['mutex_user_time']
		
	if 'mutex_system_time' in pivot_df.columns and 'coro_system_time' in pivot_df.columns:
		pivot_df['system_time_diff'] = pivot_df['coro_system_time'] - pivot_df['mutex_system_time']
		pivot_df['system_time_ratio'] = pivot_df['coro_system_time'] / pivot_df['mutex_system_time']

	return pivot_df.sort_values(['threads', 'coro', 'shared'])

def create_plots(summary_df, output_dir="plots"):
	"""Create visualization plots from summary table"""
	if not os.path.exists(output_dir):
		os.makedirs(output_dir)

	# 1. Performance comparison plot
	if 'mutex_sum' in summary_df.columns and 'coro_mutex_sum' in summary_df.columns:
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

	# 3. User Time comparison
	if 'mutex_user_time' in summary_df.columns and 'coro_user_time' in summary_df.columns:
		plt.figure(figsize=(12, 6))
		sns.barplot(
			x='threads', 
			y='value', 
			hue='variable',
			data=pd.melt(
				summary_df, 
				id_vars=['threads', 'coro', 'shared'], 
				value_vars=['mutex_user_time', 'coro_user_time']
			)
		)
		plt.title('User Time Comparison: std::mutex vs coroMutex (μs)')
		plt.ylabel('User Time (μs)')
		plt.savefig(f"{output_dir}/user_time_comparison.png", bbox_inches='tight')
		plt.close()

		# 4. User Time ratio
		if 'user_time_ratio' in summary_df.columns:
			plt.figure(figsize=(10, 5))
			sns.lineplot(x='threads', y='user_time_ratio', hue='shared', data=summary_df, marker='o')
			plt.axhline(1, color='red', linestyle='--', alpha=0.5)
			plt.title('User Time Ratio (coroMutex/std::mutex)')
			plt.ylabel('Ratio (lower = better for coroMutex)')
			plt.savefig(f"{output_dir}/user_time_ratio.png", bbox_inches='tight')
			plt.close()

	# 5. System Time comparison
	if 'mutex_system_time' in summary_df.columns and 'coro_system_time' in summary_df.columns:
		plt.figure(figsize=(12, 6))
		sns.barplot(
			x='threads', 
			y='value', 
			hue='variable',
			data=pd.melt(
				summary_df, 
				id_vars=['threads', 'coro', 'shared'], 
				value_vars=['mutex_system_time', 'coro_system_time']
			)
		)
		plt.title('System Time Comparison: std::mutex vs coroMutex (μs)')
		plt.ylabel('System Time (μs)')
		plt.savefig(f"{output_dir}/system_time_comparison.png", bbox_inches='tight')
		plt.close()

		# 6. System Time ratio
		if 'system_time_ratio' in summary_df.columns:
			plt.figure(figsize=(10, 5))
			sns.lineplot(x='threads', y='system_time_ratio', hue='shared', data=summary_df, marker='o')
			plt.axhline(1, color='red', linestyle='--', alpha=0.5)
			plt.title('System Time Ratio (coroMutex/std::mutex)')
			plt.ylabel('Ratio (lower = better for coroMutex)')
			plt.savefig(f"{output_dir}/system_time_ratio.png", bbox_inches='tight')
			plt.close()

	if 'coro' in summary_df.columns and 'ratio' in summary_df.columns:
		plt.figure(figsize=(10, 5))
		sns.lineplot(x='coro', y='ratio', hue='threads', style='shared', data=summary_df, marker='o')
		plt.axhline(1, color='red', linestyle='--', alpha=0.5)
		plt.title('Performance Ratio by Coroutine Count')
		plt.ylabel('Ratio (coroMutex/std::mutex)')
		plt.savefig(f"{output_dir}/performance_ratio_by_coro.png", bbox_inches='tight')
		plt.close()

	if all(col in summary_df.columns for col in ['mutex_user_time', 'mutex_system_time', 'coro_user_time', 'coro_system_time']):
		summary_df['total_time_m'] = summary_df['mutex_user_time'] + summary_df['mutex_system_time']
		summary_df['total_time_cm'] = summary_df['coro_user_time'] + summary_df['coro_system_time']

		plt.figure(figsize=(12, 6))
		sns.barplot(
			x='threads',
			y='value',
			hue='variable',
			data=pd.melt(
				summary_df,
				id_vars=['threads', 'coro', 'shared'],
				value_vars=['total_time_m', 'total_time_cm']
			)
		)
		plt.title('Total Execution Time Comparison (μs)')
		plt.ylabel('Total Time (User + System)')
		plt.savefig(f"{output_dir}/total_time_comparison.png", bbox_inches='tight')
		plt.close()

	if all(col in summary_df.columns for col in ['mutex_user_time', 'mutex_system_time', 'coro_user_time', 'coro_system_time']):
		fig, axes = plt.subplots(1, 2, figsize=(15, 6))
		
		# For std::mutex
		summary_df[['mutex_user_time', 'mutex_system_time']].plot(
			kind='bar', stacked=True, ax=axes[0],
			title='Time Distribution: std::mutex'
		)
		axes[0].set_ylabel('Time (μs)')
		
		# For coroMutex
		summary_df[['coro_user_time', 'coro_system_time']].plot(
			kind='bar', stacked=True, ax=axes[1],
			title='Time Distribution: coroMutex'
		)
		
		plt.tight_layout()
		plt.savefig(f"{output_dir}/time_distribution.png", bbox_inches='tight')
		plt.close()

	if all(col in summary_df.columns for col in ['threads', 'coro', 'ratio']):
		pivot = summary_df.pivot_table(index='threads', columns='coro', values='ratio')
		plt.figure(figsize=(10, 8))
		sns.heatmap(pivot, annot=True, fmt=".2f", cmap="YlGnBu", center=1.0)
		plt.title('Performance Ratio Heatmap (coroMutex/std::mutex)')
		plt.savefig(f"{output_dir}/performance_heatmap.png", bbox_inches='tight')
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