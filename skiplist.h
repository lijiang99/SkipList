#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <iostream>
#include <iomanip>
#include <iterator>
#include <memory>
#include <cstring>
#include <string>
#include <strings.h>
#include <utility>

// skiplist的节点
template <typename Value>
struct __skiplist_node {
	typedef __skiplist_node<Value>* link_type;

	// 节点值
	Value value_field;
	// 节点层级
	size_t level;
	// forward是大小为level+1的数组
	// 元素为指针，指向当前节点在每层的后继节点
	link_type *forward;

	// 构造函数
	__skiplist_node(Value value_field, size_t level) : value_field(value_field), level(level) {
		// forward的大小为level+1，因为层数从0开始
		forward = new link_type[level+1];
		// 初始化分配的内存空间，将内存清零
		bzero(forward, sizeof(link_type)*(level+1));
	} 
	// 析构函数，释放forward指向的动态分配的内存
	~__skiplist_node() { delete [] forward; }
};

// skiplist的迭代器
template <typename Value, typename Ref, typename Ptr>
class __skiplist_iterator {
	public:
		// 定义迭代器的五种特性，使其适配iterator_traits进行特性提取，并兼容STL算法
		typedef Value value_type;
		typedef Ref reference;
		typedef Ptr pointer;
		// 迭代器类型为前向迭代器
		typedef std::forward_iterator_tag iterator_category;
		typedef ptrdiff_t difference_type;

		typedef __skiplist_iterator<Value, Value&, Value*> iterator;
		typedef __skiplist_iterator<Value, const Value&, const Value*> const_iterator;

	private:
		typedef __skiplist_iterator<Value, Ref, Ptr> self;
		typedef __skiplist_node<Value>* link_type;
		// 迭代器的唯一数据就是一个原生指针
		link_type node;

	public:
		// 构造函数
		__skiplist_iterator(link_type x) : node(x) {}
		__skiplist_iterator(const iterator &it) : node(it.node) {}

		// 重载解引用运算符
		reference operator*() const { return node->value_field; }
		// 重载成员访问运算符的标准操作
		pointer operator->() const { return &(operator*()); }

		// 重载递增运算符，因为是单向迭代器类型，所以不支持递减运算符
		self& operator++() { node = node->forward[0]; return *this; } // 前置递增
		self& operator++(int) { self tmp = *this; node = node->forward[0]; return *this; } // 后置递增

		// 重载==和!=运算符，用于条件判断
		bool operator==(const iterator &it) const { return node == it.node; }
		bool operator!=(const iterator &it) const { return node != it.node; }
};

// skiplist类
template <typename Key, typename Value, typename KeyOfValue, typename Compare>
class skiplist {
	public:
		// 定义跳表的基础类型
		typedef Key key_type;
		typedef Value value_type;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;

		// 定义跳表的专属迭代器
		typedef __skiplist_iterator<value_type, reference, pointer> iterator;
		typedef __skiplist_iterator<value_type, const_reference, const_pointer> const_iterator;

	private:
		typedef __skiplist_node<Value> skiplist_node;
		typedef skiplist_node* link_type;

		// 层级上限
		size_type max_level;
		// 当前最高层
		size_type top_level;
		// 跳表的大小，即节点数量
		size_type node_count;
		// 作为节点间键值大小比较准则的函数对象
		Compare key_compare;
		// 头节点
		link_type header;


	private:
		// 生成随机数作为节点层级
		size_type random_level();
		// 创建一个节点
		link_type create_node(const value_type &val, size_t level) { return new skiplist_node(val, level); }
		// 销毁一个节点
		void destroy_node(link_type node) { delete node; }
		// 初始化头节点
		void init() { header = create_node(value_type(), max_level); }

		// 用于获得节点的value和key
		static reference value(link_type x) { return x->value_field; } //有用
		static const Key& key(link_type x) { return KeyOfValue()(value(x)); } //有用

		iterator __insert(link_type *update, const value_type &val);
		void __erase(link_type x);
		




	public:
		// 构造函数
		skiplist(size_type max_level, const Compare &comp = Compare())
			: max_level(max_level), top_level(0), node_count(0), key_compare(comp) { init(); }
		// 析构函数，需要先清空跳表，再释放头节点
		~skiplist() { clear(); destroy_node(header); }
		// 拷贝构造函数
		skiplist(const skiplist &rhs);
		skiplist& operator=(const skiplist &rhs);
		
		// 获取作为节点间键值大小比较准则的函数对象
		Compare key_comp() const { return key_compare; }

		// 首尾迭代器，首迭代器即头节点在第0层的后继
		// 因为是单向链表，所以无反向迭代器
		iterator begin() const { return header->forward[0]; }
		const_iterator cbegin() const { return header->forward[0]; }
		iterator end() const { return nullptr; }
		const_iterator cend() const { return nullptr; }

