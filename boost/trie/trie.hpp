#ifndef BOOST_TRIE_HPP
#define BOOST_TRIE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif


#include <map>
#include <iterator>
#include <utility>
#include <cstdio>
#include <memory>
#include <iostream>

namespace boost { namespace tries {

namespace detail {

template <typename Key, typename Value,
		 class Compare/*,
		 // only one value, so alloc maybe unnecessary
		 class Alloc = alloc*/>
struct trie_node {
//protected:
	typedef Key key_type;
	typedef key_type* key_ptr;
	typedef Value value_type;
	typedef value_type* value_ptr;
	typedef trie_node<key_type, value_type, Compare> node_type;
	typedef node_type* node_ptr;
	// maybe the pointer container of children could be defined by user?!
	typedef std::map<key_type, node_ptr, Compare> children_type;

	typedef typename children_type::iterator child_iter;

	children_type child;

//public:
	value_ptr value; // use unique_ptr here?
	node_ptr parent;
	key_type key_elem;
	//child_iter child_iter_of_parent;
	size_t node_count;
	size_t value_count;



	trie_node() : value(0), parent(0), node_count(0), value_count(0) 
	{
		child.clear();
	}

	~trie_node()
	{
		child.clear();
	}

};

template <typename Key, typename Value, class Compare>
struct trie_iterator
{
	//typedef std::bidirectional_iterator_tag iterator_category;
	typedef Key key_type;
	typedef Value value_type;
	typedef value_type& ref;
	typedef value_type* ptr;
	typedef trie_iterator<Key, Value, Compare> iterator;
	typedef trie_node<Key, Value, Compare> node_type;
	typedef iterator self;
	typedef node_type* node_ptr;

	node_ptr node;

	trie_iterator() : node(0)
	{
	}

	trie_iterator(node_ptr x) : node(x)
	{
	}

	trie_iterator(const iterator &it): node(it.node)
	{
	}

	ref operator*() const 
	{
		return *node->value;
	}

	ptr operator->() const
	{
		return &(operator*()); 
	}

	bool operator==(const trie_iterator& other)
	{
		return node == other.node;
	}

	bool operator!=(const trie_iterator& other)
	{
		return node != other.node;
	}

	// handle the increment of begin()
	void increment()
	{
		node_ptr cur = node;
		if (!cur->child.empty())
		{ // go down to the first node with a value in it, and there always be at least one
			do {
				cur = cur->child.begin()->second;
			} while (cur->value == NULL);
			node = cur;
		} else {
			// go up till there is a sibling next to cur
			// the algorithm here is not so efficient
			while (cur->parent != NULL)
			{
				node_ptr p = cur->parent;
				typename node_type::child_iter ci = p->child.find(cur->key_elem);
				++ci;
				if (ci != p->child.end())
				{
					cur = ci->second;
					while (cur->value == NULL) {
						cur = cur->child.begin()->second;
					}
					break;
				}
				cur = p;
			}
			node = cur;
		}
	}

	// handle the decrement of end()
	void decrement()
	{
		node_ptr cur = node;
		if (!cur->child.empty())
		{ // go down to the first node with a value in it, and there always be at least one
			do {
				cur = cur->child.rbegin()->second;
			} while (cur->value == NULL);
			node = cur;
		} else {
			// go up till there is a sibling next to cur
			// the algorithm here is not so efficient
			while (cur->parent != NULL)
			{
				node_ptr p = cur->parent;
				typename node_type::child_iter ci = p->child.find(cur->key_elem);
				if (ci != p->child.begin())
				{
					--ci;
					cur = ci->second;
					while (cur->value == NULL) {
						cur = cur->child.begin()->second;
					}
					node = cur;
				}
				cur = p;
			}
			node = cur;
		}
	}

	self& operator++() 
	{
		increment();
		// increment
		return *this;
	}
	self& operator++(int)
	{
		self tmp = *this;
		increment();
		// increment
		return tmp;
	}

	self& operator--()
	{
		// decrement
		return *this;
	}
	self& operator--(int)
	{
		self tmp = *this;
		// decrement
		return tmp;
	}
}; 

} // namespace detail

template <typename Key, typename Value,
		 class Compare>
class trie {
public:
	typedef Key key_type;
	typedef Value value_type;
	typedef value_type* value_ptr;
	typedef trie<key_type, value_type, Compare> trie_type;
	typedef typename detail::trie_node<key_type, value_type, Compare> node_type;
	typedef key_type * key_ptr;
	typedef node_type * node_ptr;
	typedef std::allocator< node_type > trie_node_allocator;
	typedef std::allocator< value_type > value_allocator;
	typedef size_t size_type;

	trie_node_allocator trie_node_alloc;
	value_allocator value_alloc;

	node_ptr root;
	size_type node_count;
	size_type value_count;

	value_ptr new_value()
	{
		return value_alloc.allocate(1);
	}

	void delete_value(value_ptr p)
	{
		if (p)
		{
			value_alloc.destroy(p);
			value_alloc.deallocate(p, 1);
		}
	}

	node_ptr get_node() 
	{
		return trie_node_alloc.allocate(1);
	}

