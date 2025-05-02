import os
import re
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from datetime import datetime
from scipy.stats import variation

def parse_usage_file(file_path):
	metrics = {}
	try:
		with open(file_path, 'r') as f:
			for line in f:
				if 'Wall Time' in line:
					metrics['wall_time_us'] = int(line.split(':')[1].strip())
				elif 'User Time' in line:
					metrics['user_time_us'] = int(line.split(':')[1].strip())
				elif 'System Time' in line:
					metrics['system_time_us'] = int(line.split(':')[1].strip())
		# Calculate CPU usage percentage
		if 'wall_time_us' in metrics and metrics['wall_time_us'] > 0:
			metrics['cpu_usage_percent'] = (
				(metrics['user_time_us'] + metrics['system_time_us']) / 
				metrics['wall_time_us'] * 100
			)
		return metrics
	except Exception as e:
		print(f"Error parsing {file_path}: {e}")
		return None

def process_directory(base_dir="runs"):
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
			
		# Process CSV files
		csv_data = {}
		for file in os.listdir(res_dir):
			if file.endswith(".csv"):
				file_path = os.path.join(res_dir, file)
				target = "cm" if "target_cm" in file else "m" if "target_m" in file else None
				if not target:
					continue
					
				stats = process_csv(file_path)
				if stats:
					csv_data[target] = stats
		
		# Process USAGE files
		usage_data = {}
		for file in os.listdir(res_dir):
			if file.endswith(".usage"):
				file_path = os.path.join(res_dir, file)
				target = "cm" if "target_cm" in file else "m" if "target_m" in file else None
				if not target:
					continue
					
				metrics = parse_usage_file(file_path)
				if metrics:
					usage_data[target] = metrics
		
		# Combine data
		for target in set(csv_data.keys()).union(usage_data.keys()):
			result = params.copy()
			result['target'] = target
			if target in csv_data:
				result.update(csv_data[target])
			if target in usage_data:
				result.update(usage_data[target])
			results.append(result)

	return pd.DataFrame(results)

def create_enhanced_summary(df):
	"""Create summary table with performance, uniformity and resource usage"""
	if df.empty:
		return pd.DataFrame()

	# Group and pivot data
	summary = df.groupby(['threads', 'coro', 'shared', 'dump_interval', 'worktime', 'target']).agg({
		'total_sum': 'mean',
		'mean': 'mean',
		'std_dev': 'mean',
		'cv': 'mean',
		'max_diff': 'mean',
		'wall_time_us': 'mean',
		'user_time_us': 'mean',
		'system_time_us': 'mean',
		'cpu_usage_percent': 'mean'
	}).reset_index()

	# Pivot to compare mutex types
	pivot_df = summary.pivot_table(
		index=['threads', 'coro', 'shared', 'dump_interval', 'worktime'],
		columns='target',
		values=[
			'total_sum', 'mean', 'std_dev', 'cv', 'max_diff',
			'wall_time_us', 'user_time_us', 'system_time_us', 'cpu_usage_percent'
		],
		aggfunc='mean'
	).reset_index()

	# Flatten multi-index columns
	pivot_df.columns = ['_'.join(col).strip('_') for col in pivot_df.columns.values]

	# Calculate comparison metrics
	if 'total_sum_cm' in pivot_df.columns and 'total_sum_m' in pivot_df.columns:
		pivot_df['sum_ratio'] = pivot_df['total_sum_cm'] / pivot_df['total_sum_m']
		pivot_df['sum_diff'] = pivot_df['total_sum_cm'] - pivot_df['total_sum_m']
		pivot_df['cv_diff'] = pivot_df['cv_cm'] - pivot_df['cv_m']
		pivot_df['wall_time_ratio'] = pivot_df['wall_time_us_m'] / pivot_df['wall_time_us_cm']
		pivot_df['cpu_usage_diff'] = pivot_df['cpu_usage_percent_cm'] - pivot_df['cpu_usage_percent_m']

	return pivot_df.sort_values(['threads', 'shared', 'coro'])

def create_resource_plots(summary_df, output_dir="plots"):
	"""Create plots for resource usage analysis"""
	if not os.path.exists(output_dir):
		os.makedirs(output_dir)

	# 1. Wall Time Comparison
	plt.figure(figsize=(12, 6))
	sns.barplot(
		x='threads', 
		y='value', 
		hue='variable',
		data=pd.melt(
			summary_df, 
			id_vars=['threads'], 
			value_vars=['wall_time_us_cm', 'wall_time_us_m']
		)
	)
	plt.title('Wall Time Comparison (lower is better)')
	plt.ylabel('Wall Time (μs)')
	plt.savefig(f"{output_dir}/wall_time_comparison.png", bbox_inches='tight')
	plt.close()

	# 2. CPU Usage Comparison
	plt.figure(figsize=(12, 6))
	sns.lineplot(
		x='threads', 
		y='value', 
		hue='variable',
		style='shared',
		markers=True,
		data=pd.melt(
			summary_df, 
			id_vars=['threads', 'shared'], 
			value_vars=['cpu_usage_percent_cm', 'cpu_usage_percent_m']
		)
	)
	plt.title('CPU Usage Percentage Comparison')
	plt.ylabel('CPU Usage (%)')
	plt.savefig(f"{output_dir}/cpu_usage_comparison.png", bbox_inches='tight')
	plt.close()

def main():
	print("Processing benchmark data with resource usage...")
	raw_df = process_directory()

	if raw_df.empty:
		print("No valid benchmark data found.")
		return

	# Create enhanced summary table
	summary_df = create_enhanced_summary(raw_df)
	timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")

	# Save results
	summary_file = f"full_summary_{timestamp}.csv"
	summary_df.to_csv(summary_file, index=False)

	# Create all plots
	create_combined_plots(summary_df, raw_df)
	create_resource_plots(summary_df)

	print("\nFull summary saved to:", summary_file)
	print("Visualizations saved to: plots/ directory")
	print("\nSummary table columns preview:")
	print("\n".join(summary_df.columns.tolist()))

if __name__ == "__main__":
	main()