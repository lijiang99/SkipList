#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <cmath>

std::mutex mtx; // 互斥锁

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
		int node_level; // 节点所在层级

	public:
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
		// 构造函数
		Node(K k, V v, int level);

		// 析构函数，释放forward指向的动态分配的内存
		~Node() { delete [] forward; }

		// 获取key/value
		K get_key() const { return key; }
		V get_value() const { return value; }

		// 设置value
		void set_value(V v) { value = v; }
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
		// 由于层数从0开始索引，所以实际层数要+1
		int _max_level; // 跳表的最大层数（上限）
		int _skip_list_level; // 跳表的当前最高层
		Node<K, V> *_header; // 跳表的头节点
		int _element_count; // 当前跳表中节点的数量

	private:
		// 生成随机层数
		int get_random_level();
		// 创建一个新节点
		Node<K, V>* create_node(K key, V value, int level) {
			return new Node<K, V>(key, value, level);
		}

	public:
		// 构造和析构函数
		SkipList(int max_level);
		~SkipList();

		// 跳表的增删改查
		int insert_element(K key, V value);
		bool search_element(K key);
		void delete_element(K key);
		void clear();

		// 打印跳表
		void display();

		// 数据载入和存盘
		void load_data(const std::string &file_path);
		void dump_data(const std::string &file_path, const std::string &delimiter = ":");

		// 获取当前跳表中节点的数量
		int size() const { return _element_count; }
		// 判断跳表是否为空
		bool empty() const { return _element_count == 0; }
};