		// 定义数据规模的相关函数
		bool empty() const { return node_count == 0; }
		size_type size() const { return node_count; }
		size_type max_size() const { return size_type(-1); }

		// 将节点插入跳表中，并保证节点唯一
		std::pair<iterator, bool> insert_unique(const value_type &val);
		// iterator insert_equal(const value_type &val);
		// 根据key在跳表中查找节点
		iterator find(const key_type &k);

		// 清空跳表
		void clear();

#ifndef NDEBUG
		// 打印跳表（仅限于调试）
		void display() const;
#endif
};

// 生成随机数作为节点层级
template <typename Key, typename Value, typename KeyOfValue, typename Compare>
typename skiplist<Key, Value, KeyOfValue, Compare>::size_type
skiplist<Key, Value, KeyOfValue, Compare>::random_level() {
	size_type level = 1;
	// 每次层级向上增长的概率为50%
	while (rand() % 2) { if (++level >= max_level) return max_level; }
	return level;
}

// 将节点插入跳表中，并保证节点唯一
// 若待插入节点的key不存在，则插入成功，并返回新节点的迭代器和true
// 若待插入节点的key已存在，则插入失败，并返回key相同的节点的迭代器和false
template <typename Key, typename Value, typename KeyOfValue, typename Compare>
std::pair<typename skiplist<Key, Value, KeyOfValue, Compare>::iterator, bool>
skiplist<Key, Value, KeyOfValue, Compare>::insert_unique(const value_type &val) {
#ifndef NDEBUG
	std::cout << "call: skiplist.insert_unique ";
#endif
	link_type current = header;
	// 使用update来保存每层中最后一个满足其key小于待插入节点的key的节点（即前驱节点)
	// update大小设置为max_level+1以确保有足够的空间来存放每层满足条件的节点
	link_type update[max_level+1];
	bzero(update, sizeof(link_type)*(max_level+1));

	// 从跳表最高层开始查找
	for (int i = top_level; i >= 0; --i) {
		// 若当前节点的后继不为空且后继的key小于待插入的节点的key
		// 表明需要在当前层继续前进，继续while循环
		while (current->forward[i] && key_compare(key(current->forward[i]), KeyOfValue()(val)))
			current = current->forward[i];
		// 若当前节点的后继为空或后继节点的key大于等于目标节点的key
		// 则current此时即为待插入节点的前一个位置（前驱节点），将其保存到update中
		update[i] = current;
	}

	// 当for循环结束时，表明已经查到了第0层
	// 那么current->forward[0]的key此时可能等于或大于待插入节点的key
	current = current->forward[0];
	
	// 若待插入的key已经存在于跳表中，则不插入新值
	if (current && !key_compare(KeyOfValue()(val), key(current))) {
#ifndef NDEBUG
		std::cout << std::endl << std::setw(6) << " "
			<< "** can not repeatedly inserted key: " << KeyOfValue()(val) << std::endl;
#endif
		return std::pair<iterator, bool>(current, false);
	}
	return std::pair<iterator, bool>(__insert(update, val), true);
}



/*
// 将节点插入跳表中，并允许节点重复
template <typename Key, typename Value, typename KeyOfValue, typename Compare>
typename skiplist<Key, Value, KeyOfValue, Compare>::iterator
skiplist<Key, Value, KeyOfValue, Compare>::insert_equal(const value_type &val) {
#ifndef NDEBUG
	std::cout << "call: insert_equal ";
#endif
	link_type current = header;
	// 使用update来保存每层中最后一个满足其key小于待插入节点的key的节点（即前驱节点)
	// update大小设置为max_level+1以确保有足够的空间来存放每层满足条件的节点
	link_type update[max_level+1];
	bzero(update, sizeof(link_type)*(max_level+1));

	// 从跳表最高层开始查找
	for (int i = top_level; i >= 0; --i) {
		// 若当前节点的后继不为空且后继的key小于待插入的节点的key
		// 表明需要在当前层继续前进，继续while循环
		while (current->forward[i] && key_compare(key(current->forward[i]), KeyOfValue()(val)))
			current = current->forward[i];
		// 若当前节点的后继为空或后继节点的key大于等于目标节点的key
		// 则current此时即为待插入节点的前一个位置（前驱节点），将其保存到update中
		update[i] = current;
	}
	return __insert(update, val);
}
*/

