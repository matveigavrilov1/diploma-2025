import os
import re
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from datetime import datetime
from scipy.stats import variation

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

def process_csv(file_path):
	"""Process CSV file and return statistics"""
	try:
		df = pd.read_csv(file_path, header=None)
		counters = df.iloc[:, 1:-1]  # All counter columns
		total_sum = df.iloc[-1].iloc[-1]  # Final sum
		
		return {
			'total_sum': total_sum,
			'mean': counters.mean().mean(),
			'std_dev': counters.stack().std(),
			'cv': variation(counters.stack()),
			'max_diff': (counters.max(axis=1) - counters.min(axis=1)).mean(),
			'counters_data': counters.values  # Store raw counter values for plotting
		}
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
					
				stats = process_csv(file_path)
				if stats:
					result = params.copy()
					result.update(stats)
					result['target'] = target
					results.append(result)

	return pd.DataFrame(results)

def create_summary_table(df):
	"""Create enhanced summary table with uniformity metrics"""
	if df.empty:
		return pd.DataFrame()

	# Group by parameters and target type
	summary = df.groupby(['threads', 'coro', 'shared', 'dump_interval', 'worktime', 'target']).agg({
		'total_sum': 'mean',
		'mean': 'mean',
		'std_dev': 'mean',
		'cv': 'mean',
		'max_diff': 'mean'
	}).reset_index()

	# Pivot to compare mutex types
	pivot_df = summary.pivot_table(
		index=['threads', 'coro', 'shared', 'dump_interval', 'worktime'],
		columns='target',
		values=['total_sum', 'mean', 'std_dev', 'cv', 'max_diff'],
		aggfunc='mean'
	).reset_index()

	# Flatten multi-index columns
	pivot_df.columns = ['_'.join(col).strip('_') for col in pivot_df.columns.values]

	# Calculate differences and ratios
	if 'total_sum_cm' in pivot_df.columns and 'total_sum_m' in pivot_df.columns:
		pivot_df['sum_ratio'] = pivot_df['total_sum_cm'] / pivot_df['total_sum_m']
		pivot_df['sum_diff'] = pivot_df['total_sum_cm'] - pivot_df['total_sum_m']
		pivot_df['cv_diff'] = pivot_df['cv_cm'] - pivot_df['cv_m']

	return pivot_df.sort_values(['threads', 'shared', 'coro'])

def create_combined_plots(summary_df, raw_df, output_dir="plots"):
	"""Create all visualization plots"""
	if not os.path.exists(output_dir):
		os.makedirs(output_dir)

	# 1. Performance comparison with error bars
	plt.figure(figsize=(14, 7))
	sns.barplot(x='threads', y='total_sum', hue='target', data=raw_df, ci='sd')
	plt.title('Total Sum with Standard Deviation')
	plt.ylabel('Total Operations')
	plt.savefig(f"{output_dir}/performance_with_error.png", bbox_inches='tight')
	plt.close()

	# 2. Uniformity comparison (CV)
	plt.figure(figsize=(14, 7))
	sns.lineplot(x='threads', y='cv', hue='target', style='shared', 
				data=raw_df, markers=True, ci='sd')
	plt.title('Counter Uniformity (Coefficient of Variation)')
	plt.ylabel('CV (lower = more uniform)')
	plt.savefig(f"{output_dir}/uniformity_comparison.png", bbox_inches='tight')
	plt.close()

	# 3. NEW: Performance vs Uniformity scatter plot
	plt.figure(figsize=(14, 7))
	sns.scatterplot(x='cv', y='total_sum', hue='threads', style='target', 
					size='shared', data=raw_df, sizes=(50, 200))
	plt.title('Performance vs Counter Uniformity')
	plt.xlabel('Coefficient of Variation')
	plt.ylabel('Total Operations')
	plt.savefig(f"{output_dir}/performance_vs_uniformity.png", bbox_inches='tight')
	plt.close()

	# 4. Max difference heatmap
	plt.figure(figsize=(12, 8))
	pivot = raw_df.pivot_table(index='threads', columns='shared', 
								values='max_diff', aggfunc='mean')
	sns.heatmap(pivot, annot=True, fmt=".1f", cmap="YlOrRd")
	plt.title('Average Max Difference Between Counters')
	plt.savefig(f"{output_dir}/max_diff_heatmap.png", bbox_inches='tight')
	plt.close()

def main():
	print("Processing benchmark data with uniformity analysis...")
	raw_df = process_runs_directory()

	if raw_df.empty:
		print("No valid benchmark data found.")
		return

	# Create enhanced summary table
	summary_df = create_summary_table(raw_df)
	timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")

	# Save results
	summary_file = f"enhanced_summary_{timestamp}.csv"
	summary_df.to_csv(summary_file, index=False)

	raw_file = f"raw_data_{timestamp}.csv"
	raw_df.to_csv(raw_file, index=False)

	# Create all plots
	create_combined_plots(summary_df, raw_df)

	print("\nEnhanced summary saved to:", summary_file)
	print("Raw data saved to:", raw_file)
	print("Visualizations saved to: plots/ directory")
	print("\nSummary table preview:")
    print(summary_df.head().to_string(index=False))

if __name__ == "__main__":
    main()