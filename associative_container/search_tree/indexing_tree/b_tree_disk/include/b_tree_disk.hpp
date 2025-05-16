#ifndef B_TREE_DISK_HPP
#define B_TREE_DISK_HPP

#include <concepts>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <stack>
#include <utility>
#include <vector>

template<typename compare, typename tkey>
concept compator = requires(const compare c, const tkey& lhs, const tkey& rhs) {
	{ c(lhs, rhs) } -> std::same_as<bool>;
} && std::copyable<compare> && std::default_initializable<compare>;

template<typename f_iter, typename tkey, typename tval>
concept input_iterator_for_pair = std::input_iterator<f_iter> && std::same_as<typename std::iterator_traits<f_iter>::value_type, std::pair<tkey, tval>>;

template<typename T>
concept serializable = requires(const T t, std::fstream& s) {
	{ t.serialize(s) };
	{ T::deserialize(s) } -> std::same_as<T>;
	{ t.serialize_size() } -> std::same_as<size_t>;
} && std::copyable<T>;


template<serializable tkey, serializable tvalue, compator<tkey> compare = std::less<tkey>, std::size_t t = 2>
class B_tree_disk final : private compare {
public:
	using tree_data_type = std::pair<tkey, tvalue>;
	using tree_data_type_const = std::pair<tkey, tvalue>;

private:
	static constexpr const size_t minimum_keys_in_node = t - 1;
	static constexpr const size_t maximum_keys_in_node = 2 * t - 1;

	// region comparators declaration

	inline bool compare_keys(const tkey& lhs, const tkey& rhs) const;

	inline bool compare_pairs(const tree_data_type& lhs, const tree_data_type& rhs) const;

	// endregion comparators declaration

public:
	struct btree_disk_node {
		size_t size;// кол-во заполненных ячеек
		bool _is_leaf;
		size_t position_in_disk;
		std::vector<tree_data_type> keys;
		std::vector<size_t> pointers;
		void serialize(std::fstream& stream, std::fstream& stream_for_data) const;

		static btree_disk_node deserialize(std::fstream& stream, std::fstream& stream_for_data);

		explicit btree_disk_node(bool is_leaf);
		btree_disk_node();
	};

private:
	friend btree_disk_node;

	//logger* _logger;

	std::fstream _file_for_tree;

	std::fstream _file_for_key_value;

	//    btree_disk_node _root;

public:
	size_t _position_root;//

private:
	btree_disk_node _current_node;

	//logger* get_logger() const noexcept override;


public:
	static size_t _count_of_node;//только растет

	// region constructors declaration

	explicit B_tree_disk(const std::string& file_path, const compare& cmp = compare(), void* logger = nullptr);


	// endregion constructors declaration

	// region five declaration

	B_tree_disk(B_tree_disk&& other) noexcept = default;
	B_tree_disk& operator=(B_tree_disk&& other) noexcept = default;

	B_tree_disk(const B_tree_disk& other) = delete;
	B_tree_disk& operator=(const B_tree_disk& other) = delete;

	~B_tree_disk() noexcept = default;

	// endregion five declaration

	// region iterators declaration

	class btree_disk_const_iterator {
		std::stack<std::pair<size_t, size_t>> _path;
		size_t _index;
		B_tree_disk<tkey, tvalue, compare, t>& _tree;

	public:
		using value_type = tree_data_type_const;
		using reference = value_type&;
		using pointer = value_type*;
		using iterator_category = std::forward_iterator_tag;
		using difference_type = ptrdiff_t;

		using self = btree_disk_const_iterator;

		friend class B_tree_disk;

		value_type operator*() noexcept;

		self& operator++();
		self operator++(int);

		self& operator--();
		self operator--(int);

		bool operator==(self& other) noexcept;
		bool operator!=(self& other) noexcept;

		explicit btree_disk_const_iterator(B_tree_disk<tkey, tvalue, compare, t>& tree, const std::stack<std::pair<size_t, size_t>>& path = std::stack<std::pair<size_t, size_t>>(), size_t index = 0);
	};

	friend class btree_disk_const_iterator;

	std::optional<tvalue> at(const tkey&);//либо пустое, либо tvalue//std::nullopt

	btree_disk_const_iterator begin();
	btree_disk_const_iterator end();

	//std::vector<tree_data_type_const> find_range(const tkey& lower, const tkey& upper) const;

	std::pair<btree_disk_const_iterator, btree_disk_const_iterator> find_range(const tkey& lower, const tkey& upper, bool include_lower = true, bool include_upper = false);

