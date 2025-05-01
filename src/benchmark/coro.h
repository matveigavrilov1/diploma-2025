#pragma once

#include <mutex>

#include "benchmark/atomic-counter-logger.h"

#include "core/coro-mutex.h"
#include "core/task.h"

cs::task coroutine(cs::atomicCounterLogger& counter, size_t id, std::atomic<bool>& running, cs::coroMutex& mtx);
cs::task coroutine(cs::atomicCounterLogger& counter, size_t id, std::atomic<bool>& running, std::mutex& mtx);