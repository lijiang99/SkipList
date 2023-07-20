#ifndef SKIP_SET_H
#define SKIP_SET_H

#include <functional>
#include "skiplist.h"

// 前置声明，在skip_set中声明友元需要
template <typename Key, typename Compare>
class skip_set;

template <typename Key, typename Compare>
bool operator==(const skip_set<Key, Compare> &lhs, const skip_set<Key, Compare> &rhs);

// template <typename Key, typename Compare>
// bool operator<(const skip_set<Key, Compare> &lhs, const skip_set<Key, Compare> &rhs);

// skip_set类
template <typename Key, typename Compare = std::less<Key>>
class skip_set {
	public:
		// set的key就是value
		typedef Key key_type;
		typedef Key value_type;
		typedef Compare key_compare;
		typedef Compare value_compare;

	private:
		// 证同函数，任何数值通过此函数后，不会有任何改变
		// 作为跳表的KeyOfValue使用，因为set的value就是key
		template <typename T>
		struct identity : public std::unary_function<T, T> {
			const T& operator()(const T& x) const { return x; }
		};
		// 使用跳表作为set的底层容器
		typedef skiplist<key_type, value_type, identity<value_type>, key_compare> rep_type;
		rep_type rep;

	public:
		// 不允许修改set已有元素，因为set的value就是key
		// 修改已有元素会破坏跳表结构，因此禁止通过迭代器进行写入
		typedef typename rep_type::const_pointer pointer;
		typedef typename rep_type::const_pointer const_pointer;
		typedef typename rep_type::const_reference reference;
		typedef typename rep_type::const_reference const_reference;
		typedef typename rep_type::const_iterator iterator;
		typedef typename rep_type::const_iterator const_iterator;
		typedef typename rep_type::size_type size_type;
		typedef typename rep_type::difference_type difference_type;

		// 构造函数，默认的层数上限为18
		skip_set() : rep(18, Compare()) {}
		explicit skip_set(size_type max_level) : rep(max_level, Compare()) {}
		explicit skip_set(const Compare &comp) : rep(18, comp) {}
		skip_set(size_type max_level, const Compare &comp) : rep(max_level, comp) {}

		// 允许从一对迭代器[first, last)指示的范围来构造skip_set
		template <typename InputIterator>
		skip_set(InputIterator first, InputIterator last)
			: rep(18, Compare()) { rep.insert_unique(first, last); }
		template <typename InputIterator>
		skip_set(InputIterator first, InputIterator last, size_type max_level)
			: rep(max_level, Compare()) { rep.insert_unique(first, last); }
		template <typename InputIterator>
		skip_set(InputIterator first, InputIterator last, const Compare &comp)
			: rep(18, comp) { rep.insert_unique(first, last); }
		template <typename InputIterator>
		skip_set(InputIterator first, InputIterator last, size_type max_level, const Compare &comp)
			: rep(max_level, comp) { rep.insert_unique(first, last); }

		// 拷贝构造
		skip_set(const skip_set<Key, Compare> &rhs) : rep(rhs.rep) {}
		// 赋值运算符
		skip_set<Key, Compare>& operator=(const skip_set<Key, Compare> &rhs) { rep = rhs.rep; return *this; }
		// 交换操作
		void swap(skip_set<Key, Compare> &rhs) { rep.swap(rhs.rep); }

		// 转调用跳表的接口
		key_compare key_comp() const { return rep.key_comp(); }
		value_compare value_comp() const { return rep.key_comp(); }
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

		// 重载关系运算符的友元声明
		friend bool operator==<Key, Compare>(const skip_set<Key, Compare> &lhs, const skip_set<Key, Compare> &rhs);
		// friend bool operator< <Key, Compare>(const skip_set<Key, Compare> &lhs, const skip_set<Key, Compare> &rhs);
};

// 定义重载的相等性判断运算符
template <typename Key, typename Compare>
inline bool operator==(const skip_set<Key, Compare> &lhs, const skip_set<Key, Compare> &rhs) {
	return lhs.rep == rhs.rep;
}

#endif
