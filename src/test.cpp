
#include "minilog.h"

#include <cstdio>
#include <iostream>
#include <tuple>

#include <chrono>
#include <thread>
using namespace std::chrono_literals;
int main()
{
	auto path = std::filesystem::current_path();
	path.append("log");
	minilog::init(path.string().c_str(), 1);
	srand(time(0));
	for (int i = 0; i < 100; ++i) {
		error("sds", "%d\n", rand());
		std::this_thread::sleep_for(1ms);
	}

	debug("sds", "test for debug\n");
	system("pause");
	return 0;
}