	/*
     * Does nothing if key exists
     * Second return value is true, when inserted
     */
	bool insert(const tree_data_type& data);

	/*
     * Updates value if key exists
     */
	bool update(const tree_data_type& data);

	/*
     * Return true if deleted
     */
	bool erase(const tkey& key);

	bool is_valid() const noexcept;


	std::pair<std::stack<std::pair<size_t, size_t>>, std::pair<size_t, bool>> find_path(const tkey& key);

public:
	btree_disk_node disk_read(size_t position);


	void check_tree(size_t pos, size_t depth);

	void disk_write(btree_disk_node& node);

private:
	std::pair<size_t, bool> find_index(const tkey& key, btree_disk_node& node) const noexcept;

	void insert_array(btree_disk_node& node, size_t right_node, const tree_data_type& data, size_t index) noexcept;

	void split_node(std::stack<std::pair<size_t, size_t>>& path);

	btree_disk_node remove_array(btree_disk_node& node, size_t index, bool remove_left_ptr = true) noexcept;

	void rebalance_node(std::stack<std::pair<size_t, size_t>>& path, btree_disk_node& node, size_t& index);

	void print_root_position() noexcept;
};

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
bool B_tree_disk<tkey, tvalue, compare, t>::is_valid() const noexcept {
	try {
		if (_count_of_node == 0 || _position_root == 0) return true;
		check_tree(_position_root, 0);
		return true;
	} catch (...) {
		return false;
	}
}


