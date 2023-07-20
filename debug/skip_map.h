#ifndef SKIP_MAP_H
#define SKIP_MAP_H

#include <functional>
#include "skiplist.h"

// 前置声明，在skip_map中声明友元需要
template <typename Key, typename T, typename Compare>
class skip_map;

template <typename Key, typename T, typename Compare>
bool operator==(const skip_map<Key, T, Compare> &lhs, const skip_map<Key, T, Compare> &rhs);

// template <typename Key, typename T, typename Compare>
// bool operator<(const skip_map<Key, T, Compare> &lhs, const skip_map<Key, T, Compare> &rhs);

// skip_map类
template <typename Key, typename T, typename Compare = std::less<Key>>
class skip_map {
	public:
		// 键值类型
		typedef Key key_type;
		// 数据（实值）类型
		typedef T data_type;
		typedef T mapped_type;
		// 元素类型（键值/实值）
		typedef std::pair<const Key, T> value_type;
		// 键值比较函数
		typedef Compare key_compare;

		// 定义嵌套类，只重载调用运算符，通过比较键值来判定元素的大小关系
		class value_compare : public std::binary_function<value_type, value_type, bool> {
			friend class skip_map<Key, T, Compare>;
			protected:
				Compare comp;
				value_compare(Compare c) : comp(c) {}
			public:
				bool operator()(const value_type &x, const value_type &y) const { return comp(x.first, y.first); }
		};

	private:
		// 选择函数，接受一个pair，并返回其first成员
		// 作为跳表的KeyOfValue使用，因为map的key是元素的first成员
		template <typename Pair>
		struct select1st : public std::unary_function<Pair, typename Pair::first_type> {
			const typename Pair::first_type& operator() (const Pair &x) const { return x.first; }
		};
		typedef skiplist<key_type, value_type, select1st<value_type>, key_compare> rep_type;
		rep_type rep;
	
	public:
		typedef typename rep_type::pointer pointer;
		typedef typename rep_type::const_pointer const_pointer;
		typedef typename rep_type::reference reference;
		typedef typename rep_type::const_reference const_reference;
		// 允许通过迭代器来修改元素的实值，因此未定义为const_iterator
		typedef typename rep_type::iterator iterator;
		typedef typename rep_type::const_iterator const_iterator;
		typedef typename rep_type::size_type size_type;
		typedef typename rep_type::difference_type difference_type;

		// 构造函数，默认的层数上限为18
		skip_map() : rep(18, Compare()) {}
		explicit skip_map(size_type max_level) : rep(max_level, Compare()) {}
		explicit skip_map(const Compare &comp) : rep(18, comp) {}
		skip_map(size_type max_level, const Compare &comp) : rep(max_level, comp) {}

		// 允许从一对迭代器[first, last)指示的范围来构造skip_map
		template <typename InputIterator>
		skip_map(InputIterator first, InputIterator last)
			: rep(18, Compare()) { rep.insert_unique(first, last); }
		template <typename InputIterator>
		skip_map(InputIterator first, InputIterator last, size_type max_level)
			: rep(max_level, Compare()) { rep.insert_unique(first, last); }
		template <typename InputIterator>
		skip_map(InputIterator first, InputIterator last, const Compare &comp)
			: rep(18, comp) { rep.insert_unique(first, last); }
		template <typename InputIterator>
		skip_map(InputIterator first, InputIterator last, size_type max_level, const Compare &comp)
			: rep(max_level, comp) { rep.insert_unique(first, last); }

		// 拷贝构造
		skip_map(const skip_map<Key, T, Compare> &rhs) : rep(rhs.rep) {}
		// 赋值运算符
		skip_map<Key, T, Compare>& operator=(const skip_map<Key, T, Compare> &rhs) { rep = rhs.rep; return *this; }
		// 交换操作
		void swap(skip_map<Key, T, Compare> &rhs) { rep.swap(rhs.rep); }

		// 转调用跳表的接口
		key_compare key_comp() const { return rep.key_comp(); }
		value_compare value_comp() const { return value_compare(rep.key_comp()); }
		iterator begin() const { return rep.begin(); }
		const_iterator cbegin() const { return rep.cbegin(); }
		iterator end() const { return rep.end(); }
		const_iterator cend() const { return rep.cend(); }
		bool empty() const { return rep.empty(); }
		size_type size() const { return rep.size(); }
		size_type max_size() const { return rep.max_size(); }

		// 插入操作
		std::pair<iterator, bool> insert(const value_type &val) { return rep.insert_unique(val); }

		template <typename InputIterator>
		void insert(InputIterator first, InputIterator last) { rep.insert_unique(first, last); }

		// 删除操作
		void erase(const key_type &k) { rep.erase(k); }

		void erase(iterator first, iterator last) { rep.erase(first, last); }

		// 清空操作
		void clear() { rep.clear(); }

		// 查找操作
		iterator find(const key_type &k) const { return rep.find(k); }

		// 重载下标运算符
		// 先用value_type(k, T())构造一个临时的对象，再通过insert将其插入底层跳表
		// 若插入成功（元素key不存在于跳表），则新节点的实值为T的默认值并返回被插入节点的迭代器和true
		// 若插入失败（元素key已存在于跳表），则不修改key相同的节点的实值并返回该节点的迭代器和false
		// 因此对insert返回的pair对象中的first成员解引用可得到对含有目标key的节点的引用
		// 那么通过second成员，即可获得对该节点实值的引用，将其返回则可实现通过下标运算符设置元素实值
		T& operator[](const key_type &k) { return (*((insert(value_type(k, T()))).first)).second; }

		// 重载关系运算符的友元声明
		friend bool operator==<Key, T, Compare>(const skip_map<Key, T, Compare> &lhs, const skip_map<Key, T, Compare> &rhs);
		// friend bool operator< <Key, T, Compare>(const skip_map<Key, T, Compare> &lhs, const skip_map<Key, T, Compare> &rhs);
};

// 定义重载的相等性判断运算符
template <typename Key, typename T, typename Compare>
inline bool operator==(const skip_map<Key, T, Compare> &lhs, const skip_map<Key, T, Compare> &rhs) {
	return lhs.rep == rhs.rep;
}

#endif
