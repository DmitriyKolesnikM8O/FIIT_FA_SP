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
#include <cassert>
#include <stdexcept>
#include <iostream>
#include <memory>

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


template<typename T>
class disk_allocator {
public:
    using value_type = T;
    
    disk_allocator() = default;
    virtual ~disk_allocator() = default;
    

    virtual T* allocate(size_t n) {
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }
    

    virtual void deallocate(T* p, size_t n) {
        ::operator delete(p);
    }
    

    template<typename... Args>
    void construct(T* p, Args&&... args) {
        new(p) T(std::forward<Args>(args)...);
    }
    

    void destroy(T* p) {
        p->~T();
    }
};

template<serializable tkey, serializable tvalue, compator<tkey> compare = std::less<tkey>, std::size_t t = 2>
class B_tree_disk final : private compare {
public:
    using tree_data_type = std::pair<tkey, tvalue>;
    using tree_data_type_const = std::pair<tkey, tvalue>;
    using allocator_type = disk_allocator<tree_data_type>;


    class exception : public std::exception {
    public:
        explicit exception(const char* message) : _message(message) {}
        const char* what() const noexcept override { return _message; }
    private:
        const char* _message;
    };
    
    class file_error : public exception {
    public:
        explicit file_error(const char* message) : exception(message) {}
    };
    
    class tree_error : public exception {
    public:
        explicit tree_error(const char* message) : exception(message) {}
    };
    
    class node_error : public exception {
    public:
        explicit node_error(const char* message) : exception(message) {}
    };

private:
    static constexpr const size_t minimum_keys_in_node = t - 1;
    static constexpr const size_t maximum_keys_in_node = 2 * t - 1;



    static constexpr const size_t node_header_size = sizeof(size_t) * 3;

    static constexpr const size_t key_pointer_overhead = sizeof(size_t) * 2;

    // region comparators declaration
    inline bool compare_keys(const tkey& lhs, const tkey& rhs) const;
    inline bool compare_pairs(const tree_data_type& lhs, const tree_data_type& rhs) const;
    // endregion comparators declaration

public:
    struct btree_disk_node {
        size_t size;
        bool _is_leaf;
        size_t position_in_disk;
        std::vector<tree_data_type> keys;
        std::vector<size_t> pointers;

        void serialize(std::fstream& stream, std::fstream& stream_for_data) const;
        static btree_disk_node deserialize(std::fstream& stream, std::fstream& stream_for_data);

        explicit btree_disk_node(bool is_leaf);
        btree_disk_node();
        size_t calculate_block_size() const;
    };

private:
    friend btree_disk_node;

    std::fstream _file_for_tree;
    std::fstream _file_for_key_value;
    allocator_type _allocator;

    size_t _node_block_size;

    size_t _position_root;

    btree_disk_node _current_node;

public:
    static size_t _count_of_node;

    // region constructors declaration
    explicit B_tree_disk(
        const std::string& file_path,
        const allocator_type& allocator = allocator_type(),
        const compare& cmp = compare()
    );
    // endregion constructors declaration

    // region five declaration
    B_tree_disk(B_tree_disk&& other) noexcept;
    B_tree_disk& operator=(B_tree_disk&& other) noexcept;

    B_tree_disk(const B_tree_disk& other) = delete;
    B_tree_disk& operator=(const B_tree_disk& other) = delete;

    ~B_tree_disk() noexcept;
    // endregion five declaration

    // region iterators declaration
    class btree_disk_const_iterator {
        std::stack<std::pair<size_t, size_t>> _path;
        size_t _index;
        B_tree_disk<tkey, tvalue, compare, t>& _tree;

    public:
        using value_type = tree_data_type_const;
        using reference = value_type;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag; // Теперь поддерживаем двунаправленные итераторы
        using difference_type = ptrdiff_t;

        using self = btree_disk_const_iterator;

        friend class B_tree_disk;

        value_type operator*() const noexcept;

        self& operator++();
        self operator++(int);

        self& operator--();
        self operator--(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        self& operator=(const self& other) {
            if (this != &other) {

                assert(&_tree == &other._tree);
                _path = other._path;
                _index = other._index;
            }
            return *this;
        }

        self& operator=(self&& other) noexcept {
            if (this != &other) {
                assert(&_tree == &other._tree);
                _path = std::move(other._path);
                _index = other._index;
            }
            return *this;
        }
        btree_disk_const_iterator(const self& other) : _path(other._path), _index(other._index), _tree(other._tree) {
        }


        explicit btree_disk_const_iterator(B_tree_disk<tkey, tvalue, compare, t>& tree, const std::stack<std::pair<size_t, size_t>>& path = std::stack<std::pair<size_t, size_t>>(), size_t index = 0);
    };

    friend class btree_disk_const_iterator;

