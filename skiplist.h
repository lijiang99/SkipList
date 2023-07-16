#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <iostream>
#include <fstream>
#include <string>

// 检查str是否为有效输入，key和value的默认分隔符为":"
static bool is_valid_string(const std::string &str, const std::string &delimiter = ":") {
	return (str.empty() || str.find(delimiter) == std::string::npos) ? false : true;
}

// 从字符串str中提取key和value
static void string_to_kv(const std::string &str, std::string &key, std::string &value,
		const std::string &delimiter = ":") {
	if (!is_valid_string(str)) return;
	std::string::size_type pos = str.find(delimiter);
	key = str.substr(0, pos);
	value = str.substr(pos+1);
}

// 链表中的节点类
template <typename K, typename V>
class Node {
	private:
		K key; // 键
		V value; // 值

	public:
		int node_level; // 节点所在层级

		// forward为一个数组，根据node_level的大小分配空间
		// 数组的每个元素类型为Node<K, V>*，即一个指向节点的指针
		Node<K, V> **forward;
		// 当索引为i时，forward[i]表示当前节点在第i层时，该节点的后继节点的位置
		// 如下，key为1的节点在level=1时，其下一个节点为key为2的节点
		// 所以key为1的节点的forward[1]为指向key为2的节点的指针
		/*
		 * level 4:           2:B
		 * level 3:           2:B
		 * level 2:           2:B           4:D           6:F
		 * level 1:    1:A    2:B           4:D    5:E    6:F
		 * level 0:    1:A    2:B    3:C    4:D    5:E    6:F
		 */

	public:
		Node() = default;  // 默认构造
		Node(K k, V v, int level); // 构造函数

		// 析构函数，释放forward指向的动态分配的内存
		~Node() { delete [] forward; }

		K get_key() const { return key; } // 获取键Key
		V get_value() const { return value; } // 获取值Value

		void set_value(V v) { value = v; } // 设置Value的值
};

// 节点的有参构造函数(Key, Value, 所在层级--随机生成)
template <typename K, typename V>
Node<K,V>::Node(K k, V v, int level) : key(k), value(v), node_level(level) {
	// forward的大小为level+1，因为层数从0开始
	forward = new Node<K, V>*[level+1];
	// 初始化分配的内存空间，将内存清零
	bzero(forward, sizeof(Node<K, V>*)*(level+1));
}

// 跳表类
template <typename K, typename V>
class SkipList {
	private:
		int _max_level; // 跳表的最大层数（上限）
		int _skip_list_level; // 跳表的当前最高层
		Node<K, V> *_header; // 跳表的头节点
		int _element_count; // 当前跳表中节点的数量

	private:
		int get_random_level(); // 生成随机层数

	public:
		SkipList(int max_level);
		~SkipList();

		// 创建一个新节点
		Node<K, V>* create_node(K key, V value, int level) {
			return new Node<K, V>(key, value, level);
		}

		// 跳表的增删改查
		int insert_element(K key, V value);
		int update_element(K key, V value, bool);
		bool search_element(K key);
		void delete_element(K key);
		void clear();

		void display(); // 打印跳表

		// 数据载入和存盘
		void load_data(const std::string &file_path);
		void dump_data(const std::string &file_path, const std::string &delimiter = ":");

		int size() const { return _element_count; } // 获取当前跳表中节点的数量
};

// 生成随机数作为节点的最高层
template <typename K, typename V>
int SkipList<K, V>::get_random_level() {
	int k = 1;
	while (rand() % 2) ++k;
	return k < _max_level ? k : _max_level;
}

// 跳表的构造函数
template <typename K, typename V>
SkipList<K, V>::SkipList(int max_level) : _max_level(max_level), _skip_list_level(0), _element_count(0) {
	// 创建头节点，头节点的所在层级为_max_level
	// 头节点的forward数组长度为_max_level+1，因为层数从0开始
	_header = new Node<K, V>(K(), V(), _max_level);
}

// 跳表的析构函数
template <typename K, typename V>
SkipList<K, V>::~SkipList() {
	// 释放头节点
	delete _header;
}

/*
 * +-------------+
 * |select key=60|
 * +-------------+
 *
 * level 4:--->1+                                           100
 * level 3:    1+------------>10+-------->50+         70    100
 * level 2:    1              10          50+         70    100
 * level 1:    1    4         10    30    50+         70    100
 * level 0:    1    4    9    10    30    50+-->60    70    100
 */

// 在跳表中根据键来查找元素
template <typename K, typename V>
bool SkipList<K, V>::search_element(K key) {
	std::cout << "=> search element..." << std::endl;
	Node<K, V> *current = _header; // 从头节点开始

	// 从跳表的最高层开始查找
	for (int i = _skip_list_level; i >= 0; --i) {
		// 若当前节点的后继不为空且后继的key小于目标key
		// 表明需要在当前层继续前进，即继续while循环
		while (current->forward[i] != nullptr && current->forward[i]->get_key() < key)
			current = current->forward[i];
		// 若当前节点的后继为空或后继的key大于等于目标的key
		// 表明需要向下降一层级，即结束while循环，开启下一次for循环
	}

	// 当for循环结束时，表明已经查找到了第0层级
	// 且此时的current节点必定是跳表中满足key小于目标key的所有节点中，key最大的那个节点
	// 若key存在，则current的后继即为所要查找的目标节点
	current = current->forward[0];

	if (current != nullptr && current->get_key() == key) {
		std::cout << "** found (key, value): (" << key << ", " << current->get_value() << ")" << std::endl;
		return true;
	}
	else {
		std::cout << "** not found key: " << key << std::endl;
	}
	return false;
}

// 打印跳表中的数据
template <typename K, typename V>
void SkipList<K, V>::display() {
	std::cout << "=> print skip list..." << std::endl;
	// 从第0层开始打印
	for (int i = 0; i <= _skip_list_level; i++) {
		Node<K, V> *node = _header->forward[i];
		std::cout << "** level " << i << ": ";
		while (node != nullptr) {
			std::cout << node->get_key() << ": " << node->get_value() << "; ";
			node = node->forward[i];
		}
		std::cout << std::endl;
	}
}

// 从文件中加载数据
template <typename K, typename V>
void SkipList<K, V>::load_data(const std::string &file_path) {
	std::cout << "=> load data from: '" << file_path << "'..." << std::endl;
	std::ifstream fin(file_path);
	std::string line, key, value;
	while (std::getline(fin, line)) {
		string_to_kv(line, key, value);
		if (key.empty() || value.empty()) continue;
		insert_element(key, value);
	}
	fin.close();
}

// 将数据写入至文件，默认的分隔符为":"
template<typename K, typename V>
void SkipList<K, V>::dump_data(const std::string &file_path, const std::string &delimiter) {
	std::cout << "=> dump data to: '" << file_path << "'..." << std::endl;
	std::ofstream fout(file_path);
	// 将第0层的数据写入文件即可
	Node<K, V> *node = _header->forward[0];
	while (node != nullptr) {
		fout << node->get_key() << delimiter << node->get_value() << "\n";
		node = node->forward[0];
	}
	fout.close();
}

#endif
