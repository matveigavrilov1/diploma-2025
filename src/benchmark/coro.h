#pragma once

#include <mutex>

#include "benchmark/counter/atomic-multiple-counter.h"

#include "core/coro-mutex.h"
#include "core/task.h"

cs::task coroutine(cs::atomicMultipleCounter& counter, size_t id, std::atomic<bool>& running, cs::coroMutex& mtx, size_t counterIdx);
cs::task coroutine(cs::atomicMultipleCounter& counter, size_t id, std::atomic<bool>& running, std::mutex& mtx, size_t counterIdx);