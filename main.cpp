#include <iostream>
#include <string>
#include "skiplist.h"

#define FILE_PATH "dump_file"
#define MAX_LEVEL 8

int main() {
	SkipList<std::string, std::string> skip_list(MAX_LEVEL);

	// 测试插入
	std::cout << "========== Insert Test ==========" << std::endl;
	skip_list.insert_element("7", "R");
	skip_list.display();
	std::cout << std::endl;

	skip_list.insert_element("9", "D");
	skip_list.display();
	std::cout << std::endl;

	skip_list.insert_element("2", "J");
	skip_list.display();
	std::cout << std::endl;

	skip_list.insert_element("8", "L");
	skip_list.display();
	std::cout << std::endl;

	skip_list.insert_element("6", "O");
	skip_list.display();
	std::cout << std::endl;

	skip_list.insert_element("0", "H");
	skip_list.display();
	std::cout << std::endl;

	skip_list.insert_element("2", "L");
	skip_list.display();
	std::cout << std::endl;

	skip_list.insert_element("1", "E");
	skip_list.display();
	std::cout << std::endl;

	skip_list.insert_element("5", "W");
	skip_list.display();
	std::cout << std::endl;

	skip_list.insert_element("3", "L");
	skip_list.display();
	std::cout << std::endl;

	skip_list.insert_element("4", "O");
	skip_list.display();
	std::cout << std::endl;

	// 存盘测试
	std::cout << "========== Dump Test ==========" << std::endl;
	skip_list.dump_data(FILE_PATH);
	std::cout << std::endl;

	// 查找测试
	std::cout << "========== Search Test ==========" << std::endl;
	skip_list.search_element("7");
	std::cout << std::endl;

	skip_list.search_element("9");
	std::cout << std::endl;

	skip_list.search_element("0");
	std::cout << std::endl;

	skip_list.search_element("10");
	std::cout << std::endl;

	// 删除测试
	std::cout << "========== Delete Test ==========" << std::endl;
	skip_list.delete_element("0");
	skip_list.display();
	std::cout << std::endl;

	skip_list.delete_element("7");
	skip_list.display();
	std::cout << std::endl;

	skip_list.delete_element("9");
	skip_list.display();
	std::cout << std::endl;

	skip_list.delete_element("0");
	skip_list.display();
	std::cout << std::endl;

	// 清空测试
	std::cout << "========== Clear Test ==========" << std::endl;
	skip_list.clear();
	std::cout << "** skip list size: " << skip_list.size() << std::endl;
	std::cout << "** skip list status: " << (skip_list.empty() ? "empty" : "not empty") << std::endl;
	std::cout << std::endl;

	// 载入测试
	std::cout << "========== Load Test ==========" << std::endl;
	skip_list.load_data(FILE_PATH);
	skip_list.display();
	std::cout << "** skip list size: " << skip_list.size() << std::endl;
	std::cout << "** skip list status: " << (skip_list.empty() ? "empty" : "not empty") << std::endl;
	std::cout << std::endl;

	return 0;
}
