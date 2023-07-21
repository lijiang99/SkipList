#include <iostream>
#include <iomanip>
#include <random>
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include "include/skip_set.h"

std::mutex mtx;
skip_set<size_t> iset;

void insert(size_t item_nums, size_t thread_nums) {
	size_t count = item_nums / thread_nums;
	static std::default_random_engine e(42);
	static std::uniform_int_distribution<size_t> u(0, item_nums);
	for (size_t i = 0; i < count; ++i) {
		std::lock_guard<std::mutex> lock(mtx);
		size_t tmp = u(e);

#ifndef NDEBUG
		std::cout << "thread: " << std::this_thread::get_id()
			<< " insert key => " << tmp << std::endl;
#endif

		auto ret = iset.insert(tmp);

#ifndef NDEBUG
		std::cout << std::setw(8) << " ";
		if (ret.second)
			std::cout << "**successfully inserted" << std::endl;
		else
			std::cout << "**cannot insert duplicate key" << std::endl;
#endif
	}

#ifndef NDEBUG
	std::cout << "thread: " << std::this_thread::get_id()
		<< " finished" << std::endl;
#endif
}

void find(size_t item_nums, size_t thread_nums) {
	size_t count = item_nums / thread_nums;
	static std::default_random_engine e(3407);
	static std::uniform_int_distribution<size_t> u(0, item_nums);
	for (size_t i = 0; i < count; ++i) {
#ifndef NDEBUG
		std::lock_guard<std::mutex> lock(mtx);
#endif
		size_t tmp = u(e);

#ifndef NDEBUG
		std::cout << "thread: " << std::this_thread::get_id()
			<< " find key => " << tmp << std::endl;
#endif

		// 使用成员find
		auto ret = iset.find(tmp);

#ifndef NDEBUG
		std::cout << std::setw(8) << " ";
		if (ret != iset.end())
			std::cout << "**found" << std::endl;
		else
			std::cout << "**not found" << std::endl;
#endif
	}

#ifndef NDEBUG
	std::cout << "thread: " << std::this_thread::get_id()
		<< " finished" << std::endl;
#endif
}

void stl_find(size_t item_nums, size_t thread_nums) {
	size_t count = item_nums / thread_nums;
	static std::default_random_engine e(3407);
	static std::uniform_int_distribution<size_t> u(0, item_nums);
	for (size_t i = 0; i < count; ++i) {
#ifndef NDEBUG
		std::lock_guard<std::mutex> lock(mtx);
#endif
		size_t tmp = u(e);

#ifndef NDEBUG
		std::cout << "thread: " << std::this_thread::get_id()
			<< " find key => " << tmp << std::endl;
#endif

		// 使用通用find
		auto ret = std::find(iset.begin(), iset.end(), tmp);

#ifndef NDEBUG
		std::cout << std::setw(8) << " ";
		if (ret != iset.end())
			std::cout << "**found" << std::endl;
		else
			std::cout << "**not found" << std::endl;
#endif
	}

#ifndef NDEBUG
	std::cout << "thread: " << std::this_thread::get_id()
		<< " finished" << std::endl;
#endif
}


int main(int argc, char* argv[]) {
	if (argc != 3) {
		std::cout << "usage: " << argv[0]
			<< " <item_nums> <thread_nums>" << std::endl;
		exit(1);
	}

	size_t item_nums = std::stoi(argv[1]);
	size_t thread_nums = std::stoi(argv[2]);

	std::cout << "stress test: => ["
		<< item_nums << ", " << thread_nums << "]" << std::endl;

	std::cout << std::fixed << std::setprecision(7);

	// 插入测试
	{
		std::vector<std::thread> threads;
		threads.reserve(thread_nums);

		auto start = std::chrono::high_resolution_clock::now();
		for (size_t i = 0; i < thread_nums; ++i)
			threads.push_back(std::thread(insert, item_nums, thread_nums));
		for (size_t i = 0; i < thread_nums; ++i)
			threads[i].join();
		auto finish = std::chrono::high_resolution_clock::now();

		std::chrono::duration<double> elapsed = finish - start;
		std::cout << "insert elapsed: " << elapsed.count() << std::endl;
	}

	// 查找测试，使用高效的成员find
	{
		std::vector<std::thread> threads;
		threads.reserve(thread_nums);

		auto start = std::chrono::high_resolution_clock::now();
		for (size_t i = 0; i < thread_nums; ++i)
			threads.push_back(std::thread(find, item_nums, thread_nums));
		for (size_t i = 0; i < thread_nums; ++i)
			threads[i].join();
		auto finish = std::chrono::high_resolution_clock::now();

		std::chrono::duration<double> elapsed = finish - start;
		std::cout << "find elapsed: " << elapsed.count() << std::endl;
	}

	// 查找测试，使用低效的通用find
	{
		std::vector<std::thread> threads;
		threads.reserve(thread_nums);

		auto start = std::chrono::high_resolution_clock::now();
		for (size_t i = 0; i < thread_nums; ++i)
			threads.push_back(std::thread(stl_find, item_nums, thread_nums));
		for (size_t i = 0; i < thread_nums; ++i)
			threads[i].join();
		auto finish = std::chrono::high_resolution_clock::now();

		std::chrono::duration<double> elapsed = finish - start;
		std::cout << "stl find elapsed: " << elapsed.count() << std::endl;
	}

	return 0;
}
