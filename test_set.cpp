#include <iostream>
#include "include/skip_set.h"

// 测试skip_set的例子，取自《STL源码剖析》第236页
int main() {
	int i;
	int ia[5] = {0, 1, 2, 3, 4};
	skip_set<int> iset(ia, ia+5);

	std::cout << "size=" << iset.size() << std::endl;

	iset.insert(3);
	std::cout << "size=" << iset.size() << std::endl;
	
	iset.insert(5);
	std::cout << "size=" << iset.size() << std::endl;

	iset.erase(1);
	std::cout << "size=" << iset.size() << std::endl;

	std::ostream_iterator<int> oiter(std::cout, " ");
	copy(iset.begin(), iset.end(), oiter);
	std::cout << std::endl;

	// 使用STL通用的find来查找，可以运行，但是效率低
	auto ite1 = std::find(iset.begin(), iset.end(), 3);
	std::cout << 3 << (ite1 != iset.end() ? " found" : " not found") << std::endl;

	ite1 = std::find(iset.begin(), iset.end(), 1);
	std::cout << 1 << (ite1 != iset.end() ? " found" : " not found") << std::endl;

	// 使用效率更高的成员find进行查找，更加高效
	ite1 = iset.find(3);
	std::cout << 3 << (ite1 != iset.end() ? " found" : " not found") << std::endl;

	ite1 = iset.find(1);
	std::cout << 1 << (ite1 != iset.end() ? " found" : " not found") << std::endl;

	return 0;
}