    std::optional<tvalue> at(const tkey&);

    btree_disk_const_iterator begin();
    btree_disk_const_iterator end();

    std::pair<btree_disk_const_iterator, btree_disk_const_iterator> find_range(const tkey& lower, const tkey& upper, bool include_lower = true, bool include_upper = false);

    bool insert(const tree_data_type& data);
    bool update(const tree_data_type& data);
    bool erase(const tkey& key);
    bool is_valid() const noexcept;


    size_t get_root_position() const { return _position_root; }

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
    void write_metadata();


    size_t calculate_node_position(size_t node_number) const;


    std::pair<btree_disk_node, size_t> find_max_element(size_t node_position);


    std::pair<btree_disk_node, size_t> find_min_element(size_t node_position);
};

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_node B_tree_disk<tkey, tvalue, compare, t>::btree_disk_node::deserialize(std::fstream& stream, std::fstream& stream_for_data) {
    btree_disk_node node;


    if (!stream.good() || !stream_for_data.good()) {
        throw B_tree_disk<tkey, tvalue, compare, t>::file_error("Stream is not good for deserialization");
    }


    if (!stream.read(reinterpret_cast<char*>(&node.size), sizeof(node.size)) ||
        !stream.read(reinterpret_cast<char*>(&node._is_leaf), sizeof(node._is_leaf)) ||
        !stream.read(reinterpret_cast<char*>(&node.position_in_disk), sizeof(node.position_in_disk))) {
        throw B_tree_disk<tkey, tvalue, compare, t>::file_error("Failed to read node header");
    }

    // if (node.size > 1000) {
    //     throw B_tree_disk<tkey, tvalue, compare, t>::node_error("Node size too large, possibly corrupted data");
    // }

    size_t key_count;
    if (!stream.read(reinterpret_cast<char*>(&key_count), sizeof(key_count))) {
        throw B_tree_disk<tkey, tvalue, compare, t>::file_error("Failed to read key count");
    }


    // if (key_count > 1000) {
    //     throw B_tree_disk<tkey, tvalue, compare, t>::node_error("Key count too large, possibly corrupted data");
    // }

    node.keys.clear();
    for (size_t i = 0; i < key_count; ++i) {
        size_t offset;
        if (!stream.read(reinterpret_cast<char*>(&offset), sizeof(offset))) {
            throw B_tree_disk<tkey, tvalue, compare, t>::file_error("Failed to read key offset");
        }


        stream_for_data.seekg(0, std::ios::end);
        size_t file_size = static_cast<size_t>(stream_for_data.tellg());
        if (offset >= file_size) {
            throw B_tree_disk<tkey, tvalue, compare, t>::file_error("Invalid offset in data file");
        }

        stream_for_data.seekg(offset, std::ios::beg);
        if (!stream_for_data.good()) {
            throw B_tree_disk<tkey, tvalue, compare, t>::file_error("Failed to position in data file");
        }

        try {
            tkey k = tkey::deserialize(stream_for_data);
            tvalue v = tvalue::deserialize(stream_for_data);
            node.keys.emplace_back(std::move(k), std::move(v));
        } catch (const std::exception& e) {
            throw B_tree_disk<tkey, tvalue, compare, t>::file_error(std::string("Error deserializing key-value: ").append(e.what()).c_str());
        }
    }

    size_t ptr_count;
    if (!stream.read(reinterpret_cast<char*>(&ptr_count), sizeof(ptr_count))) {
        if (node._is_leaf) {
            node.pointers.clear();
            return node;
        }
        throw B_tree_disk<tkey, tvalue, compare, t>::file_error("Failed to read pointer count");
    }

    if (ptr_count > 1000) {
        throw B_tree_disk<tkey, tvalue, compare, t>::node_error("Pointer count too large, possibly corrupted data");
    }

    node.pointers.clear();
    for (size_t i = 0; i < ptr_count; ++i) {
        size_t ptr;
        if (!stream.read(reinterpret_cast<char*>(&ptr), sizeof(ptr))) {
            throw B_tree_disk<tkey, tvalue, compare, t>::file_error("Failed to read pointer");
        }
        node.pointers.push_back(ptr);
    }

    return node;
}


template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
std::pair<size_t, bool> B_tree_disk<tkey, tvalue, compare, t>::find_index(const tkey& key, btree_disk_node& node) const noexcept {
    if (node.keys.empty()) {
        return {0, false};
    }

    for (size_t i = 0; i < node.size && i < node.keys.size(); ++i) {
        const auto& current_key = node.keys[i].first;

        if (!compare_keys(key, current_key) && !compare_keys(current_key, key)) {
            return {i, true};
        }

        if (compare_keys(key, current_key)) {
            return {i, false};
        }
    }

    return {node.size, false};
}


template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
std::optional<tvalue> B_tree_disk<tkey, tvalue, compare, t>::at(const tkey& key) {
    try {

        if (!_file_for_tree.is_open() || !_file_for_key_value.is_open()) {
            throw file_error("Files not open for reading");
        }


        auto [path, index_info] = find_path(key);
        auto [index, found] = index_info;

        if (!found || path.empty()) {
            return std::nullopt;
        }


        auto [pos, idx] = path.top();
        auto node = disk_read(pos);

        if (idx < node.keys.size()) {

            const auto& node_key = node.keys[idx].first;


            if (!compare_keys(key, node_key) && !compare_keys(node_key, key)) {
                return node.keys[idx].second;
            }
        }

        return std::nullopt;
    } catch (const exception& e) {
        std::cerr << "Error in at(): " << e.what() << std::endl;
        return std::nullopt;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error in at(): " << e.what() << std::endl;
        return std::nullopt;
    }
}


template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
bool B_tree_disk<tkey, tvalue, compare, t>::insert(const B_tree_disk::tree_data_type& data) {
    try {

        if (_file_for_tree.good() && _file_for_key_value.good()) {

            auto [path, info] = find_path(data.first);
            if (info.second) return false;

            if (path.empty() || _position_root == 0) {
                btree_disk_node root(true);
                root.keys.push_back(data);
                root.size = 1;
                _count_of_node++;
                root.position_in_disk = _count_of_node;
                _position_root = root.position_in_disk;
                disk_write(root);
                write_metadata();
                return true;
            }

            auto [node_pos, idx] = path.top();
            auto node = disk_read(node_pos);

            insert_array(node, 0, data, idx);
            disk_write(node);

            if (node.size > maximum_keys_in_node) {
                auto split_path = path;
                split_node(split_path);
            }

            write_metadata();
            return true;
        }

        return false;
    } catch (const exception& e) {
        std::cerr << "Error in insert: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error in insert: " << e.what() << std::endl;
        return false;
    }
}


template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
size_t B_tree_disk<tkey, tvalue, compare, t>::btree_disk_node::calculate_block_size() const {
    return node_header_size +
           (keys.size() * key_pointer_overhead) +
           (pointers.size() * sizeof(size_t));
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
size_t B_tree_disk<tkey, tvalue, compare, t>::calculate_node_position(size_t node_number) const {
    if (node_number == 0) return 0;

    return sizeof(size_t) * 2 + sizeof(_node_block_size) + (node_number - 1) * _node_block_size;
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
void B_tree_disk<tkey, tvalue, compare, t>::write_metadata() {
    if (!_file_for_tree.is_open()) {
        throw file_error("Tree file is not open for writing metadata");
    }

    std::streampos current_pos = _file_for_tree.tellp();

    _file_for_tree.seekp(0, std::ios::beg);

    if (!_file_for_tree.good()) {
        _file_for_tree.clear();
        throw file_error("Failed to position file pointer for metadata");
    }

    _file_for_tree.write(reinterpret_cast<const char*>(&_count_of_node), sizeof(_count_of_node));
    _file_for_tree.write(reinterpret_cast<const char*>(&_position_root), sizeof(_position_root));
    _file_for_tree.write(reinterpret_cast<const char*>(&_node_block_size), sizeof(_node_block_size));

    if (!_file_for_tree.good()) {
        _file_for_tree.clear();
        throw file_error("Failed to write metadata");
    }

    _file_for_tree.flush();

    if (current_pos >= 0) {
        _file_for_tree.seekp(current_pos);
    }
}


template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
B_tree_disk<tkey, tvalue, compare, t>::B_tree_disk(
    const std::string& file_path,
    const allocator_type& allocator,
    const compare& cmp
) : compare(cmp), _allocator(allocator), _node_block_size(1024) {
    std::filesystem::path base(file_path);
    auto idx_path = base;
    idx_path += ".tree";
    auto data_path = base;
    data_path += ".data";

    bool files_exist = std::filesystem::exists(idx_path) && std::filesystem::exists(data_path);

    if (files_exist) {
        _file_for_tree.open(idx_path, std::ios::in | std::ios::out | std::ios::binary);
        _file_for_key_value.open(data_path, std::ios::in | std::ios::out | std::ios::binary);

        if (_file_for_tree.good() && _file_for_key_value.good()) {
            _file_for_tree.seekg(0, std::ios::beg);

            if (_file_for_tree.read(reinterpret_cast<char*>(&_count_of_node), sizeof(_count_of_node)) &&
                _file_for_tree.read(reinterpret_cast<char*>(&_position_root), sizeof(_position_root)) &&
                _file_for_tree.read(reinterpret_cast<char*>(&_node_block_size), sizeof(_node_block_size))) {

                if (_count_of_node > 0 && _position_root > 0 && _node_block_size >= 1024) {
                    return;
                }
            }
        }

        if (_file_for_tree.is_open()) _file_for_tree.close();
        if (_file_for_key_value.is_open()) _file_for_key_value.close();
    }

    _file_for_tree.open(idx_path, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
    _file_for_key_value.open(data_path, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

    if (!_file_for_tree.good() || !_file_for_key_value.good()) {
        if (_file_for_tree.is_open()) _file_for_tree.close();
        if (_file_for_key_value.is_open()) _file_for_key_value.close();
        throw file_error("Failed to create database files");
    }

    _count_of_node = 0;
    _position_root = 0;

    btree_disk_node root(true);
    root.position_in_disk = ++_count_of_node;
    _position_root = root.position_in_disk;

    write_metadata();
    disk_write(root);
}



template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
void B_tree_disk<tkey, tvalue, compare, t>::disk_write(btree_disk_node& node) {
    if (!_file_for_tree.is_open() || !_file_for_key_value.is_open()) {
        throw file_error("Files not open for writing");
    }

    if (node.position_in_disk == 0) {
        _count_of_node++;
        node.position_in_disk = _count_of_node;
    }

    if (node.keys.size() != node.size) {
        node.size = node.keys.size();
    }

    size_t node_size = node.calculate_block_size();
    if (node_size > _node_block_size) {
        _node_block_size = node_size;
    }

    size_t file_position = sizeof(size_t) * 3;
    file_position += (node.position_in_disk - 1) * _node_block_size;

    _file_for_tree.seekp(file_position, std::ios::beg);

    if (!_file_for_tree.good()) {
        _file_for_tree.clear();
        throw file_error("Failed to position file pointer for writing");
    }

    try {
        node.serialize(_file_for_tree, _file_for_key_value);

        if (!_file_for_tree.good() || !_file_for_key_value.good()) {
            _file_for_tree.clear();
            _file_for_key_value.clear();
            throw file_error("Failed to write node to disk");
        }

        _file_for_tree.flush();
        _file_for_key_value.flush();
    } catch(const std::exception& e) {
        _file_for_tree.clear();
        _file_for_key_value.clear();
        throw;
    }
}



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
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator
B_tree_disk<tkey, tvalue, compare, t>::begin() {
    std::stack<std::pair<size_t, size_t>> path;
    if (_position_root == 0 || _count_of_node == 0) {
        return end();
    }
    try {

        size_t current = _position_root;

        while (true) {
            auto node = disk_read(current);

            path.push({node.position_in_disk, 0});

            if (node._is_leaf) {
                break;
            }

            if (node.pointers.empty()) {
                break;
            }

            current = node.pointers[0];
        }

        if (!path.empty()) {
            auto [pos, idx] = path.top();
            auto node = disk_read(pos);

            if (node.size == 0) {
                return end();
            }
        }

        return btree_disk_const_iterator(*this, path);
    } catch (const std::exception& e) {
        std::cerr << "Error in begin(): " << e.what() << std::endl;
        return end();
    }
}




template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
bool B_tree_disk<tkey, tvalue, compare, t>::erase(const tkey& key) {
    try {
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
            size_t child_pos = node.pointers[index];
            auto pred = disk_read(child_pos);
            while (!pred._is_leaf) {
                child_pos = pred.pointers[pred.size];
                pred = disk_read(child_pos);
            }

            if (pred.size == 0) {
                throw node_error("Predecessor node is empty");
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
        write_metadata();
        return true;
    } catch (const exception& e) {
        std::cerr << "Error in erase: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error in erase: " << e.what() << std::endl;
        return false;
    }
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
void B_tree_disk<tkey, tvalue, compare, t>::rebalance_node(std::stack<std::pair<size_t, size_t>>& path, btree_disk_node& node, size_t& index) {
    constexpr size_t min_keys = minimum_keys_in_node;
    if (node._is_leaf && node.size >= min_keys) return;
    if (!node._is_leaf && node.size >= min_keys) return;

    if (path.empty()) return;


    auto [parent_pos, parent_index] = path.top();
    auto parent = disk_read(parent_pos);


    if (parent_index > 0) {
        size_t left_pos = parent.pointers[parent_index - 1];
        auto left = disk_read(left_pos);
        if (left.size > min_keys) {

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
    if (parent_index > 0) {

        size_t left_pos = parent.pointers[parent_index - 1];
        auto left = disk_read(left_pos);

        left.keys.push_back(parent.keys[parent_index - 1]);
        for (auto& k: node.keys) left.keys.push_back(k);
        if (!node._is_leaf) {
            for (auto& p: node.pointers) left.pointers.push_back(p);
        }

        left.size = left.keys.size();
        disk_write(left);

        parent.keys.erase(parent.keys.begin() + parent_index - 1);
        parent.pointers.erase(parent.pointers.begin() + parent_index);
    } else {
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
    if (index < node.keys.size()) {
        node.keys.erase(node.keys.begin() + index);
    }

    if (remove_left_ptr && index < node.pointers.size()) {
        node.pointers.erase(node.pointers.begin() + index);
    } else if (!remove_left_ptr && index + 1 < node.pointers.size()) {
        node.pointers.erase(node.pointers.begin() + index + 1);
    }

    if (node.size > 0) {
        --node.size;
    }
    return node;
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
bool B_tree_disk<tkey, tvalue, compare, t>::update(const B_tree_disk::tree_data_type& data) {
    try {
        auto [path, info] = find_path(data.first);
        size_t idx = info.first;
        bool found = info.second;
        if (!found) return false;

        auto [node_pos, node_index] = path.top();
        auto node = disk_read(node_pos);

        if (idx < node.keys.size()) {
            node.keys[idx].second = data.second;
            disk_write(node);
            write_metadata();
            return true;
        }
        return false;
    } catch (const exception& e) {
        std::cerr << "Error in update: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error in update: " << e.what() << std::endl;
        return false;
    }
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
void B_tree_disk<tkey, tvalue, compare, t>::split_node(std::stack<std::pair<size_t, size_t>>& path) {
    if (path.empty()) {
        throw node_error("Cannot split node: path is empty");
    }

    auto [pos, index] = path.top();
    path.pop();
    auto node = disk_read(pos);
    if (node.keys.size() < 2*t-1) {
        throw node_error("Cannot split node: too few keys");
    }

    btree_disk_node new_node(node._is_leaf);
    new_node.size = t - 1;
    auto median_key = node.keys[t-1];
    new_node.keys.clear();
    for (size_t i = t; i < node.keys.size(); ++i) {
        new_node.keys.push_back(node.keys[i]);
    }
    if (!node._is_leaf) {
        new_node.pointers.clear();
        for (size_t i = t; i <= node.keys.size() && i < node.pointers.size(); ++i) {
            new_node.pointers.push_back(node.pointers[i]);
        }
    }
    node.keys.resize(t - 1);
    if (!node._is_leaf) {
        node.pointers.resize(t);
    }

    node.size = t - 1;
    _count_of_node++;
    new_node.position_in_disk = _count_of_node;

    disk_write(node);
    disk_write(new_node);

    if (path.empty()) {
        btree_disk_node root_node(false);
        root_node.size = 1;
        root_node.keys.push_back(median_key);
        root_node.pointers.push_back(node.position_in_disk);
        root_node.pointers.push_back(new_node.position_in_disk);
        _count_of_node++;
        root_node.position_in_disk = _count_of_node;
        _position_root = root_node.position_in_disk;
        disk_write(root_node);
        write_metadata();
    } else {
        auto [ppos, pindex] = path.top();
        auto parent = disk_read(ppos);

        if (pindex > parent.keys.size()) {
            pindex = parent.keys.size();
        }

        insert_array(parent, new_node.position_in_disk, median_key, pindex);
        disk_write(parent);
    }
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
void B_tree_disk<tkey, tvalue, compare, t>::insert_array(btree_disk_node& node, size_t right_node, const tree_data_type& data, size_t index) noexcept {
    if (index > node.keys.size()) {
        index = node.keys.size();
    }

    if (index == node.keys.size()) {
        node.keys.push_back(data);
    } else {
        node.keys.insert(node.keys.begin() + index, data);
    }
    if (!node._is_leaf) {
        if (right_node != 0) {
            if (index + 1 >= node.pointers.size()) {
                node.pointers.push_back(right_node);
            } else {
                node.pointers.insert(node.pointers.begin() + index + 1, right_node);
            }
        }
    }

    node.size = node.keys.size();
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
std::pair<std::stack<std::pair<size_t, size_t>>, std::pair<size_t, bool>>
B_tree_disk<tkey, tvalue, compare, t>::find_path(const tkey& key) {
    std::stack<std::pair<size_t, size_t>> path;
    size_t current_pos = _position_root;

    if (current_pos == 0 || _count_of_node == 0) {
        return {path, {0, false}};
    }
    try {
        while (true) {
            auto node = disk_read(current_pos);
            if (node.position_in_disk == 0) {
                throw node_error("Invalid node encountered in find_path");
            }
            auto [idx, found] = find_index(key, node);
            if (idx > node.size) {
                idx = node.size;
            }

            path.push({current_pos, idx});

            if (found) return {path, {idx, true}};
            if (node._is_leaf) return {path, {idx, false}};
            if (node.pointers.empty()) {
                throw node_error("No child pointers in internal node");
            }

            if (idx >= node.pointers.size()) {
                idx = node.pointers.size() - 1;
            }

            current_pos = node.pointers[idx];
            if (current_pos == 0) {
                throw node_error("Invalid child pointer (0) encountered");
            }
        }
    } catch (const exception& e) {
        std::cerr << "Error in find_path: " << e.what() << std::endl;
        return {std::stack<std::pair<size_t, size_t>>(), {0, false}};
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error in find_path: " << e.what() << std::endl;
        return {std::stack<std::pair<size_t, size_t>>(), {0, false}};
    }
}



template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
void B_tree_disk<tkey, tvalue, compare, t>::btree_disk_node::serialize(std::fstream& stream, std::fstream& stream_for_data) const {
    size_t actual_size = keys.size();

    stream.write(reinterpret_cast<const char*>(&actual_size), sizeof(actual_size));
    stream.write(reinterpret_cast<const char*>(&_is_leaf), sizeof(_is_leaf));
    stream.write(reinterpret_cast<const char*>(&position_in_disk), sizeof(position_in_disk));

    size_t key_count = actual_size;
    stream.write(reinterpret_cast<const char*>(&key_count), sizeof(key_count));

    for (size_t i = 0; i < key_count && i < keys.size(); ++i) {
        const auto& kv = keys[i];
        stream_for_data.seekp(0, std::ios::end);
        size_t offset = static_cast<size_t>(stream_for_data.tellp());

        if (!stream_for_data.good()) {
            throw B_tree_disk<tkey, tvalue, compare, t>::file_error("Data stream not good before key-value serialization");
        }
        kv.first.serialize(stream_for_data);
        kv.second.serialize(stream_for_data);

        if (!stream_for_data.good()) {
            throw B_tree_disk<tkey, tvalue, compare, t>::file_error("Failed to serialize key-value pair");
        }

        stream.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
    }
    size_t ptr_count = pointers.size();
    stream.write(reinterpret_cast<const char*>(&ptr_count), sizeof(ptr_count));

    for (size_t i = 0; i < ptr_count && i < pointers.size(); ++i) {
        size_t ptr = pointers[i];
        stream.write(reinterpret_cast<const char*>(&ptr), sizeof(ptr));
    }
    if (!stream.good() || !stream_for_data.good()) {
        throw B_tree_disk<tkey, tvalue, compare, t>::file_error("Stream error after node serialization");
    }
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_node
B_tree_disk<tkey, tvalue, compare, t>::disk_read(size_t node_position) {
    if (!_file_for_tree.is_open() || !_file_for_key_value.is_open()) {
        throw file_error("Files not open for reading");
    }
    if (node_position == 0) {
        return btree_disk_node();
    }
    if (node_position > _count_of_node) {
        throw node_error("Invalid node position: higher than count of nodes");
    }
    size_t file_position = sizeof(size_t) * 3; // _count_of_node, _position_root, _node_block_size
    file_position += (node_position - 1) * _node_block_size;
    _file_for_tree.seekg(0, std::ios::end);
    size_t file_size = static_cast<size_t>(_file_for_tree.tellg());
    if (file_position >= file_size) {
        throw file_error("Invalid file position for reading");
    }
    _file_for_tree.seekg(file_position, std::ios::beg);

    if (!_file_for_tree.good()) {
        _file_for_tree.clear();
        throw file_error("Failed to position file pointer for reading");
    }
    try {
        auto node = btree_disk_node::deserialize(_file_for_tree, _file_for_key_value);
        return node;
    } catch(const std::exception& e) {
        _file_for_tree.clear();
        throw;
    }
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
B_tree_disk<tkey, tvalue, compare, t>::btree_disk_node::btree_disk_node(bool is_leaf)
    : size(0), _is_leaf(is_leaf), position_in_disk(0) {
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
B_tree_disk<tkey, tvalue, compare, t>::btree_disk_node::btree_disk_node()
    : size(0), _is_leaf(true), position_in_disk(0) {
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
bool B_tree_disk<tkey, tvalue, compare, t>::compare_pairs(const B_tree_disk::tree_data_type& lhs,
                                                          const B_tree_disk::tree_data_type& rhs) const {
    return (*this)(lhs.first, rhs.first);
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
bool B_tree_disk<tkey, tvalue, compare, t>::compare_keys(const tkey& lhs, const tkey& rhs) const {
    try {
        return static_cast<const compare&>(*this)(lhs, rhs);
    } catch (const std::exception& e) {
        std::cerr << "Error in compare_keys: " << e.what() << std::endl;
        return false;
    }
}


template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
size_t B_tree_disk<tkey, tvalue, compare, t>::_count_of_node = 0;

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
B_tree_disk<tkey, tvalue, compare, t>::B_tree_disk(B_tree_disk&& other) noexcept
    : compare(static_cast<compare&&>(other)),
      _allocator(std::move(other._allocator)),
      _node_block_size(other._node_block_size),
      _position_root(other._position_root),
      _current_node(std::move(other._current_node)) {
    _file_for_tree.swap(other._file_for_tree);
    _file_for_key_value.swap(other._file_for_key_value);
    other._position_root = 0;
    other._node_block_size = 0;
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
B_tree_disk<tkey, tvalue, compare, t>& B_tree_disk<tkey, tvalue, compare, t>::operator=(B_tree_disk&& other) noexcept {
    if (this != &other) {

        if (_file_for_tree.is_open()) _file_for_tree.close();
        if (_file_for_key_value.is_open()) _file_for_key_value.close();
        static_cast<compare&>(*this) = static_cast<compare&&>(other);
        _allocator = std::move(other._allocator);
        _node_block_size = other._node_block_size;
        _position_root = other._position_root;
        _current_node = std::move(other._current_node);

        _file_for_tree.swap(other._file_for_tree);
        _file_for_key_value.swap(other._file_for_key_value);

        other._position_root = 0;
        other._node_block_size = 0;
    }
    return *this;
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
B_tree_disk<tkey, tvalue, compare, t>::~B_tree_disk() noexcept {
    try {
        if (_file_for_tree.is_open()) {
            write_metadata();
            _file_for_tree.close();
        }

        if (_file_for_key_value.is_open()) {
            _file_for_key_value.close();
        }
    } catch (...) {
    }
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
void B_tree_disk<tkey, tvalue, compare, t>::check_tree(size_t pos, size_t depth) {
    if (pos == 0) return;

    auto node = disk_read(pos);
    size_t min_keys = (pos == _position_root ? 1 : minimum_keys_in_node);
    size_t max_keys = maximum_keys_in_node;

    if (!(node.size >= min_keys && node.size <= max_keys)) {
        throw tree_error("Invalid number of keys in node");
    }
    for (size_t i = 1; i < node.size && i < node.keys.size(); ++i) {
        if (!compare_keys(node.keys[i - 1].first, node.keys[i].first)) {
            throw tree_error("Keys are not in order");
        }
    }
    if (!node._is_leaf) {
        if (node.pointers.size() != node.size + 1) {
            throw tree_error("Invalid number of pointers in node");
        }

        for (size_t i = 0; i < node.pointers.size(); ++i) {
            check_tree(node.pointers[i], depth + 1);
        }
    }
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::btree_disk_const_iterator(
    B_tree_disk<tkey, tvalue, compare, t>& tree,
    const std::stack<std::pair<size_t, size_t>>& path,
    size_t index)
    : _tree(tree), _path(path), _index(index) {
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator
B_tree_disk<tkey, tvalue, compare, t>::end() {

    return btree_disk_const_iterator(*this);
}
template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::value_type
B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::operator*() const noexcept {
    if (_path.empty()) {
        return tree_data_type_const();
    }
    try {
        auto [node_pos, idx] = _path.top();
        auto node = _tree.disk_read(node_pos);

        if (idx < node.keys.size()) {
            return node.keys[idx];
        }
        return tree_data_type_const();
    } catch (...) {
        return tree_data_type_const();
    }
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::self&
B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::operator++() {
    if (_path.empty()) {
        return *this;
    }
    try {
        auto [current_pos, current_idx] = _path.top();
        auto current_node = _tree.disk_read(current_pos);
        if (!current_node._is_leaf) {
            if (current_idx + 1 < current_node.pointers.size()) {
                size_t min_pos = current_node.pointers[current_idx + 1];
                _path.top().second++;
                while (true) {
                    auto min_node = _tree.disk_read(min_pos);
                    _path.push({min_pos, 0});
                    if (min_node._is_leaf) {
                        break;
                    }
                    if (min_node.pointers.empty()) {
                        break;
                    }
                    min_pos = min_node.pointers[0];
                }
                return *this;
            }
        }

        if (current_idx + 1 < current_node.size) {
            _path.top().second++;
            return *this;
        }
        std::stack<std::pair<size_t, size_t>> new_path;
        size_t child_pos = current_pos;
        _path.pop();
        bool found_parent = false;
        while (!_path.empty() && !found_parent) {
            auto [parent_pos, parent_idx] = _path.top();
            _path.pop();

            auto parent_node = _tree.disk_read(parent_pos);
            int child_idx = -1;
            for (size_t i = 0; i < parent_node.pointers.size(); i++) {
                if (parent_node.pointers[i] == child_pos) {
                    child_idx = i;
                    break;
                }
            }
            if (child_idx == -1) {
                _path = std::stack<std::pair<size_t, size_t>>();
                return *this;
            }
            if (child_idx < parent_node.size) {

                new_path.push({parent_pos, child_idx});
                found_parent = true;
                break;
            }
            child_pos = parent_pos;
        }
        if (found_parent) {

            while (!new_path.empty()) {
                _path.push(new_path.top());
                new_path.pop();
            }
            return *this;
        }
        return *this;
    } catch (const std::exception& e) {
        std::cerr << "Error in operator++: " << e.what() << std::endl;
        _path = std::stack<std::pair<size_t, size_t>>();
        return *this;
    }
}
template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator
B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::operator++(int) {
    self tmp = *this;
    ++(*this);
    return tmp;
}
template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::self&
B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::operator--() {
    if (_path.empty()) {

        try {
            auto max_element = _tree.find_max_element(_tree._position_root);
            if (max_element.first.size > 0) {
                _path.push({max_element.first.position_in_disk, max_element.second});
            }
            return *this;
        } catch (const std::exception& e) {
            std::cerr << "Error in operator-- from end(): " << e.what() << std::endl;
            return *this;
        }
    }
    try {
        auto [node_pos, idx] = _path.top();
        auto node = _tree.disk_read(node_pos);
        if (idx > 0) {
            _path.top().second = idx - 1;
            if (!node._is_leaf) {
                size_t pos = node.pointers[idx];
                auto [max_node, max_idx] = _tree.find_max_element(pos);
                _path.push({max_node.position_in_disk, max_idx});
            }

            return *this;
        }
        _path.pop();

        if (_path.empty()) {
            auto begin_it = _tree.begin();
            _path = begin_it._path;
            _index = begin_it._index;
            return *this;
        }

        return *this;
    } catch (const exception& e) {
        std::cerr << "Error in operator--: " << e.what() << std::endl;
        return *this;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error in operator--: " << e.what() << std::endl;
        return *this;
    }
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator
B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::operator--(int) {
    self tmp = *this;
    --(*this);
    return tmp;
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
bool B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::operator==(const self& other) const noexcept {
    if (_path.empty() && other._path.empty()) {
        return true;
    }
    if (_path.empty() || other._path.empty()) {
        return false;
    }
    auto [this_node, this_idx] = _path.top();
    auto [other_node, other_idx] = other._path.top();
    return this_node == other_node && this_idx == other_idx;
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
bool B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator::operator!=(const self& other) const noexcept {
    return !(*this == other);
}

template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
std::pair<typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_node, size_t>
B_tree_disk<tkey, tvalue, compare, t>::find_max_element(size_t node_position) {
    if (node_position == 0) {
        throw node_error("Cannot find max element in empty subtree");
    }

    auto node = disk_read(node_position);

    if (node.size == 0) {
        throw node_error("Empty node encountered when finding max element");
    }

    if (node._is_leaf) {

        return {node, node.size - 1};
    }

    return find_max_element(node.pointers[node.size]);
}
template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
std::pair<typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_node, size_t>
B_tree_disk<tkey, tvalue, compare, t>::find_min_element(size_t node_position) {
    if (node_position == 0) {
        throw node_error("Cannot find min element in empty subtree");
    }
    auto node = disk_read(node_position);
    if (node.size == 0) {
        throw node_error("Empty node encountered when finding min element");
    }

    if (node._is_leaf) {
        return {node, 0};
    }
    return find_min_element(node.pointers[0]);
}
template<serializable tkey, serializable tvalue, compator<tkey> compare, std::size_t t>
std::pair<typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator,
          typename B_tree_disk<tkey, tvalue, compare, t>::btree_disk_const_iterator>
B_tree_disk<tkey, tvalue, compare, t>::find_range(const tkey& lower, const tkey& upper, bool include_lower, bool include_upper) {
    try {
        auto [lower_path, lower_info] = find_path(lower);
        auto [lower_idx, lower_found] = lower_info;
        auto [upper_path, upper_info] = find_path(upper);
        auto [upper_idx, upper_found] = upper_info;
        auto lower_it = btree_disk_const_iterator(*this, lower_path, lower_idx);
        auto upper_it = btree_disk_const_iterator(*this, upper_path, upper_idx);
        if (!include_lower && lower_found) {
            ++lower_it;
        }
        if (!include_upper && upper_found) {
            btree_disk_const_iterator end_iterator(*this);
            return std::make_pair(lower_it, end_iterator);
        } else if (upper_found) {
            ++upper_it;
        }
        return std::make_pair(lower_it, upper_it);
    } catch (const exception& e) {
        std::cerr << "Error in find_range(): " << e.what() << std::endl;
        btree_disk_const_iterator end_iterator(*this);
        return std::make_pair(end_iterator, end_iterator);
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error in find_range(): " << e.what() << std::endl;
        btree_disk_const_iterator end_iterator(*this);
        return std::make_pair(end_iterator, end_iterator);
    }
}
#endif//B_TREE_DISK_HPP