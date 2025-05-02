#!/bin/bash

./setup_benchmark_venv.sh

# Создаем директорию runs, если ее нет
mkdir -p runs

# Запускаем бенчмарки
./coroMutexBenchmark -c 100 -d 100 -s 10 -t cm -n 5 -w 5 -o runs 
latest_cm_file=$(find runs -name "*.csv" -type f -printf "%T@ %p\n" | sort -n | tail -1 | cut -d' ' -f2-)
latest_cm_usage_file=$(find runs -name "*.usage" -type f -printf "%T@ %p\n" | sort -n | tail -1 | cut -d' ' -f2-)

./coroMutexBenchmark -c 100 -d 100 -s 10 -t m -n 5 -w 5 -o runs
latest_m_file=$(find runs -name "*.csv" -type f -printf "%T@ %p\n" | sort -n | tail -1 | cut -d' ' -f2-)
latest_m_usage_file=$(find runs -name "*.usage" -type f -printf "%T@ %p\n" | sort -n | tail -1 | cut -d' ' -f2-)

# Генерируем графики
source .venv_bench/bin/activate

python gen_bench_graphic.py "$latest_cm_file"
python gen_bench_graphic.py "$latest_m_file"
python gen_bench_graphic_comparison.py "$latest_cm_file"
python gen_bench_usage_diagram.py "$latest_cm_usage_file"
python gen_bench_usage_diagram.py "$latest_m_usage_file"
python gen_bench_usage_diagram_comparison.py "$latest_cm_usage_file"

deactivate

# Функция для получения имени папки (без даты, расширения и target_*)
get_basename() {
    local filename=$(basename "$1")
    # Удаляем дату в формате YYYY-MM-DD_HH-MM-SS
    local no_date=$(echo "$filename" | sed -E 's/[0-9]{4}-[0-9]{2}-[0-9]{2}_[0-9]{2}-[0-9]{2}-[0-9]{2}_//')
    # Удаляем target_* часть
    local no_target=$(echo "${no_date%.*}" | sed -E 's/target_[^_]*_//g')
    echo "$no_target"
}

# Создаем папку для результатов (например, было "threads_5_coro_100_shared_10_target_cm_dump_100_worktime_5",
# станет "threads_5_coro_100_shared_10_dump_100_worktime_5")
result_dir_name=$(get_basename "$latest_cm_file")
result_dir="runs/$result_dir_name"
mkdir -p "$result_dir"

# Перемещаем все файлы в папку результатов
mv runs/*.csv "$result_dir/" 2>/dev/null || true
mv runs/*.usage "$result_dir/" 2>/dev/null || true
mv runs/*.png "$result_dir/" 2>/dev/null || true
mv runs/*.log "$result_dir/" 2>/dev/null || true