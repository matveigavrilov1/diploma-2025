#include <iostream>

#include "core/coro-mutex.h"

int main()
{
    cs::coroMutex mtx;
    mtx.lock();
	std::cout << "Hello" << std::endl;
	return 0;
}