	node_ptr create_node() 
	{
		node_ptr tmp = get_node();
		new(tmp) node_type();
		return tmp;
	}

	void delete_node(node_ptr p)
	{
		delete_value(p->value);
		if (p)
		{
			trie_node_alloc.destroy(p);
			trie_node_alloc.deallocate(p, 1);
		}
	}

	// need constant time to get leftmost
	node_ptr leftmost() const
	{
		if (empty())
			return root;
		node_ptr cur = root;
		while (cur->value == NULL)
		{
			cur = cur->child.begin()->second;
		}
		return cur;
	}

	node_ptr& rightmost() const
	{
		// to be written
		return root;
	}

public:
	// iterators still unavailable here
	
	trie() : root(create_node()), value_count(0), node_count(0) {
		//root.parent = &root;
	}

	typedef detail::trie_iterator<Key, Value, Compare> iterator;
	typedef std::pair<iterator, bool> pair_iterator_bool;

	iterator begin() const
	{
		return leftmost();
	}

	iterator end() const
	{
		return root;
	}

	iterator rbegin() const
	{
		return rightmost();
	}

	iterator rend() const
	{
		return root;
	}

	template<typename Iter>
	iterator __insert(node_ptr cur, Iter first, Iter last,
			const value_type& value)
	{
		for (; first != last; ++first)
		{
			const key_type& cur_key = *first;
			node_ptr new_node = create_node();
			new_node->parent = cur;
			new_node->key_elem = cur_key;
			typename node_type::child_iter ci = cur->child.insert(std::make_pair(cur_key, new_node)).first;
			cur = ci->second;
		}
		cur->value = new_value();
		value_alloc.construct(cur->value, value);
		++value_count;
		return cur;
	}

	template<typename Iter>
	pair_iterator_bool insert_unique(Iter first, Iter last,
			const value_type& value)
	{
		node_ptr cur = root;
		for (; first != last; ++first)
		{
			const key_type& cur_key = *first;
			typename node_type::child_iter ci = cur->child.find(cur_key);
			if (ci == cur->child.end())
			{
				return std::make_pair(__insert(cur, first, last, value), true);
			}
			cur = ci->second;
		}
		// this should be changed to adapt to single value
		if (!cur->value)
		{
			++value_count;
			return std::make_pair(__insert(cur, first, last, value), true);
		}

		return std::make_pair((iterator)cur, false);
	}

	template<typename Container>
	pair_iterator_bool insert_unique(const Container &container, const value_type& value)
	{
		return insert_unique(container.begin(), container.end(), value);
	}

// find 
	template<typename Iter>
	iterator find(Iter first, Iter last)
	{
		node_ptr cur = root;
		for (; first != last; ++first)
		{
			const key_type& cur_key = *first;
			typename node_type::child_iter ci = cur->child.find(cur_key);
			if (ci == cur->child.end())
			{
				return end();
			}
			cur = ci->second;
		}
		// this should be changed to adapt to single value
		return cur;
	}

	template<typename Container>
	iterator find(const Container &container)
	{
		return find(container.begin(), container.end());
	}

	void clear(node_ptr cur)
	{
		typename node_type::child_iter ci = cur->child.begin();
		// end() may take time to calc
		typename node_type::child_iter ci_end = cur->child.end();
		for (; ci != ci_end; ++ci)
		{
			node_ptr c = ci->second;
			clear(c);
			delete_node(c);
		}
	}

	size_t size() const
	{
		return value_count;
	}
	
	bool empty() const {
		return value_count == 0;
	}

	void destroy()
	{
		clear(root);
		delete_node(root);
	}

	~trie()
	{
		destroy();
	}

};

template<typename Key, typename Value,
		class Compare = std::less<Key> >
class trie_map
{
public:
	typedef Key key_type;
	typedef Value value_type;
	typedef trie<key_type, value_type, Compare> trie_type;
	typedef typename trie_type::iterator iterator;
	typedef typename trie_type::pair_iterator_bool pair_iterator_bool;
	typedef size_t size_type;

protected:
	trie_type t;

public:
	trie_map() 
	{
	}

	iterator begin() const 
	{
		return t.begin();
	}

	iterator end() const
	{
		return t.end();
	}

	iterator rbegin() const 
	{
		return t.rbegin();
	}

	iterator rend() const
	{
		return t.rend();
	}

	template<typename Container>
	value_type& operator [] (const Container& container)
	{
		return *(t.insert_unique(container, value_type()).first);
	}

	template<typename Iter>
	pair_iterator_bool insert(Iter first, Iter last, const value_type& value)
	{
		return t.insert_unique(first, last, value);
	}

	template<typename Container>
	pair_iterator_bool insert(const Container& container, const value_type& value)
	{
		return t.insert_unique(container, value);
	}

	template<typename Iter>
	iterator find(Iter first, Iter last)
	{
		return t.find(first, last);
	}

	template<typename Container>
	iterator find(const Container& container)
	{
		return t.find(container);
	}

	size_t size()
	{
		return t.size();
	}

	bool empty()
	{
		return t.empty();
	}

	~trie_map()
	{
		t.destroy();
	}

};

} // tries
} // boost
#endif // BOOST_TRIE_HPP