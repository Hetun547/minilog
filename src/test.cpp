
#include "minilog.h"

#include <cstdio>
#include <iostream>
#include <tuple>
template <typename... Args>
void print(char* format, Args... args) {
	//auto tt = std::make_tuple(args...);
	//if constexpr (std::tuple_size_v<decltype(tt)> > 1) {
	//	auto sdd = std::get<1>(tt);
	//	if constexpr (std::is_same_v<char, decltype(sdd)> || std::is_convertible_v<decltype(sdd), std::string>)
	//		std::cout << sdd << std::endl;
	//}
	char sd[128] = { 0 };
	std::sprintf(sd, format, args...);
	
	return;
}

#include <chrono>
#include <thread>
using namespace std::chrono_literals;
int main()
{
	auto path = std::filesystem::current_path();
	path.append("log");
	minilog::init(path.string().c_str());
	srand(time(0));
	for (int i = 0; i < 100; ++i) {
		error("sds", "%d\n", rand());
		std::this_thread::sleep_for(1ms);
	}

	debug("sds", "test for debug\n");
	system("pause");
	return 0;
}