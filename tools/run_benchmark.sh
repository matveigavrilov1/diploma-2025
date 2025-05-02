#!/bin/bash

while getopts "c:d:s:n:w:" opt; do
  case $opt in
    c) CORO=$OPTARG ;;
    d) DUMP=$OPTARG ;;
    s) SHARED=$OPTARG ;;
    n) THREADS=$OPTARG ;;
    w) WORKTIME=$OPTARG ;;
    *) echo "Usage: $0 -c coro -d dump -s shared -n threads -w worktime" >&2
       exit 1 ;;
  esac
done

if [ -z "$CORO" ] || [ -z "$DUMP" ] || [ -z "$SHARED" ] || [ -z "$THREADS" ] || [ -z "$WORKTIME" ]; then
  echo "All parameters (-c, -d, -s, -n, -w) are required"
  echo "Usage: $0 -c coro -d dump -s shared -n threads -w worktime"
  exit 1
fi

mkdir -p runs

./coroMutexBenchmark -c "$CORO" -d "$DUMP" -s "$SHARED" -t cm -n "$THREADS" -w "$WORKTIME" -o runs 
latest_cm_file=$(find runs -name "*.csv" -type f -printf "%T@ %p\n" | sort -n | tail -1 | cut -d' ' -f2-)
latest_cm_usage_file=$(find runs -name "*.usage" -type f -printf "%T@ %p\n" | sort -n | tail -1 | cut -d' ' -f2-)

./coroMutexBenchmark -c "$CORO" -d "$DUMP" -s "$SHARED" -t m -n "$THREADS" -w "$WORKTIME" -o runs
latest_m_file=$(find runs -name "*.csv" -type f -printf "%T@ %p\n" | sort -n | tail -1 | cut -d' ' -f2-)
latest_m_usage_file=$(find runs -name "*.usage" -type f -printf "%T@ %p\n" | sort -n | tail -1 | cut -d' ' -f2-)

source .venv_bench/bin/activate

python gen_bench_graphic.py "$latest_cm_file"
python gen_bench_graphic.py "$latest_m_file"
python gen_bench_graphic_comparison.py "$latest_cm_file"
python gen_bench_usage_diagram.py "$latest_cm_usage_file"
python gen_bench_usage_diagram.py "$latest_m_usage_file"
python gen_bench_usage_diagram_comparison.py "$latest_cm_usage_file"

deactivate

get_basename() {
    local filename=$(basename "$1")
    # Удаляем дату в формате YYYY-MM-DD_HH-MM-SS
    local no_date=$(echo "$filename" | sed -E 's/[0-9]{4}-[0-9]{2}-[0-9]{2}_[0-9]{2}-[0-9]{2}-[0-9]{2}_//')
    # Удаляем target_* часть
    local no_target=$(echo "${no_date%.*}" | sed -E 's/target_[^_]*_//g')
    echo "$no_target"
}

result_dir_name=$(get_basename "$latest_cm_file")
result_dir="runs/$result_dir_name"
mkdir -p "$result_dir"

mv runs/*.csv "$result_dir/" 2>/dev/null || true
mv runs/*.usage "$result_dir/" 2>/dev/null || true
mv runs/*.png "$result_dir/" 2>/dev/null || true
mv runs/*.log "$result_dir/" 2>/dev/null || true