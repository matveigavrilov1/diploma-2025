#!/bin/bash

BENCH_VENV_DIR=".venv_bench"

if ! command -v python3 &> /dev/null
then
	echo "Error: python3 was not found"
	exit 1
fi

if ! command -v pip &> /dev/null
then
	echo "Error: pip was not found"
	exit 1
fi
echo "Creating virtual environment..."
python3 -m venv $BENCH_VENV_DIR

echo "Activating virtual environment..."
source ${BENCH_VENV_DIR}/bin/activate

echo "Installing dependencies.."
pip install pandas matplotlib
