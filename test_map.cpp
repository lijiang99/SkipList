#include <iostream>
#include <string>
#include "include/skip_map.h"

// 测试skip_map的例子，取自《STL源码剖析》第242页
int main() {
	skip_map<std::string, int> simap;
	simap[std::string("jjhou")] = 1;
	simap[std::string("jerry")] = 2;
	simap[std::string("jason")] = 3;
	simap[std::string("jimmy")] = 4;

	std::pair<std::string, int> value(std::string("david"), 5);
	simap.insert(value);

	skip_map<std::string, int>::iterator simap_iter = simap.begin();
	for (; simap_iter != simap.end(); ++simap_iter)
		std::cout << simap_iter->first << " " << simap_iter->second << std::endl;

	int number = simap[std::string("jjhou")];
	std::cout << number << std::endl;

	skip_map<std::string, int>::iterator ite1;

	// 对于关联容器，应该用成员find来查找元素，比使用STL通用的find效率更高
	ite1 = simap.find(std::string("mchen"));
	std::cout << "mchen" << (ite1 == simap.end() ? " not found" : " found") << std::endl;

	ite1 = simap.find(std::string("jerry"));
	std::cout << "jerry" << (ite1 == simap.end() ? " not found" : " found") << std::endl;

	// 可以通过skip_map的迭代器修改实值
	ite1->second = 9;
	int number2 = simap[std::string("jerry")];
	std::cout << number2 << std::endl;

	return 0;
}
