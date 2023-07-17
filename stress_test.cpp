#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <pthread.h>
#include "skiplist.h"

SkipList<int, std::string> skiplist(18);

struct Test {
	int node_nums; // 待插入的节点数量
	int thread_nums; // 工作线程的数量
} test;

// 线程所调用的插入测试函数
void* insert_test(void *test_item) {
	Test *item = (Test*)test_item;

	// tmp为每个线程需要向跳表中插入的节点数
	int tmp = item->node_nums/item->thread_nums;
	for (int count = 0; count < tmp; ++count) {
		// 生成随机数作为key
		skiplist.insert_element(rand() % item->node_nums, "A");
	}

	pthread_exit(NULL);
}

// 线程所调用的查找测试函数
void* search_test(void *test_item) {
	Test *item = (Test*)test_item;

	// tmp为每个线程需要在跳表中查找的节点数
	int tmp = item->node_nums/item->thread_nums;
	for (int count = 0; count < tmp; ++count)
		// 生成随机数作为key
		skiplist.search_element(rand() % item->node_nums);

	pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
	// 根据系统时间设置随机数种子seed
	srand(time(NULL));

	if (argc != 3) {
		std::cout << "usage: " << argv[0] << " <node nums> <thread nums>" << std::endl;
		exit(1);
	}

	// 通过命令行参数设置节点数和线程数
	test.node_nums = atoi(argv[1]);
	test.thread_nums = atoi(argv[2]);

	std::cout << "========== Stress Test ==========" << std::endl;
	std::cout << "node nums: " << test.node_nums << std::endl;
	std::cout << "thread nums: " << test.thread_nums << std::endl;

	// 插入测试
	{
		// 数组中每个元素为一个线程的标识
		pthread_t threads[test.thread_nums];

		// 记录开始时间
		auto start = std::chrono::high_resolution_clock::now();

		for (int i = 0; i < test.thread_nums; ++i) {
			// std::cout << "creating thread: " << i << std::endl;
			// 创建线程，并执行插入测试函数
			if (pthread_create(&threads[i], NULL, insert_test, (void*)&test) != 0) {
				std::cout << "pthread_create() error" << std::endl; 
				exit(-1);
			}
		}

		void *ret;
		for (int i = 0; i < test.thread_nums; ++i) {
			// 等待线程结束
			if (pthread_join(threads[i], &ret) != 0) {
				perror("pthread_join() error");
				exit(-1);
			}
			// std::cout << "thread: " << i << " finished" << std::endl;
		}

		// 记录结束时间
		auto finish = std::chrono::high_resolution_clock::now();
		// 计算运行耗时
		std::chrono::duration<double> elapsed = finish - start;
		std::cout << "insert elapsed: " << elapsed.count() << std::endl;

		// skiplist.display();
	}

	// 查找测试
	{
		// 数组中每个元素为一个线程的标识
		pthread_t threads[test.thread_nums];

		// 记录开始时间
		auto start = std::chrono::high_resolution_clock::now();

		for (int i = 0; i < test.thread_nums; ++i) {
			// std::cout << "creating thread: " << i << std::endl;
			// 创建线程，并执行查找测试函数
			if (pthread_create(&threads[i], NULL, search_test, (void*)&test) != 0) {
				std::cout << "pthread_create() error" << std::endl; 
				exit(-1);
			}
		}

		void *ret;
		for (int i = 0; i < test.thread_nums; ++i) {
			// 等待线程结束
			if (pthread_join(threads[i], &ret) != 0) {
				perror("pthread_join() error");
				exit(-1);
			}
			// std::cout << "thread: " << i << " finished" << std::endl;
		}

		// 记录结束时间
		auto finish = std::chrono::high_resolution_clock::now();
		// 计算运行耗时
		std::chrono::duration<double> elapsed = finish - start;
		std::cout << "search elapsed: " << elapsed.count() << std::endl;

		// skiplist.display();
	}

	return 0;
}