// 创建一个节点并设置相应的值以及前驱和后继，返回指向新节点的迭代器
template <typename Key, typename Value, typename KeyOfValue, typename Compare>
typename skiplist<Key, Value, KeyOfValue, Compare>::iterator
skiplist<Key, Value, KeyOfValue, Compare>::__insert(link_type* update, const value_type &val) {
#ifndef NDEBUG
	std::cout << "=> skiplist.__insert..." << std::endl;
#endif
	// 为待插入的节点生成随机层数，层索引从0开始，所以实际层数为level+1
	size_type level = random_level();

	// 若待插入节点的level大于当前跳表中的最高层级top_level（不是max_level）
	// 则表明在层级为[top_level+1, level]范围内，待插入节点的前驱必为header
	if (level > top_level) {
		for (size_type i = top_level+1; i <= level; ++i)
			update[i] = header;
		// 更新跳表的最高层级
		top_level = level;
	}

	// 创建待插入的新节点
	link_type node = create_node(val, level);

	// 设置新节点在跳表中的前驱和后继
	for (size_type i = 0; i <= level; ++i) {
		// 将新节点的后继设置为事先所保存的前驱节点的后继
		node->forward[i] = update[i]->forward[i];
		// 更新事先所保存的前驱节点的后继为当前节点
		update[i]->forward[i] = node;
	}

	// 更新跳表中的节点总数
	++node_count;
#ifndef NDEBUG
	std::cout << std::setw(6) << " " << "** successfully inserted: " << val << ":" << KeyOfValue()(val) << std::endl;
#endif

	// 返回新节点的位置
	return node;
}

// 在跳表中根据key查找节点，key存在则返回指向该节点的迭代器，否则返回尾迭代器
template <typename Key, typename Value, typename KeyOfValue, typename Compare>
typename skiplist<Key, Value, KeyOfValue, Compare>::iterator
skiplist<Key, Value, KeyOfValue, Compare>::find(const key_type &k) {
#ifndef NDEBUG
	std::cout << "call: skiplist.find..." << std::endl;
#endif
	link_type current = header;
	// 从跳表最高层开始查找
	for (int i = top_level; i >= 0; --i) {
		// 若当前节点的后继不为空且后继的key小于目标key
		// 表明需要在当前层继续前进，继续while循环
		while (current->forward[i] && key_compare(key(current->forward[i]), k))
			current = current->forward[i];
		// 若当前节点的后继为空或后继节点的key大于等于目标的key
		// 表明需要向下降一层级，即结束while循环，开启下一次for循环
	}
	// 当for循环结束时，表明已经查找到了第0层级
	// 且此时的current节点必定是跳表中满足key小于目标key的所有节点中，key最大的那个节点
	// 若key存在，则current的后继即为所要查找的目标节点
	current = current->forward[0];

	if (current && !key_compare(k, key(current))) {
#ifndef NDEBUG
		std::cout << std::setw(6) << " " << "** successfully found: " << k << ":" << value(current) << std::endl;
#endif
		return current;
	}
#ifndef NDEBUG
	std::cout << std::setw(6) << " " << "** not found key: " << k << std::endl;
#endif
	return end();
}

// 清空跳表，释放跳表中除header外的所有节点
template <typename Key, typename Value, typename KeyOfValue, typename Compare>
void skiplist<Key, Value, KeyOfValue, Compare>::clear() {
#ifndef NDEBUG
	std::cout << "call: skiplist.clear..." << std::endl;
#endif
	// 从第0层的头节点的后继开始
	link_type node = header->forward[0];
#ifndef NDEBUG
	std::cout << std::setw(6) << " " << "** skiplist node count: " << std::to_string(size()) << std::endl;
	if (node) std::cout << std::setw(6) << " " << "** successfully deleted: ";
	else return;
#endif
	while (node) {
		link_type tmp = node;
		node = node->forward[0];
#ifndef NDEBUG
		std::cout << value(tmp) << ":" << key(tmp);
		if (node) std::cout << " => ";
#endif
		// 销毁节点
		destroy_node(tmp);
	}
	std::cout << std::endl;

	// 重新初始化header的forward数组
	bzero(header->forward, sizeof(link_type)*(top_level+1));
	// 重置最高层级和节点数量
	top_level = 0;
	node_count = 0;
}

#ifndef NDEBUG
// 打印跳表中的所有节点（仅限于调试）
template <typename Key, typename Value, typename KeyOfValue, typename Compare>
void skiplist<Key, Value, KeyOfValue, Compare>::display() const {
	std::cout << "call: skiplist.display..." << std::endl;
	std::cout << std::setw(6) << " " << "** skiplist node count: " << std::to_string(size()) << std::endl;
	if (empty()) return;
	// 从最高层开始打印
	for (int i = top_level; i >= 0; --i) {
		// node为正在打印的层
		link_type node = header->forward[i];
		std::cout << std::setw(6) << " " << "** level " << i << ": ";
		// tmp为第0层，存放实际要打印的值
		link_type tmp = header->forward[0];
		// 为了方便显示，所以手动指定了输出补白
		// 因此跳表中的key和value最好是单个字符
		while (node) {
			while (tmp && node != tmp) {
				// 若tmp的value不存在于当前层，则输出空白
				std::cout << std::setw(7) << " ";
				tmp = tmp->forward[0];
			}
			std::cout << std::setw(4) << " " << value(node) << ":" << key(node);
			tmp = tmp->forward[0];
			node = node->forward[i];
		}
		std::cout << std::endl;
	}
}
#endif

#endif
