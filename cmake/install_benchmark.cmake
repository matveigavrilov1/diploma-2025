set(BENCHMARK_INSTALL_DIR ${CMAKE_SOURCE_DIR}/${CMAKE_BUILD_TYPE}-benchmark-${GIT_COMMIT_HASH}-${PROJECT_VERSION})

install(
	FILES
		${CMAKE_SOURCE_DIR}/build/src/benchmark/coroMutexBenchmark
	DESTINATION ${BENCHMARK_INSTALL_DIR}
	PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
)

install(
	FILES
		tools/gen_bench_graphic_comparison.py
		tools/gen_bench_graphic.py
		tools/gen_bench_usage_diagram_comparison.py
		tools/gen_bench_usage_diagram.py
		tools/gen_summary.py
		tools/run_benchmark.sh
		tools/run.sh
		tools/setup_benchmark_venv.sh
	DESTINATION ${BENCHMARK_INSTALL_DIR}
	PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
)