template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
bool B_tree_disk<tkey, tvalue, compare, t>::erase(const tkey& key) {
	auto [path, info] = find_path(key);
	size_t index = info.first;
	bool found = info.second;
	if (!found) return false;

	auto [node_pos, node_idx] = path.top();
	auto node = disk_read(node_pos);

	if (node._is_leaf) {
		auto new_node = remove_array(node, index, false);
		disk_write(new_node);
	} else {
		// Internal node: replace with predecessor
		size_t child_pos = node.pointers[index];
		auto pred = disk_read(child_pos);
		while (!pred._is_leaf) {
			child_pos = pred.pointers[pred.size];
			pred = disk_read(child_pos);
		}
		auto pred_pair = pred.keys[pred.size - 1];
		node.keys[index] = pred_pair;
		disk_write(node);
		pred = remove_array(pred, pred.size - 1, false);
		disk_write(pred);
		path.push({child_pos, pred.size});
	}

	while (!path.empty()) {
		auto [pos, pos_index] = path.top();
		path.pop();
		auto curr = disk_read(pos);
		rebalance_node(path, curr, pos_index);
		disk_write(curr);
	}

	auto root = disk_read(_position_root);
	if (!root._is_leaf && root.size == 0) {
		_position_root = root.pointers[0];
	}

	disk_write(root);
	return true;
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
void B_tree_disk<tkey, tvalue, compare, t>::print_root_position() noexcept {
	if (!_file_for_tree.is_open()) return;

	_file_for_tree.seekg(sizeof(size_t), std::ios::beg);
	_file_for_tree.write(reinterpret_cast<const char*>(&_position_root), sizeof(size_t));
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
void B_tree_disk<tkey, tvalue, compare, t>::rebalance_node(std::stack<std::pair<size_t, size_t>>& path, btree_disk_node& node, size_t& index) {
	constexpr size_t min_keys = minimum_keys_in_node;
	if (node._is_leaf && node.size >= min_keys) return;
	if (!node._is_leaf && node.size >= min_keys) return;

	if (path.empty()) return;// root case handled separately

	// Parent info
	auto [parent_pos, parent_index] = path.top();
	auto parent = disk_read(parent_pos);

	// Try borrow from left sibling
	if (parent_index > 0) {
		size_t left_pos = parent.pointers[parent_index - 1];
		auto left = disk_read(left_pos);
		if (left.size > min_keys) {
			// Shift keys right in node
			node.keys.insert(node.keys.begin(), parent.keys[parent_index - 1]);
			if (!node._is_leaf) node.pointers.insert(node.pointers.begin(), left.pointers.back());

			parent.keys[parent_index - 1] = left.keys.back();
			left.keys.pop_back();
			if (!left._is_leaf) left.pointers.pop_back();

			--left.size;
			++node.size;
			disk_write(left);
			disk_write(parent);
			return;
		}
	}
	// Try borrow from right sibling
	if (parent_index < parent.pointers.size() - 1) {
		size_t right_pos = parent.pointers[parent_index + 1];
		auto right = disk_read(right_pos);
		if (right.size > min_keys) {
			node.keys.push_back(parent.keys[parent_index]);
			if (!node._is_leaf) node.pointers.push_back(right.pointers.front());

			parent.keys[parent_index] = right.keys.front();
			right.keys.erase(right.keys.begin());
			if (!right._is_leaf) right.pointers.erase(right.pointers.begin());

			--right.size;
			++node.size;
			disk_write(right);
			disk_write(parent);
			return;
		}
	}
	// Merge with sibling (no borrow)
	if (parent_index > 0) {
		// merge with left
		size_t left_pos = parent.pointers[parent_index - 1];
		auto left = disk_read(left_pos);

		// bring down parent key
		left.keys.push_back(parent.keys[parent_index - 1]);
		for (auto& k: node.keys) left.keys.push_back(k);
		if (!node._is_leaf) {
			for (auto& p: node.pointers) left.pointers.push_back(p);
		}

		left.size = left.keys.size();
		disk_write(left);

		// remove parent key and pointer
		parent.keys.erase(parent.keys.begin() + parent_index - 1);
		parent.pointers.erase(parent.pointers.begin() + parent_index);
	} else {
		// merge with right
		size_t right_pos = parent.pointers[parent_index + 1];
		auto right = disk_read(right_pos);
		node.keys.push_back(parent.keys[parent_index]);
		for (auto& k: right.keys) node.keys.push_back(k);
		if (!node._is_leaf) {
			for (auto& p: right.pointers) node.pointers.push_back(p);
		}

		node.size = node.keys.size();
		disk_write(node);
		parent.keys.erase(parent.keys.begin() + parent_index);
		parent.pointers.erase(parent.pointers.begin() + parent_index + 1);
	}
	--parent.size;
	disk_write(parent);
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_node B_tree_disk<tkey, tvalue, compare, t>::remove_array(btree_disk_node& node, size_t index, bool remove_left_ptr) noexcept {
	node.keys.erase(node.keys.begin() + index);
	if (remove_left_ptr) {
		node.pointers.erase(node.pointers.begin() + index);
	} else {
		node.pointers.erase(node.pointers.begin() + index + 1);
	}
	--node.size;
	return node;
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
bool B_tree_disk<tkey, tvalue, compare, t>::update(const B_tree_disk::tree_data_type& data) {
	auto [path, info] = find_path(data.first);
	size_t idx = info.first;
	bool found = info.second;
	if (!found) return false;
	auto [node_pos, node_index] = path.top();
	auto node = disk_read(node_pos);
	node.keys[idx].second = data.second;
	disk_write(node);
	return true;
}


template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
bool B_tree_disk<tkey, tvalue, compare, t>::insert(const B_tree_disk::tree_data_type& data) {
	auto [path, info] = find_path(data.first);
	if (info.second) return false;

	// Insert into leaf
	auto [leaf_pos, leaf_index] = path.top();
	auto leaf = disk_read(leaf_pos);
	insert_array(leaf, 0, data, info.first);
	disk_write(leaf);

	// Split nodes on overflow
	while (true) {
		auto [pos, idx] = path.top();
		auto node = disk_read(pos);
		if (node.size <= maximum_keys_in_node) break;
		split_node(path);
	}

	return true;
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
void B_tree_disk<tkey, tvalue, compare, t>::split_node(std::stack<std::pair<size_t, size_t>>& path) {
	auto [pos, index] = path.top();
	path.pop();
	auto node = disk_read(pos);

	btree_disk_node new_node(node._is_leaf);
	new_node.size = t - 1;
	for (size_t i = 0; i < t - 1; ++i) {
		new_node.keys.push_back(node.keys[t + i]);
	}

	if (!node._is_leaf) {
		for (size_t i = 0; i < t; ++i) {
			new_node.pointers.push_back(node.pointers[t + i]);
		}
	}

	node.keys.resize(t - 1);
	if (!node._is_leaf) {
		node.pointers.resize(t);
	}

	node.size = t - 1;
	new_node.position_in_disk = ++_count_of_node;
	disk_write(node);
	disk_write(new_node);

	auto median = node.keys[t - 1];
	if (path.empty()) {
		// Create new root
		btree_disk_node root_node(false);
		root_node.size = 1;
		root_node.keys.push_back(median);
		root_node.pointers.push_back(node.position_in_disk);
		root_node.pointers.push_back(new_node.position_in_disk);
		root_node.position_in_disk = ++_count_of_node;
		_position_root = root_node.position_in_disk;
		disk_write(root_node);
	} else {
		// Insert into parent
		auto [ppos, pindex] = path.top();
		auto parent = disk_read(ppos);
		insert_array(parent, new_node.position_in_disk, median, index);
		parent.size++;
		disk_write(parent);
	}

	if (!path.empty()) {
		if (path.top().second > index) {
			path.top().second = path.top().second + 1;
		}
	}
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
void B_tree_disk<tkey, tvalue, compare, t>::insert_array(btree_disk_node& node, size_t right_node, const tree_data_type& data, size_t index) noexcept {
	node.keys.insert(node.keys.begin() + index, data);
	if (!node._is_leaf) {
		node.pointers.insert(node.pointers.begin() + index + 1, right_node);
	}
	++node.size;
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
std::pair<std::stack<std::pair<size_t, size_t>>, std::pair<size_t, bool>> B_tree_disk<tkey, tvalue, compare, t>::find_path(const tkey& key) {
	std::stack<std::pair<size_t, size_t>> path;
	size_t current_pos = _position_root;
	if (current_pos == 0) return {path, {0, false}};

	while (true) {
		auto node = disk_read(current_pos);
		auto [idx, found] = find_index(key, node);
		path.push({current_pos, idx});
		if (found) return {path, {idx, true}};
		if (node._is_leaf) return {path, {idx, false}};

		current_pos = node.pointers[idx];
	}
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
std::pair<size_t, bool> B_tree_disk<tkey, tvalue, compare, t>::find_index(const tkey& key, btree_disk_node& node) const noexcept {
	size_t low = 0;
	size_t high = node.size;
	while (low < high) {
		size_t mid = (low + high) / 2;
		if (compare_keys(node.keys[mid].first, key)) {
			low = mid + 1;
		} else {
			high = mid;
		}
	}
	if (low < node.size && !compare_keys(key, node.keys[low].first) && !compare_keys(node.keys[low].first, key)) {
		return {low, true};
	}

	return {low, false};
}


template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
void B_tree_disk<tkey, tvalue, compare, t>::btree_disk_node::serialize(std::fstream& stream, std::fstream& stream_for_data) const {
	// Serialize node header
	stream.write(reinterpret_cast<const char*>(&size), sizeof(size));
	stream.write(reinterpret_cast<const char*>(&_is_leaf), sizeof(_is_leaf));
	stream.write(reinterpret_cast<const char*>(&position_in_disk), sizeof(position_in_disk));

	// Serialize keys and values
	// First write count
	size_t key_count = keys.size();
	stream.write(reinterpret_cast<const char*>(&key_count), sizeof(key_count));
	for (const auto& kv: keys) {
		// Serialize into data file
		kv.first.serialize(stream_for_data);
		kv.second.serialize(stream_for_data);
		size_t offset = static_cast<size_t>(stream_for_data.tellp()) - kv.first.serialize_size() - kv.second.serialize_size();
		stream.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
	}

	// Serialize child pointers
	size_t ptr_count = pointers.size();
	stream.write(reinterpret_cast<const char*>(&ptr_count), sizeof(ptr_count));
	for (size_t ptr: pointers) {
		stream.write(reinterpret_cast<const char*>(&ptr), sizeof(ptr));
	}
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
void B_tree_disk<tkey, tvalue, compare, t>::disk_write(btree_disk_node& node) {
	_file_for_tree.seekp(0, std::ios::end);
	node.position_in_disk = static_cast<size_t>(_file_for_tree.tellp());
	node.serialize(_file_for_tree, _file_for_key_value);
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_node B_tree_disk<tkey, tvalue, compare, t>::btree_disk_node::deserialize(std::fstream& stream, std::fstream& stream_for_data) {
	btree_disk_node node;
	stream.read(reinterpret_cast<char*>(&node.size), sizeof(node.size));
	stream.read(reinterpret_cast<char*>(&node._is_leaf), sizeof(node._is_leaf));
	stream.read(reinterpret_cast<char*>(&node.position_in_disk), sizeof(node.position_in_disk));

	// Read keys and their offsets
	size_t key_count;
	stream.read(reinterpret_cast<char*>(&key_count), sizeof(key_count));
	node.keys.clear();
	for (size_t i = 0; i < key_count; ++i) {
		size_t offset;
		stream.read(reinterpret_cast<char*>(&offset), sizeof(offset));
		stream_for_data.seekg(offset, std::ios::beg);
		tkey k = tkey::deserialize(stream_for_data);
		tvalue v = tvalue::deserialize(stream_for_data);
		node.keys.emplace_back(std::move(k), std::move(v));
	}

	// Read child pointers
	size_t ptr_count;
	stream.read(reinterpret_cast<char*>(&ptr_count), sizeof(ptr_count));
	node.pointers.clear();
	for (size_t i = 0; i < ptr_count; ++i) {
		size_t ptr;
		stream.read(reinterpret_cast<char*>(&ptr), sizeof(ptr));
		node.pointers.push_back(ptr);
	}

	return node;
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_node B_tree_disk<tkey, tvalue, compare, t>::disk_read(size_t node_position) {
	_file_for_tree.seekg(node_position, std::ios::beg);
	return btree_disk_node::deserialize(_file_for_tree, _file_for_key_value);
}


template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
B_tree_disk<tkey, tvalue, compare, t>::btree_disk_node::btree_disk_node(bool is_leaf) : _is_leaf(is_leaf), size(0), position_in_disk(_count_of_node) {
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
B_tree_disk<tkey, tvalue, compare, t>::btree_disk_node::btree_disk_node() : _is_leaf(true), size(0), position_in_disk(_count_of_node) {
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
bool B_tree_disk<tkey, tvalue, compare, t>::compare_pairs(const B_tree_disk::tree_data_type& lhs,
														  const B_tree_disk::tree_data_type& rhs) const {
	return compare(lhs.first, rhs.first);
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
bool B_tree_disk<tkey, tvalue, compare, t>::compare_keys(const tkey& lhs, const tkey& rhs) const {
	return compare(lhs, rhs);
}


template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
size_t B_tree_disk<tkey, tvalue, compare, t>::_count_of_node = 0;

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
B_tree_disk<tkey, tvalue, compare, t>::B_tree_disk(const std::string& file_path, const compare& cmp, void* logger) : compare(cmp) {
	std::filesystem::path base(file_path);
	auto idx_path = base;
	idx_path += ".tree";
	auto data_path = base;
	data_path += ".data";
	_file_for_tree.open(idx_path, std::ios::in | std::ios::out | std::ios::binary);
	_file_for_key_value.open(data_path, std::ios::in | std::ios::out | std::ios::binary);
	if (!_file_for_tree.is_open() || !_file_for_key_value.is_open()) {
		_file_for_tree.open(idx_path, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
		_file_for_key_value.open(data_path, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
		if (!_file_for_tree || !_file_for_key_value) {
			!_file_for_tree ? _file_for_key_value.close() : _file_for_tree.close();
			throw std::runtime_error("Failed to open database files");
		}

		btree_disk_node root(true);
		root.position_in_disk = ++_count_of_node;
		_position_root = root.position_in_disk;
		disk_write(root);
	} else {
		_position_root = 0;
		_file_for_tree.seekg(0, std::ios::end);
		size_t file_size = static_cast<size_t>(_file_for_tree.tellg());
		if (file_size > 0) {
			_file_for_tree.seekg(0, std::ios::beg);
			_file_for_tree.read(reinterpret_cast<char*>(&_count_of_node), sizeof(_count_of_node));
			_file_for_tree.read(reinterpret_cast<char*>(&_position_root), sizeof(_position_root));
		}
	}
}


template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
void B_tree_disk<tkey, tvalue, compare, t>::check_tree(size_t pos, size_t depth) {
	auto node = disk_read(pos);
	size_t min_keys = (pos == _position_root ? 1 : minimum_keys_in_node);
	size_t max_keys = maximum_keys_in_node;
	assert(node.size >= min_keys && node.size <= max_keys);

	// Check sorted keys
	for (size_t i = 1; i < node.size; ++i) {
		assert(compare_keys(node.keys[i - 1].first, node.keys[i].first));
	}

	if (!node._is_leaf) {
		// pointers should be size+1
		assert(node.pointers.size() == node.size + 1);
		for (size_t i = 0; i < node.pointers.size(); ++i) {
			check_tree(node.pointers[i], depth + 1);
		}
	}
}


template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::btree_disk_const_iterator(B_tree_disk<tkey, tvalue, compare, t>& tree, const std::stack<std::pair<size_t, size_t>>& path, size_t index) : _tree(tree), _path(path), _index(index) {}


template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator B_tree_disk<tkey, tvalue, compare, t>::begin() {
	std::stack<std::pair<size_t, size_t>> path;
	size_t pos = _position_root;
	if (pos == 0) return end();
	while (true) {
		auto node = disk_read(pos);
		path.push({pos, 0});
		if (node._is_leaf) break;
		pos = node.pointers[0];
	}
	return btree_disk_const_iterator(*this, path);
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator B_tree_disk<tkey, tvalue, compare, t>::end() {
	return btree_disk_const_iterator(*this);
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::self& B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::operator++() {
	while (!_path.empty()) {
		auto& [node_pos, idx] = _path.top();
		auto node = _tree.disk_read(node_pos);
		if (idx + 1 < node.size) {
			_path.top().second = idx + 1;
			// descend to leftmost of this subtree
			if (!node._is_leaf) {
				size_t pos = node.pointers[idx + 1];
				while (true) {
					auto cnode = _tree.disk_read(pos);
					_path.push({pos, 0});
					if (cnode._is_leaf) break;
					pos = cnode.pointers[0];
				}
			}
			return *this;
		}
		_path.pop();
	}
	return *this;
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::self B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::operator++(int) {
	self tmp = *this;
	++(*this);
	return tmp;
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::self& B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::operator--() {
	if (_path.empty()) return *this;
	auto [node_pos, idx] = _path.top();
	auto node = _tree.disk_read(node_pos);
	if (idx > 0) {
		_path.top().second = idx - 1;
		// descend to rightmost of this subtree
		if (!node._is_leaf) {
			size_t pos = node.pointers[idx];
			while (true) {
				auto cnode = _tree.disk_read(pos);
				size_t i = cnode.size - 1;
				_path.push({pos, i});
				if (cnode._is_leaf) break;
				pos = cnode.pointers[i + 1];
			}
		}
	} else {
		// ascend until we can move left
		_path.pop();
		while (!_path.empty()) {
			auto& [p_pos, p_idx] = _path.top();
			if (p_idx > 0) break;
			_path.pop();
		}
		if (_path.empty()) return *this;
		auto& [p_pos, p_idx] = _path.top();
		_path.top().second = p_idx - 1;
	}
	return *this;
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::self B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::operator--(int) {
	self tmp = *this;
	--(*this);
	return tmp;
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
bool B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::operator==(B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::self& other) noexcept {
	if (_path.size() != other._path.size()) return false;
	return _path == other._path;
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
bool B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::operator!=(B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::self& other) noexcept {
	if (_path.size() == other._path.size()) return false;
	return _path != other._path;
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::value_type B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::operator*() noexcept {
	auto [node_pos, idx] = _path.top();
	auto node = _tree.disk_read(node_pos);
	return node.keys[idx];
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
std::optional<tvalue> B_tree_disk<tkey, tvalue, compare, t>::at(const tkey& key) {
	auto [path, index_info] = find_path(key);
	auto [node_pos, found] = index_info;

	if (!found) return std::nullopt;

	const auto& [pos, idx] = path.top();
	auto node = disk_read(pos);
	return node.keys[idx].second;
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
std::pair<typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator,
		  typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator>
B_tree_disk<tkey, tvalue, compare, t>::find_range(const tkey& lower, const tkey& upper, bool include_lower, bool include_upper) {
	auto [lower_path, lower_info] = find_path(lower);
	auto [lower_node, lower_found] = lower_info;

	if (!lower_found || !include_lower) {
		while (!lower_path.empty()) {
			auto& [node_pos, idx] = lower_path.top();
			auto node = disk_read(node_pos);
			while (idx < node.size && compare{}(node.keys[idx].first, lower) <= 0) {
				++idx;
			}

			if (idx < node.size) break;
			lower_path.pop();
		}
	}

	auto [upper_path, upper_info] = find_path(upper);
	auto [upper_node, upper_found] = upper_info;

	if (upper_found && !include_upper) {
		while (!upper_path.empty()) {
			auto& [node_pos, idx] = upper_path.top();
			auto node = disk_read(node_pos);
			while (idx > 0 && compare{}(node.keys[idx].first, upper) >= 0) {
				--idx;
			}

			if (idx >= 0 && compare{}(node.keys[idx].first, upper) < 0) break;
			upper_path.pop();
		}
	} else if (!upper_found) {
		while (!upper_path.empty()) {
			auto& [node_pos, idx] = upper_path.top();
			auto node = disk_read(node_pos);
			while (idx < node.size && compare{}(node.keys[idx].first, upper) <= 0) {
				++idx;
			}

			if (idx < node.size) break;
			upper_path.pop();
		}
	}

	return {btree_disk_const_iterator(*this, lower_path), btree_disk_const_iterator(*this, upper_path)};
}

#endif//B_TREE_DISK_HPP