// 生成随机数作为节点的最高层
template <typename K, typename V>
int SkipList<K, V>::get_random_level() {
	int k = 1;
	// 控制每次增长的概率为50%
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
 * |insert key=50|
 * +-------------+
 *
 * level 4:--->1+                  insert+--+               100
 * level 3:    1+------------>10+------->|50|         70    100
 * level 2:    1              10         |50|         70    100
 * level 1:    1    4         10    30   |50|         70    100
 * level 0:    1    4    9    10    30   |50|   60    70    100
 */

// 在跳表中插入节点，若key不存在，则直接插入，并返回0
// 若key已存在于跳表中，则修改key对应的value，并返回1
template <typename K, typename V>
int SkipList<K, V>::insert_element(K key, V value) {
	mtx.lock(); // 对于修改跳表的操作需要加锁
#ifndef NDEBUG
	std::cout << "=> insert element..." << std::endl;
#endif
	Node<K, V> *current = _header; // 从头节点开始

	// 使用update来保存每层中最后一个满足其key小于待插入key的节点（即前驱节点）
	// update的大小设置为_max_level+1从而确保有足够的空间来存放每层满足条件的节点
	Node<K, V> *update[_max_level+1];
	bzero(update, sizeof(Node<K, V>*)*(_max_level+1));

	// 从跳表的最高层开始查找
	for (int i = _skip_list_level; i >= 0; --i) {
		// 若当前节点的后继不为空且后继的key小于目标key
		// 表明需要在当前层继续前进，即继续while循环
		while (current->forward[i] != nullptr && current->forward[i]->get_key() < key)
			current = current->forward[i];
		// 若当前节点的后继为空或后继的key大于等于目标的key
		// 则此时的current即为待插入的目标key的前一个位置
		// 那么将current的值保存至update[i]中
		update[i] = current;
		// 开启下一次for循环，向下降一层级继续查找
	}

	// 当for循环结束时，表明已经查找到了第0层级
	// 那么current->forward[0]则表示第0层中第一个满足其key大于或等于待插入的key的节点
	current = current->forward[0];

	// 情况1: 待插入的key已经存在于跳表中，则更新对应的value即可
	if (current != nullptr && current->get_key() == key) {
#ifndef NDEBUG
		std::cout << "** successfully updated: (" << key << ", " << current->get_value()
			<< ") -> (" << key << ", " << value << ")" << std::endl;
#endif
		current->set_value(value);
		mtx.unlock(); // 解锁
		return 1;
	}

	// 情况2: 待插入的key不存在于跳表中，则需创建新节点插入，并根据随机数设置层级
	if (current == nullptr || current->get_key() != key) {
		// 为待插入的节点生成随机层数
		// 层索引从0开始，所以实际层数为level+1
		int random_level = get_random_level();

		// 若random_level大于当前的最高层_skip_list_level(不是_max_level)
		// 则表明在层级为[_skip_list_level+1, random_level]内，待插入节点的前驱必为_header
		if (random_level > _skip_list_level) {
			for (int i = _skip_list_level+1; i <= random_level; ++i)
				update[i] = _header;
			_skip_list_level = random_level; // 更新当前最高层
		}

		// 创建待插入的新节点
		Node<K, V> *node = create_node(key, value, random_level);

		// 插入新节点，从第0层开始修改前驱及后继
		for (int i = 0; i <= random_level; ++i) {
			// 将待插入节点的后继设置为事先所保存的前驱节点的后继
			node->forward[i] = update[i]->forward[i];
			// 更新事先所保存的前驱节点的后继为当前节点
			update[i]->forward[i] = node;
		}

		++_element_count; // 更新跳表中的节点总数
#ifndef NDEBUG
		std::cout << "** successfully inserted: (" << key << ", " << value << ")" << std::endl;
#endif
	}
	mtx.unlock(); // 解锁
	return 0;
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

// 在跳表中根据键来查找节点
template <typename K, typename V>
bool SkipList<K, V>::search_element(K key) {
#ifndef NDEBUG
	std::cout << "=> search element..." << std::endl;
#endif
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
#ifndef NDEBUG
		std::cout << "** successfully found: (" << key << ", " << current->get_value() << ")" << std::endl;
#endif
		return true;
	}
#ifndef NDEBUG
	std::cout << "** not found key: " << key << std::endl;
#endif
	return false;
}

// 在跳表中根据键来删除节点
template <typename K, typename V>
void SkipList<K, V>::delete_element(K key) {
	mtx.lock(); // 对于修改跳表的操作需要加锁
#ifndef NDEBUG
	std::cout << "=> delete element..." << std::endl;
#endif
	Node<K, V> *current = _header; // 从头节点开始

	// 使用update来保存每层中最后一个满足其key小于待删除key的节点（即前驱节点）
	// update的大小设置为_max_level+1从而确保有足够的空间来存放每层满足条件的节点
	Node<K, V> *update[_max_level+1];
	bzero(update, sizeof(Node<K, V>*)*(_max_level+1));

	// 从跳表的最高层开始查找
	for (int i = _skip_list_level; i >= 0; --i) {
		// 若当前节点的后继不为空且后继的key小于目标key
		// 表明需要在当前层继续前进，即继续while循环
		while (current->forward[i] != nullptr && current->forward[i]->get_key() < key)
			current = current->forward[i];
		// 若当前节点的后继为空或后继的key大于等于目标的key
		// 则此时的current即为待删除的目标key的前一个位置
		// 那么将current的值保存至update[i]中
		update[i] = current;
		// 开启下一次for循环，向下降一层级继续查找
	}

	// 当for循环结束时，表明已经查找到了第0层级
	// 那么current->forward[0]则表示第0层中第一个满足其key大于或等于待删除的key的节点
	current = current->forward[0];

	// 待删除的key存在于跳表中，则删除对应的节点
	if (current != nullptr && current->get_key() == key) {
		// 从第0层开始修改前驱及后继
		for (int i = 0; i <= _skip_list_level; ++i) {
			// 若前驱的后继不再是待删除的节点，则退出循环
			if (update[i]->forward[i] != current) break;
			// 将前驱的后继修改为待删除节点的后继
			update[i]->forward[i] = current->forward[i];
		}
#ifndef NDEBUG
		V value = current->get_value();
#endif
		delete current; // 释放节点所占用的内存空间

		// 由于删除的节点的层级可能为当前跳表的唯一最大层
		// 因此删除节点后，需要更新当前跳表的最大层级
		// 若头节点在最高层的后继为空，则表明最高层为空，需要降低最高层
		while (_skip_list_level > 0 && _header->forward[_skip_list_level] == nullptr)
			--_skip_list_level;

		--_element_count; // 更新跳表中的节点总数

#ifndef NDEBUG
		std::cout << "** successfully deleted: (" << key << ", " << value <<")" << std::endl;
#endif
	} else {
#ifndef NDEBUG
		std::cout << "** not found key: " << key << std::endl;
#endif
	}
	mtx.unlock(); // 解锁
}

// 清空跳表，释放所有节点
template <typename K, typename V>
void SkipList<K, V>::clear() {
	mtx.lock(); // 对于修改跳表的操作需要加锁
#ifndef NDEBUG
	std::cout << "=> clear skip list..." << std::endl;
#endif
	// 从第0层的头节点的后继开始
	Node <K, V> *node = _header->forward[0];
	while (node != nullptr) {
		Node <K, V> *tmp = node;
#ifndef NDEBUG
		std::cout << "** successfully deleted: (" << tmp->get_key() << ", " << tmp->get_value() << ")" << std::endl;
#endif
		node = node->forward[0];
		delete tmp;
	}

	// 重新初始化_header
	bzero(_header->forward, sizeof(Node<K, V>*)*(_skip_list_level+1));
	_skip_list_level = 0;
	_element_count = 0;

	mtx.unlock(); // 解锁
}

// 打印跳表中的数据
template <typename K, typename V>
void SkipList<K, V>::display() {
	std::cout << "=> display skip list..." << std::endl;
	// 从最高层开始打印
	for (int i = _skip_list_level; i >= 0; --i) {
		Node<K, V> *node = _header->forward[i];
		std::cout << "** level " << i << ": ";
		Node<K, V> *tmp = _header->forward[0];
		// 为了方便理解，所以手动指定了输出补白
		// 因此跳表中的key和value最好是单个字符
		while (node != nullptr) {
			while (tmp != nullptr && node != tmp) {
				std::cout << std::setw(7) << " ";
				tmp = tmp->forward[0];
			}
			std::cout << std::setw(4) << " " << node->get_key() << ":" << node->get_value();
			tmp = tmp->forward[0];
			node = node->forward[i];
		}
		std::cout << std::endl;
	}
}

// 从文件中加载数据
template <typename K, typename V>
void SkipList<K, V>::load_data(const std::string &file_path) {
#ifndef NDEBUG
	std::cout << "=> load data from: '" << file_path << "'..." << std::endl;
#endif
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
#ifndef NDEBUG
	std::cout << "=> dump data to: '" << file_path << "'..." << std::endl;
#endif
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
