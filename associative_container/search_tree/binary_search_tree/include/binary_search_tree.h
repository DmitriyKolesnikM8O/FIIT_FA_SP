#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_BINARY_SEARCH_TREE_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_BINARY_SEARCH_TREE_H

#include <list>
#include <stack>
#include <vector>
#include <memory>
#include <logger.h>
#include <logger_guardant.h>
#include <not_implemented.h>
#include <search_tree.h>
#include <stack>
#include <ranges>
#include <pp_allocator.h>
#include <concepts>

namespace __detail
{
    template<typename tkey, typename tvalue, typename compare, typename tag>
    class bst_impl;

    class BST_TAG;
}


template<typename tkey, typename tvalue, compator<tkey> compare = std::less<tkey>, typename tag = __detail::BST_TAG>
class binary_search_tree : private compare
{
public:

    using value_type = std::pair<const tkey, tvalue>;

    friend class __detail::bst_impl<tkey, tvalue, compare, tag>;

protected:

    struct node
    {

    public:

        value_type data;

        node* parent;
        node* left_subtree;
        node* right_subtree;

        template<class ...Args>
        explicit node(node* parent, Args&& ...args);


        virtual ~node() =default;
    };
    friend class __detail::bst_impl<tkey, tvalue, compare, tag>;
    friend class binary_search_tree<tkey, tvalue, compare, tag>::node;
    inline bool compare_keys(const tkey& lhs, const tkey& rhs) const;

    inline bool compare_pairs(const value_type & lhs, const value_type & rhs) const;

public:




    class prefix_iterator;
    class prefix_const_iterator;
    class prefix_reverse_iterator;
    class prefix_const_reverse_iterator;

    class infix_iterator;
    class infix_const_iterator;
    class infix_reverse_iterator;
    class infix_const_reverse_iterator;

    class postfix_iterator;
    class postfix_const_iterator;
    class postfix_reverse_iterator;
    class postfix_const_reverse_iterator;

    class prefix_iterator
    {
    protected:

        node* _data;
        node* _backup;

    public:

        using value_type = binary_search_tree<tkey, tvalue, compare>::value_type;
        using difference_type = ptrdiff_t;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit prefix_iterator(node* data = nullptr, node* backup = nullptr);

        virtual ~prefix_iterator() =default;

        bool operator==(
            prefix_iterator const &other) const noexcept;

        bool operator!=(
            prefix_iterator const &other) const noexcept;

        prefix_iterator &operator++() & noexcept;

        prefix_iterator operator++(int not_used) noexcept;

        prefix_iterator &operator--() & noexcept;

        prefix_iterator const operator--(int not_used) const noexcept;

        reference operator*();


        pointer operator->() noexcept;
        size_t depth() const noexcept;

    };

    class prefix_const_iterator
    {
    protected:

        prefix_iterator _base;

    public:

        using value_type = binary_search_tree<tkey, tvalue, compare>::value_type;
        using difference_type = ptrdiff_t;
        using reference = const value_type&;
        using pointer = value_type* const;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit prefix_const_iterator(const node* data = nullptr, const node* backup = nullptr);

        prefix_const_iterator(const prefix_iterator&) noexcept;

        virtual ~prefix_const_iterator() =default;

        bool operator==(
                prefix_iterator const &other) const noexcept;

        bool operator!=(
                prefix_iterator const &other) const noexcept;

        prefix_const_iterator &operator++() & noexcept;

        prefix_const_iterator operator++(int not_used) noexcept;

        prefix_const_iterator &operator--() & noexcept;

        prefix_const_iterator const operator--(int not_used) const noexcept;

        reference operator*();


        pointer operator->() noexcept;
        size_t depth() const noexcept;

    };

    class prefix_reverse_iterator
    {
    protected:

        prefix_iterator _base;

    public:

        using value_type = binary_search_tree<tkey, tvalue, compare>::value_type;
        using difference_type = ptrdiff_t;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit prefix_reverse_iterator(node* data = nullptr, node* backup = nullptr);

        prefix_reverse_iterator(const prefix_iterator&) noexcept;
        operator prefix_iterator() const noexcept;

        prefix_iterator base() const noexcept;

        virtual ~prefix_reverse_iterator() =default;

        bool operator==(prefix_reverse_iterator const &other) const noexcept;

        bool operator!=(prefix_reverse_iterator const &other) const noexcept;

        prefix_reverse_iterator &operator++() & noexcept;

        prefix_reverse_iterator operator++(int not_used) noexcept;

        prefix_reverse_iterator &operator--() & noexcept;

        prefix_reverse_iterator const operator--(int not_used) const noexcept;

        reference operator*();


        pointer operator->() noexcept;
        size_t depth() const noexcept;

    };

    class prefix_const_reverse_iterator
    {
    protected:

        prefix_const_iterator _base;

    public:

        using value_type = binary_search_tree<tkey, tvalue, compare>::value_type;
        using difference_type = ptrdiff_t;
        using reference = const value_type&;
        using pointer = value_type* const;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit prefix_const_reverse_iterator(const node* data = nullptr, const node* backup = nullptr);

        prefix_const_reverse_iterator(const prefix_const_iterator&) noexcept;

        operator prefix_const_iterator() const noexcept;
        prefix_const_iterator base() const noexcept;

        virtual ~prefix_const_reverse_iterator() =default;

        bool operator==(prefix_const_reverse_iterator const &other) const noexcept;

        bool operator!=(prefix_const_reverse_iterator const &other) const noexcept;

        prefix_const_reverse_iterator &operator++() & noexcept;

        prefix_const_reverse_iterator operator++(int not_used) noexcept;

        prefix_const_reverse_iterator &operator--() & noexcept;

        prefix_const_reverse_iterator const operator--(int not_used) const noexcept;

        reference operator*();

        pointer operator->() noexcept;
        size_t depth() const noexcept;

    };


    class infix_iterator
    {
        friend class binary_search_tree;
        friend class __detail::bst_impl<tkey, tvalue, compare, tag>;
    protected:

        node* _data;

        node* _backup;


    public:

        using value_type = binary_search_tree<tkey, tvalue, compare>::value_type;
        using difference_type = ptrdiff_t;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit infix_iterator(node* data = nullptr, node* backup = nullptr);

        virtual ~infix_iterator() =default;

        bool operator==(
                infix_iterator const &other) const noexcept;

        bool operator!=(
                infix_iterator const &other) const noexcept;

        infix_iterator &operator++() & noexcept;

        infix_iterator operator++(int not_used) noexcept;

        infix_iterator &operator--() & noexcept;

        infix_iterator const operator--(int not_used) const noexcept;

        reference operator*();

        pointer operator->() noexcept;
        size_t depth() const noexcept;

    };

    class infix_const_iterator
    {
    protected:

        infix_iterator _base;

    public:

        using value_type = binary_search_tree<tkey, tvalue, compare>::value_type;
        using difference_type = ptrdiff_t;
        using reference = const value_type&;
        using pointer = value_type* const;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit infix_const_iterator(const node* data = nullptr, const node* backup = nullptr);

        infix_const_iterator(const infix_iterator&) noexcept;

        virtual ~infix_const_iterator() =default;

        bool operator==(
                infix_iterator const &other) const noexcept;

        bool operator!=(
                infix_iterator const &other) const noexcept;

        infix_const_iterator &operator++() & noexcept;

        infix_const_iterator operator++(int not_used) noexcept;

        infix_const_iterator &operator--() & noexcept;

        infix_const_iterator const operator--(int not_used) const noexcept;

        reference operator*();


        pointer operator->() noexcept;
        size_t depth() const noexcept;

    };

    class infix_reverse_iterator
    {
    protected:

        infix_iterator _base;

    public:

        using value_type = binary_search_tree<tkey, tvalue, compare>::value_type;
        using difference_type = ptrdiff_t;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit infix_reverse_iterator(node* data = nullptr, node* backup = nullptr);

        infix_reverse_iterator(const infix_iterator&) noexcept;
        operator infix_iterator() const noexcept;

        infix_iterator base() const noexcept;

        virtual ~infix_reverse_iterator() =default;

        bool operator==(infix_reverse_iterator const &other) const noexcept;

        bool operator!=(infix_reverse_iterator const &other) const noexcept;

        infix_reverse_iterator &operator++() & noexcept;

        infix_reverse_iterator operator++(int not_used) noexcept;

        infix_reverse_iterator &operator--() & noexcept;

        infix_reverse_iterator const operator--(int not_used) const noexcept;

        reference operator*();


        pointer operator->() noexcept;
        size_t depth() const noexcept;

    };

    class infix_const_reverse_iterator
    {
    protected:

        infix_const_iterator _base;

    public:

        using value_type = binary_search_tree<tkey, tvalue, compare>::value_type;
        using difference_type = ptrdiff_t;
        using reference = const value_type&;
        using pointer = value_type* const;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit infix_const_reverse_iterator(const node* data = nullptr, const node* backup = nullptr);

        infix_const_reverse_iterator(const infix_const_iterator&) noexcept;

        operator infix_const_iterator() const noexcept;
        infix_const_iterator base() const noexcept;

        virtual ~infix_const_reverse_iterator() =default;

        bool operator==(infix_const_reverse_iterator const &other) const noexcept;

        bool operator!=(infix_const_reverse_iterator const &other) const noexcept;

        infix_const_reverse_iterator &operator++() & noexcept;

        infix_const_reverse_iterator operator++(int not_used) noexcept;

        infix_const_reverse_iterator &operator--() & noexcept;

        infix_const_reverse_iterator const operator--(int not_used) const noexcept;

        reference operator*();


        pointer operator->() noexcept;
        size_t depth() const noexcept;

    };


    class postfix_iterator
    {
    protected:

        node* _data;

        node* _backup;

    public:

        using value_type = binary_search_tree<tkey, tvalue, compare>::value_type;
        using difference_type = ptrdiff_t;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit postfix_iterator(node* data = nullptr, node* backup = nullptr);

        virtual ~postfix_iterator() =default;

        bool operator==(
                postfix_iterator const &other) const noexcept;

        bool operator!=(
                postfix_iterator const &other) const noexcept;

        postfix_iterator &operator++() & noexcept;

        postfix_iterator operator++(int not_used) noexcept;

        postfix_iterator &operator--() & noexcept;

        postfix_iterator const operator--(int not_used) const noexcept;

        reference operator*();


        pointer operator->() noexcept;
        size_t depth() const noexcept;

    };

    class postfix_const_iterator
    {
    protected:

        postfix_iterator _base;

    public:

        using value_type = binary_search_tree<tkey, tvalue, compare>::value_type;
        using difference_type = ptrdiff_t;
        using reference = const value_type&;
        using pointer = value_type* const;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit postfix_const_iterator(const node* data = nullptr, const node* backup = nullptr);

        postfix_const_iterator(const postfix_iterator&) noexcept;

        virtual ~postfix_const_iterator() =default;

        bool operator==(
                postfix_iterator const &other) const noexcept;

        bool operator!=(
                postfix_iterator const &other) const noexcept;

        postfix_const_iterator &operator++() & noexcept;

        postfix_const_iterator operator++(int not_used) noexcept;

        postfix_const_iterator &operator--() & noexcept;

        postfix_const_iterator const operator--(int not_used) const noexcept;

        reference operator*();


        pointer operator->() noexcept;
        size_t depth() const noexcept;

    };

    class postfix_reverse_iterator
    {
    protected:

        postfix_iterator _base;

    public:

        using value_type = binary_search_tree<tkey, tvalue, compare>::value_type;
        using difference_type = ptrdiff_t;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit postfix_reverse_iterator(node* data = nullptr, node* backup = nullptr);

        postfix_reverse_iterator(const postfix_iterator&) noexcept;
        operator postfix_iterator() const noexcept;

        postfix_iterator base() const noexcept;

        virtual ~postfix_reverse_iterator() =default;

        bool operator==(postfix_reverse_iterator const &other) const noexcept;

        bool operator!=(postfix_reverse_iterator const &other) const noexcept;

        postfix_reverse_iterator &operator++() & noexcept;

        postfix_reverse_iterator operator++(int not_used) noexcept;

        postfix_reverse_iterator &operator--() & noexcept;

        postfix_reverse_iterator const operator--(int not_used) const noexcept;

        reference operator*();


        pointer operator->() noexcept;
        size_t depth() const noexcept;

    };

    class postfix_const_reverse_iterator
    {
    protected:

        postfix_const_iterator _base;

    public:

        using value_type = binary_search_tree<tkey, tvalue, compare>::value_type;
        using difference_type = ptrdiff_t;
        using reference = const value_type&;
        using pointer = value_type* const;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit postfix_const_reverse_iterator(const node* data = nullptr, const node* backup = nullptr);

        postfix_const_reverse_iterator(const postfix_const_iterator&) noexcept;

        operator postfix_const_iterator() const noexcept;
        postfix_const_iterator base() const noexcept;

        virtual ~postfix_const_reverse_iterator() =default;

        bool operator==(postfix_const_reverse_iterator const &other) const noexcept;

        bool operator!=(postfix_const_reverse_iterator const &other) const noexcept;

        postfix_const_reverse_iterator &operator++() & noexcept;

        postfix_const_reverse_iterator operator++(int not_used) noexcept;

        postfix_const_reverse_iterator &operator--() & noexcept;

        postfix_const_reverse_iterator const operator--(int not_used) const noexcept;

        reference operator*();


        pointer operator->() noexcept;
        size_t depth() const noexcept;

    };



    friend class prefix_iterator;
    friend class prefix_const_iterator;
    friend class prefix_reverse_iterator;
    friend class prefix_const_reverse_iterator;

    friend class postfix_iterator;
    friend class postfix_const_iterator;
    friend class postfix_reverse_iterator;
    friend class postfix_const_reverse_iterator;

    friend class infix_iterator;
    friend class infix_const_iterator;
    friend class infix_reverse_iterator;
    friend class infix_const_reverse_iterator;



protected:

    node *_root;
    logger* _logger;
    size_t _size;

    pp_allocator<value_type> _allocator;

public:
    explicit binary_search_tree(
            const compare& comp = compare(),
            pp_allocator<value_type> alloc = pp_allocator<value_type>(),
            logger *logger = nullptr);

    explicit binary_search_tree(
            pp_allocator<value_type> alloc,
            const compare& comp = compare(),
            logger *logger = nullptr);


    template<input_iterator_for_pair<tkey, tvalue> iterator>
    explicit binary_search_tree(iterator begin, iterator end, const compare& cmp = compare(),
                                pp_allocator<value_type> alloc = pp_allocator<value_type>(),
                                logger* logger = nullptr);

    template<std::ranges::input_range Range>
    explicit binary_search_tree(Range&& range, const compare& cmp = compare(),
                                pp_allocator<value_type> alloc = pp_allocator<value_type>(),
                                logger* logger = nullptr);


    binary_search_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(),
                       pp_allocator<value_type> alloc = pp_allocator<value_type>(),
                       logger* logger = nullptr);

public:

    binary_search_tree(const binary_search_tree &other);

    binary_search_tree(binary_search_tree &&other) noexcept;

    binary_search_tree &operator=(const binary_search_tree &other);

    binary_search_tree &operator=(binary_search_tree &&other) noexcept;

    virtual ~binary_search_tree();

public:

    tvalue& at(const tkey& key);
    const tvalue& at(const tkey& key) const;

    tvalue& operator[](const tkey& key);
    tvalue& operator[](tkey&& key);

    bool empty() const noexcept;

    size_t size() const noexcept;

    void clear() noexcept;

    void clear(node *n);

    std::pair<infix_iterator, bool> insert(const value_type&);
    std::pair<infix_iterator, bool> insert(value_type&&);

    template<std::input_iterator InputIt>
    void insert(InputIt first, InputIt last);

    template<std::ranges::input_range R>
    void insert_range( R&& rg );

    template<class ...Args>
    std::pair<infix_iterator, bool> emplace(Args&&...args);

    infix_iterator insert_or_assign(const value_type&);
    infix_iterator insert_or_assign(value_type&&);

    template<std::input_iterator InputIt >
    void insert_or_assign(InputIt first, InputIt last);


    template<class ...Args>
    infix_iterator emplace_or_assign(Args&&...args);

    virtual void swap(binary_search_tree& other) noexcept;

    bool contains(const tkey& key) const;

    infix_iterator find(const tkey&);
    infix_const_iterator find(const tkey&) const;


    infix_iterator lower_bound(const tkey&);
    infix_const_iterator lower_bound(const tkey&) const;

    infix_iterator upper_bound(const tkey&);
    infix_const_iterator upper_bound(const tkey&) const;

    infix_iterator erase(infix_iterator pos);
    infix_iterator erase(infix_const_iterator pos);

    infix_iterator erase(infix_iterator first, infix_iterator last);
    infix_iterator erase(infix_const_iterator first, infix_const_iterator last);

    size_t erase(const tkey& key);

public:

    infix_iterator begin() noexcept;

    infix_iterator end() noexcept;

    infix_const_iterator begin() const noexcept;

    infix_const_iterator end() const noexcept;

    infix_const_iterator cbegin() const noexcept;

    infix_const_iterator cend() const noexcept;

    infix_reverse_iterator rbegin() noexcept;

    infix_reverse_iterator rend() noexcept;

    infix_const_reverse_iterator rbegin() const noexcept;

    infix_const_reverse_iterator rend() const noexcept;

    infix_const_reverse_iterator crbegin() const noexcept;

    infix_const_reverse_iterator crend() const noexcept;


    prefix_iterator begin_prefix() noexcept;

    prefix_iterator end_prefix() noexcept;

    prefix_const_iterator begin_prefix() const noexcept;

    prefix_const_iterator end_prefix() const noexcept;

    prefix_const_iterator cbegin_prefix() const noexcept;

    prefix_const_iterator cend_prefix() const noexcept;

    prefix_reverse_iterator rbegin_prefix() noexcept;

    prefix_reverse_iterator rend_prefix() noexcept;

    prefix_const_reverse_iterator rbegin_prefix() const noexcept;

    prefix_const_reverse_iterator rend_prefix() const noexcept;

    prefix_const_reverse_iterator crbegin_prefix() const noexcept;

    prefix_const_reverse_iterator crend_prefix() const noexcept;


    infix_iterator begin_infix() noexcept;

    infix_iterator end_infix() noexcept;

    infix_const_iterator begin_infix() const noexcept;

    infix_const_iterator end_infix() const noexcept;

    infix_const_iterator cbegin_infix() const noexcept;

    infix_const_iterator cend_infix() const noexcept;

    infix_reverse_iterator rbegin_infix() noexcept;

    infix_reverse_iterator rend_infix() noexcept;

    infix_const_reverse_iterator rbegin_infix() const noexcept;

    infix_const_reverse_iterator rend_infix() const noexcept;

    infix_const_reverse_iterator crbegin_infix() const noexcept;

    infix_const_reverse_iterator crend_infix() const noexcept;


    postfix_iterator begin_postfix() noexcept;

    postfix_iterator end_postfix() noexcept;

    postfix_const_iterator begin_postfix() const noexcept;

    postfix_const_iterator end_postfix() const noexcept;

    postfix_const_iterator cbegin_postfix() const noexcept;

    postfix_const_iterator cend_postfix() const noexcept;

    postfix_reverse_iterator rbegin_postfix() noexcept;

    postfix_reverse_iterator rend_postfix() noexcept;

    postfix_const_reverse_iterator rbegin_postfix() const noexcept;

    postfix_const_reverse_iterator rend_postfix() const noexcept;

    postfix_const_reverse_iterator crbegin_postfix() const noexcept;

    postfix_const_reverse_iterator crend_postfix() const noexcept;

protected:

    static void small_left_rotation(node *&subtree_root) noexcept;

    static void small_right_rotation(node *&subtree_root) noexcept;

    static void big_left_rotation(node *&subtree_root) noexcept;

    static void big_right_rotation(node *&subtree_root) noexcept;

    static void double_left_rotation(node *&subtree_root) noexcept;

    static void double_right_rotation(node *&subtree_root) noexcept;

};

namespace __detail
{
    template<typename tkey, typename tvalue, typename compare, typename tag>
    class bst_impl
    {
        friend class binary_search_tree<tkey, tvalue, compare, tag>;
        template<class ...Args>
        static binary_search_tree<tkey, tvalue, compare, tag>::node* create_node(binary_search_tree<tkey, tvalue, compare, tag>& cont, Args&& ...args);

        static void delete_node(binary_search_tree<tkey, tvalue, compare, tag>& cont, binary_search_tree<tkey, tvalue, compare, tag>::node** node);

        static void post_search(binary_search_tree<tkey, tvalue, compare, tag>::node**){}

        static void post_insert(binary_search_tree<tkey, tvalue, compare, tag>& cont, binary_search_tree<tkey, tvalue, compare, tag>::node**){}

        static void erase(binary_search_tree<tkey, tvalue, compare, tag>& cont, binary_search_tree<tkey, tvalue, compare, tag>::node**);

        static void swap(binary_search_tree<tkey, tvalue, compare, tag>& lhs, binary_search_tree<tkey, tvalue, compare, tag>& rhs) noexcept;
    };
}

template<typename tkey, typename tvalue, typename compare, typename tag>
void __detail::bst_impl<tkey, tvalue, compare, tag>::swap(binary_search_tree<tkey, tvalue, compare, tag> &lhs,
                                                binary_search_tree<tkey, tvalue, compare, tag> &rhs) noexcept
{
    using std::swap;
    swap(lhs.root, rhs.root);
    swap(lhs._logger, rhs._logger);
    swap(lhs._size, rhs._size);
    swap(lhs._allocator, rhs._allocator);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
template<input_iterator_for_pair<tkey, tvalue> iterator>
binary_search_tree<tkey, tvalue, compare, tag>::binary_search_tree(iterator begin, iterator end, const compare &cmp,
                                                                   pp_allocator<typename binary_search_tree<tkey, tvalue, compare, tag>::value_type> alloc, logger *logger) : compare(cmp), _root(nullptr), _logger(logger), _size(0), _allocator(alloc)
{
    for (auto it = begin; it != end; ++it)
    {
        emplace(it->first, it->second);
    }
}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::compare_pairs(const binary_search_tree::value_type &lhs,
                                                              const binary_search_tree::value_type &rhs) const
{
    return compare_keys(lhs.first, rhs.first);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::compare_keys(const tkey &lhs, const tkey &rhs) const
{
    return compare::operator()(lhs, rhs);
}

template<typename compare, typename U, typename iterator>
explicit binary_search_tree(iterator begin, iterator end, const compare& cmp = compare(),
                            pp_allocator<U> alloc = pp_allocator<U>(),
                            logger* logger = nullptr) -> binary_search_tree<const typename std::iterator_traits<iterator>::value_type::first_type, typename std::iterator_traits<iterator>::value_type::second_type, compare>;

template<typename compare, typename U, std::ranges::forward_range Range>
explicit binary_search_tree(Range&& range, const compare& cmp = compare(),
        pp_allocator<U> alloc = pp_allocator<U>(),
        logger* logger = nullptr) -> binary_search_tree<const typename std::iterator_traits<typename std::ranges::iterator_t<Range>>::value_type::first_type, typename std::iterator_traits<typename std::ranges::iterator_t<Range>>::value_type::second_type, compare> ;

template<typename tkey, typename tvalue, typename compare, typename U>
binary_search_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(),
        pp_allocator<U> alloc = pp_allocator<U>(),
        logger* logger = nullptr) -> binary_search_tree<tkey, tvalue, compare>;

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
template<class ...Args>
binary_search_tree<tkey, tvalue, compare, tag>::node::node(node* parent, Args&& ...args) : data(std::forward<Args>(args)...), parent(parent), left_subtree(nullptr), right_subtree(nullptr)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator::prefix_iterator(node* data, node* backup) : _data(data), _backup(backup)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator::operator==(
        prefix_iterator const &other) const noexcept
{
    return _data == &other._data;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator::operator!=(
        prefix_iterator const &other) const noexcept
{
    return _data != &other._data;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator::operator++() & noexcept
{
    if (_data == nullptr)
    {
        return *this;
    }
    if (_data->left_subtree != nullptr)
    {
        _backup = _data;
        _data = _data->left_subtree;
    }
    else if (_data->right_subtree != nullptr)
    {
        _backup = _data;
        _data = _data->right_subtree;
    }
    else
    {
        _backup = _data;
        node* parent = _data->parent;
        while (parent != nullptr && (_data == parent->right_subtree || parent->right_subtree == nullptr))
        {
            _data = parent;
            parent = parent->parent;
        }

        if (parent == nullptr)
        {
            _data = nullptr;
        }
        else
        {
            _data = parent->right_subtree;
        }
    }
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator::operator++(int not_used) noexcept
{
    prefix_iterator temp = *this;
    ++(*this);
    return temp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator::operator--() & noexcept
{
    if (_data == nullptr)
    {
        return *this;
    }

    _backup = _data;
    node* parent = _data->parent;

    if (parent == nullptr)
    {
        _data = nullptr;
    }
    else if (parent->left_subtree == _data)
    {
        _data = parent;
    }
    else if (parent->right_subtree == _data)
    {
        node* temp = parent->left_subtree;
        while (temp->right_subtree || temp->left_subtree)
        {
            if (temp->right_subtree)
            {
                temp = temp->right_subtree;
            }
            else
            {
                temp = temp->left_subtree;
            }
        }
        _data = temp;
    }

    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator const
binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator::operator--(int not_used) const noexcept
{
    prefix_iterator temp = *this;
    --(*this);
    return temp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator::reference
binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator::operator*()
{
    if (!_data)
    {
        throw std::out_of_range("Dereferencing end or invalid iterator");
    }
    return _data->data;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator::pointer
binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator::operator->() noexcept
{
    return _data ? &(_data->data) : nullptr;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator::depth() const noexcept
{
    size_t depth = 0;
    node* current = _data;
    while (current->parent != nullptr) {
        current = current->parent;
        ++depth;
    }
    return depth;
}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator::prefix_const_iterator(const node* data, const node* backup) : _base(const_cast<node*>(data), const_cast<node*>(backup))
{
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator::prefix_const_iterator(const prefix_iterator& other) noexcept : _base(other)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator::operator==(
        prefix_iterator const &other) const noexcept
{
    return _base == other._base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator::operator!=(
        prefix_iterator const &other) const noexcept
{
    return _base != other._base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator::operator++() & noexcept
{
    ++_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator::operator++(int not_used) noexcept
{
    prefix_const_iterator temp = *this;
    ++_base;
    return temp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator::operator--() & noexcept
{
    --_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator const
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator::operator--(int not_used) const noexcept
{
    prefix_const_iterator temp = *this;
    --_base;
    return temp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator::reference
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator::operator*()
{
    return _base.operator*();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator::pointer
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator::operator->() noexcept
{
    return _base.operator->();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator::depth() const noexcept
{
    return _base.depth();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::prefix_reverse_iterator(node* data, node* backup) : _base(data, backup)
{

}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::prefix_reverse_iterator(const prefix_iterator& it) noexcept : _base(it)
{
    --_base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::operator binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator() const noexcept
{
    auto copy = _base;
    ++copy;
    return copy;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::base() const noexcept
{
    return _base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::operator==(prefix_reverse_iterator const &other) const noexcept
{
    return _base == other._base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::operator!=(prefix_reverse_iterator const &other) const noexcept
{
    return _base != other._base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::operator++() & noexcept
{
    --_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::operator++(int not_used) noexcept
{
    prefix_reverse_iterator tmp = *this;
    --_base;
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::operator--() & noexcept
{
    ++_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator const
binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::operator--(int not_used) const noexcept
{
    prefix_reverse_iterator tmp = *this;
    ++_base;
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::reference
binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::operator*()
{
    return _base.operator*();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::pointer
binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::operator->() noexcept
{
    return _base.operator->();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator::depth() const noexcept
{
    return _base.depth();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::prefix_const_reverse_iterator(const node* data, const node* backup) : _base(data, backup)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::prefix_const_reverse_iterator(const prefix_const_iterator& it) noexcept : _base(it._base)
{
    --_base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::operator binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator() const noexcept
{
    auto copy = _base;
    ++copy;
    return copy;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::base() const noexcept
{
    return _base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::operator==(prefix_const_reverse_iterator const &other) const noexcept
{
    return _base == other._base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::operator!=(prefix_const_reverse_iterator const &other) const noexcept
{
    return _base != other._base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::operator++() & noexcept
{
    --_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::operator++(int not_used) noexcept
{
    prefix_const_reverse_iterator tmp = *this;
    --_base;
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::operator--() & noexcept
{
    ++_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator const
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::operator--(int not_used) const noexcept
{
    prefix_const_reverse_iterator tmp = *this;
    ++_base;
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::reference
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::operator*()
{
    return _base.operator*();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::pointer
binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::operator->() noexcept
{
    return _base.operator->();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator::depth() const noexcept
{
    return _base.depth();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator::infix_iterator(node* data, node* backup) : _data(data), _backup(backup)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator::operator==(infix_iterator const &other) const noexcept
{
    return _data == other._data;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator::operator!=(infix_iterator const &other) const noexcept
{
    return _data != other._data;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator::operator++() & noexcept
{
    if (_data == nullptr)
    {
        return *this;
    }

    _backup = _data;

    if (_data->right_subtree != nullptr)
    {
        _data = _data->right_subtree;
        while (_data->left_subtree != nullptr)
        {
            _data = _data->left_subtree;
        }
    }
    else
    {
        node* parent = _data->parent;
        while (parent != nullptr && _data == parent->right_subtree)
        {
            _data = parent;
            parent = parent->parent;
        }
        _data = parent;
    }

    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator::operator++(int not_used) noexcept
{
    infix_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator::operator--() & noexcept
{
    if (_data == nullptr)
    {
        _data = _backup;
        return *this;
    }

    _backup = _data;

    if (_data->left_subtree != nullptr)
    {
        _data = _data->left_subtree;
        while (_data->right_subtree != nullptr)
        {
            _data = _data->right_subtree;
        }
    }
    else
    {
        node* parent = _data->parent;
        while (parent != nullptr && _data == parent->left_subtree)
        {
            _data = parent;
            parent = parent->parent;
        }
        _data = parent;
    }

    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator const
binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator::operator--(int not_used) const noexcept
{
    infix_iterator tmp = *this;
    --(*this);
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator::reference
binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator::operator*()
{
    if (!_data)
    {
        throw std::out_of_range("Dereferencing end or invalid iterator");
    }
    return _data->data;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator::pointer
binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator::operator->() noexcept
{
    return _data ? &(_data->data) : nullptr;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator::depth() const noexcept
{
    size_t depth = 0;
    node* current = _data;
    while (current->parent != nullptr) {
        current = current->parent;
        ++depth;
    }
    return depth;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator::infix_const_iterator(const node* data, const node* backup) : _base(const_cast<node*>(data), const_cast<node*>(backup))
{
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator::infix_const_iterator(const infix_iterator& it) noexcept : _base(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator::operator==(infix_iterator const &other) const noexcept
{
    return _base == other._base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator::operator!=(infix_iterator const &other) const noexcept
{
    return _base != other._base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator::operator++() & noexcept
{
    ++_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator::operator++(int not_used) noexcept
{
    infix_const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator::operator--() & noexcept
{
    --_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator const
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator::operator--(int not_used) const noexcept
{
    infix_const_iterator tmp = *this;
    --(*this);
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator::reference
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator::operator*()
{
    return _base.operator*();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator::pointer
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator::operator->() noexcept
{
    return _base.operator->();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator::depth() const noexcept
{
    return _base.depth();
}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::infix_reverse_iterator(node* data, node* backup) : _base(data, backup)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::infix_reverse_iterator(const infix_iterator& it) noexcept : _base(it)
{
    --_base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::operator binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator() const noexcept
{
    auto copy = _base;
    ++copy;
    return copy;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::base() const noexcept
{
    return _base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::operator==(infix_reverse_iterator const &other) const noexcept
{
    return _base == other._base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::operator!=(infix_reverse_iterator const &other) const noexcept
{
    return _base != other._base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::operator++() & noexcept
{
    --_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::operator++(int not_used) noexcept
{
    infix_reverse_iterator tmp = *this;
    --_base;
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::operator--() & noexcept
{
    ++_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator const
binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::operator--(int not_used) const noexcept
{
    infix_reverse_iterator tmp = *this;
    ++_base;
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::reference
binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::operator*()
{
    return _base.operator*();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::pointer
binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::operator->() noexcept
{
    return _base.operator->();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator::depth() const noexcept
{
    return _base.depth();
}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::infix_const_reverse_iterator(const node* data, const node* backup) : _base(data, backup)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::infix_const_reverse_iterator(const infix_const_iterator& it) noexcept : _base(it)
{
    --_base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::operator binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator() const noexcept
{
    auto copy = _base;
    ++copy;
    return copy;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::base() const noexcept
{
    return _base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::operator==(infix_const_reverse_iterator const &other) const noexcept
{
    return _base == other._base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::operator!=(infix_const_reverse_iterator const &other) const noexcept
{
    return _base != other._base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::operator++() & noexcept
{
    --_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::operator++(int not_used) noexcept
{
    infix_const_reverse_iterator tmp = *this;
    --_base;
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::operator--() & noexcept
{
    ++_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator const
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::operator--(int not_used) const noexcept
{
    infix_const_reverse_iterator tmp = *this;
    ++_base;
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::reference
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::operator*()
{
    return _base.operator*();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::pointer
binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::operator->() noexcept
{
    return _base.operator->();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator::depth() const noexcept
{
    return _base->depth();
}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator::postfix_iterator(node* data, node* backup) : _data(data), _backup(backup)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator::operator==(postfix_iterator const &other) const noexcept
{
    return _data == other._data;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator::operator!=(postfix_iterator const &other) const noexcept
{
    return _data != other._data;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator::operator++() & noexcept
{
    if (_data == nullptr)
    {
        return *this;
    }

    _backup = _data;

    node* parent = _data->parent;

    if (parent == nullptr || _data == parent->right_subtree || parent->right_subtree == nullptr)
    {
        _data = parent;
    }
    else
    {
        _data = parent->right_subtree;
        while (_data->left_subtree != nullptr || _data->right_subtree != nullptr)
        {
            if (_data->left_subtree != nullptr)
            {
                _data = _data->left_subtree;
            }
            else
            {
                _data = _data->right_subtree;
            }
        }
    }

    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator::operator++(int not_used) noexcept
{
    postfix_iterator tmp = *this;
    ++_data;
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator::operator--() & noexcept
{
    if (_data == nullptr)
    {
        _data = _backup;
        return *this;
    }

    _backup = _data;
    node* parent = _data->parent;

    if (_data->right_subtree != nullptr)
    {
        _data = _data->right_subtree;
    }
    else
    {
        while (_data != parent->right_subtree)
        {
            _data = parent;
            parent = parent->parent;
        }
        _data = parent->left_subtree;
    }
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator const
binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator::operator--(int not_used) const noexcept
{
    postfix_iterator tmp = *this;
    --(*this);
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator::reference
binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator::operator*()
{
    if (!_data)
    {
        throw std::out_of_range("Dereferencing end or invalid iterator");
    }
    return _data->data;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator::pointer
binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator::operator->() noexcept
{
    return _data ? &(_data->data) : nullptr;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator::depth() const noexcept
{
    size_t depth = 0;
    node* current = _data;
    while (current->parent != nullptr) {
        current = current->parent;
        ++depth;
    }
    return depth;
}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator::postfix_const_iterator(const node* data, const node* backup) : _base(const_cast<node*>(data), const_cast<node*>(backup))
{
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator::postfix_const_iterator(const postfix_iterator& it) noexcept : _base(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator::operator==(postfix_iterator const &other) const noexcept
{
    return _base == other._base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator::operator!=(postfix_iterator const &other) const noexcept
{
    return _base != other._base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator::operator++() & noexcept
{
    ++_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator::operator++(int not_used) noexcept
{
    postfix_const_iterator tmp = *this;
    ++_base;
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator::operator--() & noexcept
{
    --_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator const
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator::operator--(int not_used) const noexcept
{
    postfix_const_iterator tmp = *this;
    --_base;
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator::reference
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator::operator*()
{
    return _base.operator*();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator::pointer
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator::operator->() noexcept
{
    return _base.operator->();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator::depth() const noexcept
{
    return _base.depth();
}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::postfix_reverse_iterator(node* data, node* backup) : _base(data, backup)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::postfix_reverse_iterator(const postfix_iterator& it) noexcept : _base(it)
{
    --_base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::operator binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator() const noexcept
{
    auto copy = _base;
    ++copy;
    return copy;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::base() const noexcept
{
    return _base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::operator==(postfix_reverse_iterator const &other) const noexcept
{
    return _base == other._base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::operator!=(postfix_reverse_iterator const &other) const noexcept
{
    return _base != other._base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::operator++() & noexcept
{
    --_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::operator++(int not_used) noexcept
{
    postfix_reverse_iterator tmp = *this;
    --_base;
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::operator--() & noexcept
{
    ++_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator const
binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::operator--(int not_used) const noexcept
{
    postfix_reverse_iterator tmp = *this;
    ++_base;
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::reference
binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::operator*()
{
    return _base.operator*();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::pointer
binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::operator->() noexcept
{
    return _base.operator->();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator::depth() const noexcept
{
    return _base.depth();
}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::postfix_const_reverse_iterator(const node* data, const node* backup) : _base(const_cast<node*>(data), const_cast<node*>(backup))
{
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::postfix_const_reverse_iterator(const postfix_const_iterator& it) noexcept : _base(it)
{
    --_base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::operator binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator() const noexcept
{
    auto copy = _base;
    ++copy;
    return copy;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::base() const noexcept
{
    return _base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::operator==(postfix_const_reverse_iterator const &other) const noexcept
{
    return _base == other._base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::operator!=(postfix_const_reverse_iterator const &other) const noexcept
{
    return _base != other._base;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::operator++() & noexcept
{
    --_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::operator++(int not_used) noexcept
{
    postfix_const_reverse_iterator tmp = *this;
    --_base;
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator &
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::operator--() & noexcept
{
    ++_base;
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator const
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::operator--(int not_used) const noexcept
{
    postfix_const_reverse_iterator tmp = *this;
    --_base;
    return tmp;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::reference
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::operator*()
{
    return _base.operator*();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::pointer
binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::operator->() noexcept
{
    return _base.operator->();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator::depth() const noexcept
{
    return _base.depth();
}


template <typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::binary_search_tree(const compare &comp, pp_allocator<value_type> alloc,
                                                                   logger *logger)
    : compare(comp), _allocator(alloc), _logger(logger), _root(nullptr)
{
}

template <typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::binary_search_tree(pp_allocator<value_type> alloc, const compare &comp,
                                                                   logger *logger)
    : compare(comp), _allocator(alloc), _logger(logger), _root(nullptr)
{
}

template <typename tkey, typename tvalue, compator<tkey> compare, typename tag>
template <std::ranges::input_range Range>
binary_search_tree<tkey, tvalue, compare, tag>::binary_search_tree(Range &&range, const compare &cmp,
                                                                   pp_allocator<value_type> alloc, logger *logger)
    : compare(cmp), _allocator(alloc), _logger(logger), _root(nullptr)
{
    for (auto &&element : range)
    {
        emplace(std::forward<decltype(element)>(element));
    }
}

template <typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::binary_search_tree(std::initializer_list<std::pair<tkey, tvalue>> data,
                                                                   const compare &cmp, pp_allocator<value_type> alloc,
                                                                   logger *logger)
    : compare(cmp), _allocator(alloc), _logger(logger), _root(nullptr)
{
    for (const auto &element : data)
    {
        emplace(element.first, element.second);
    }
}


template <typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::binary_search_tree(const binary_search_tree &other)
    : compare(other.compare), _logger(other._logger), _allocator(other._allocator)
{
    if (other._root)
    {
        _root = _allocator.template new_object<node>(nullptr, other._root->data);
        for (auto it = other.begin(); it != other.end(); ++it)
        {
            if (it != other.begin())
            {
                emplace(it->first, it->second);
            }
        }
    }
    else
    {
        _root = nullptr;
    }
}

template <typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::binary_search_tree(binary_search_tree &&other) noexcept
    : compare(std::move(other)), _root(other._root), _logger(other._logger), _allocator(std::move(other._allocator)),
      _size(other._size)
{
    other._root = nullptr;
    other._logger = nullptr;
    other._size = 0;
}

template <typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag> &binary_search_tree<tkey, tvalue, compare, tag>::operator=(
    const binary_search_tree &other)
{
    if (this != &other)
    {
        binary_search_tree temp(other);
        __detail::bst_impl<tkey, tvalue, compare, tag>::swap(*this, temp);
    }
    return *this;
}

template <typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag> &binary_search_tree<tkey, tvalue, compare, tag>::operator=(
    binary_search_tree &&other) noexcept
{
    if (this != &other)
    {
        __detail::bst_impl<tkey, tvalue, compare, tag>::swap(*this, other);
    }
    return *this;
}

template <typename tkey, typename tvalue, compator<tkey> compare, typename tag>
binary_search_tree<tkey, tvalue, compare, tag>::~binary_search_tree()
{
    clear(_root);
}


template <typename tkey, typename tvalue, compator<tkey> compare, typename tag>
tvalue &binary_search_tree<tkey, tvalue, compare, tag>::at(const tkey &key)
{
    node *current = _root;
    while (current != nullptr)
    {
        if (compare_keys(key, current->data.first))
        {
            current = current->left_subtree;
        }
        else if (compare_keys(current->data.first, key))
        {
            current = current->right_subtree;
        }
        else
        {
            return current->data.second;
        }
    }
    throw std::out_of_range("Key not found");
}

template <typename tkey, typename tvalue, compator<tkey> compare, typename tag>
const tvalue &binary_search_tree<tkey, tvalue, compare, tag>::at(const tkey &key) const
{
    const node *current = _root;
    while (current != nullptr)
    {
        if (compare_keys(key, current->data.first))
        {
            current = current->left_subtree;
        }
        else if (compare_keys(current->data.first, key))
        {
            current = current->right_subtree;
        }
        else
        {
            return current->data.second;
        }
    }
    throw std::out_of_range("Key not found");
}

template <typename tkey, typename tvalue, compator<tkey> compare, typename tag>
tvalue &binary_search_tree<tkey, tvalue, compare, tag>::operator[](const tkey &key)
{
    {
        node *current = _root;
        node *parent = nullptr;

        while (current != nullptr)
        {
            if (compare_keys(key, current->data.first))
            {
                parent = current;
                current = current->left_subtree;
            }
            else if (compare_keys(current->data.first, key))
            {
                parent = current;
                current = current->right_subtree;
            }
            else
            {
                return current->data.second;
            }
        }


        throw std::out_of_range("Key not found");
        // Исправление: вместо исключения вставляем новый узел.
        // node *new_node = __detail::bst_impl<tkey, tvalue, compare, tag>::create_node(
        //     *this, parent, value_type(key, tvalue()));
        // if (parent == nullptr)
        // {
        //     _root = new_node;
        // }
        // else if (compare_keys(key, parent->data.first))
        // {
        //     parent->left_subtree = new_node;
        // }
        // else
        // {
        //     parent->right_subtree = new_node;
        // }
        // ++_size;
        // return new_node->data.second;
    }
}

template <typename tkey, typename tvalue, compator<tkey> compare, typename tag>
tvalue &binary_search_tree<tkey, tvalue, compare, tag>::operator[](tkey &&key)
{
    node *current = _root;
    node *parent = nullptr;

    while (current != nullptr)
    {
        if (compare_keys(key, current->data.first))
        {
            parent = current;
            current = current->left_subtree;
        }
        else if (compare_keys(current->data.first, key))
        {
            parent = current;
            current = current->right_subtree;
        }
        else
        {
            return current->data.second;
        }
    }
    throw std::out_of_range("Key not found");
    // Исправление: вместо исключения вставляем новый узел.
    // node *new_node = __detail::bst_impl<tkey, tvalue, compare, tag>::create_node(
    //     *this, parent, value_type(std::move(key), tvalue()));
    // if (parent == nullptr)
    // {
    //     _root = new_node;
    // }
    // else if (compare_keys(key, parent->data.first))
    // {
    //     parent->left_subtree = new_node;
    // }
    // else
    // {
    //     parent->right_subtree = new_node;
    // }
    // ++_size;
    // return new_node->data.second;
}

template <typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::empty() const noexcept
{
    return _root == nullptr;
}

template <typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::size() const noexcept
{
    return this->_size;
}

template <typename tkey, typename tvalue, compator<tkey> compare, typename tag>
void binary_search_tree<tkey, tvalue, compare, tag>::clear() noexcept
{
    clear(_root);
}

template <typename tkey, typename tvalue, compator<tkey> compare, typename tag>
void binary_search_tree<tkey, tvalue, compare, tag>::clear(node *n)
{
    if (n != nullptr)
    {
        clear(n->left_subtree);
        clear(n->right_subtree);
        _allocator.delete_object(n);
    }
}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
std::pair<typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator, bool>
binary_search_tree<tkey, tvalue, compare, tag>::insert(const value_type& value)
{
    node* current = _root;
    node* parent = nullptr;

    // Поиск подходящего места для вставки
    while (current != nullptr)
    {
        parent = current;
        if (compare_keys(value.first, current->data.first))
        {
            current = current->left_subtree;
        }
        else if (compare_keys(current->data.first, value.first))
        {
            current = current->right_subtree;
        }
        else
        {

            return std::make_pair(infix_iterator(current), false);
        }
    }


    node* new_node = __detail::bst_impl<tkey, tvalue, compare, tag>::create_node(*this, parent, value);


    if (parent == nullptr)
    {
        _root = new_node;
    }
    else if (compare_keys(new_node->data.first, parent->data.first))
    {
        parent->left_subtree = new_node;
    }
    else
    {
        parent->right_subtree = new_node;
    }

    ++_size;

    if (_logger)
    {
        _logger->log("Successfully inserted new node", logger::severity::debug);
    }

    __detail::bst_impl<tkey, tvalue, compare, tag>::post_insert(*this, &new_node);
    return std::make_pair(infix_iterator(new_node), true);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
std::pair<typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator, bool>
binary_search_tree<tkey, tvalue, compare, tag>::insert(value_type&& value)
{
    node* current = _root;
    node* parent = nullptr;

    while (current != nullptr)
    {
        parent = current;
        if (compare_keys(value.first, current->data.first))
        {
            current = current->left_subtree;
        }
        else if (compare_keys(current->data.first, value.first))
        {
            current = current->right_subtree;
        }
        else
        {

            return std::make_pair(infix_iterator(current), false);
        }
    }

    node* new_node = __detail::bst_impl<tkey, tvalue, compare, tag>::create_node(*this, parent, std::move(value));

    if (parent == nullptr)
    {
        _root = new_node;
    }
    else if (compare_keys(new_node->data.first, parent->data.first))
    {
        parent->left_subtree = new_node;
    }
    else
    {
        parent->right_subtree = new_node;
    }

    ++_size;
    if (_logger)
    {
        _logger->log("Successfully inserted new node with move semantics", logger::severity::debug);
    }
    __detail::bst_impl<tkey, tvalue, compare, tag>::post_insert(*this, &new_node);
    return std::make_pair(infix_iterator(new_node), true);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
template<std::input_iterator InputIt>
void binary_search_tree<tkey, tvalue, compare, tag>::insert(InputIt first, InputIt last)
{
    for (auto it = first; it != last; ++it)
    {
        insert(*it);
    }
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
template<std::ranges::input_range R>
void binary_search_tree<tkey, tvalue, compare, tag>::insert_range(R&& rg)
{
    for (auto&& element : rg)
    {
        insert(std::forward<decltype(element)>(element));
    }
}

template <typename tkey, typename tvalue, compator<tkey> compare, typename tag>
template <class... Args>
std::pair<typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator, bool> binary_search_tree<
    tkey, tvalue, compare, tag>::emplace(Args &&...args)
{
    value_type value(std::forward<Args>(args)...);
    return insert(value);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::insert_or_assign(const value_type& value)
{
    node* current = _root;
    node* parent = nullptr;


    while (current != nullptr)
    {
        parent = current;
        if (compare_keys(value.first, current->data.first))
        {
            current = current->left_subtree;
        }
        else if (compare_keys(current->data.first, value.first))
        {
            current = current->right_subtree;
        }
        else
        {

            current->data.second = value.second;
            return infix_iterator(current);
        }
    }


    node* new_node = __detail::bst_impl<tkey, tvalue, compare, tag>::create_node(*this, parent, value);


    if (parent == nullptr)
    {
        _root = new_node;
    }
    else if (compare_keys(new_node->data.first, parent->data.first))
    {
        parent->left_subtree = new_node;
    }
    else
    {
        parent->right_subtree = new_node;
    }

    ++_size;

    if (_logger)
    {
        _logger->log("Inserted or assigned new node", logger::severity::debug);
    }
    __detail::bst_impl<tkey, tvalue, compare, tag>::post_insert(*this, &new_node);
    return infix_iterator(new_node);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::insert_or_assign(value_type&& value)
{
    node* current = _root;
    node* parent = nullptr;


    while (current != nullptr)
    {
        parent = current;
        if (compare_keys(value.first, current->data.first))
        {
            current = current->left_subtree;
        }
        else if (compare_keys(current->data.first, value.first))
        {
            current = current->right_subtree;
        }
        else
        {

            current->data.second = std::move(value.second);
            return infix_iterator(current);
        }
    }


    node* new_node = __detail::bst_impl<tkey, tvalue, compare, tag>::create_node(*this, parent, std::move(value));


    if (parent == nullptr)
    {
        _root = new_node;
    }
    else if (compare_keys(new_node->data.first, parent->data.first))
    {
        parent->left_subtree = new_node;
    }
    else
    {
        parent->right_subtree = new_node;
    }

    ++_size;

    if (_logger)
    {
        _logger->log("Inserted or assigned new node with move semantics", logger::severity::debug);
    }
    __detail::bst_impl<tkey, tvalue, compare, tag>::post_insert(*this, &new_node);
    return infix_iterator(new_node);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
template<std::input_iterator InputIt>
void binary_search_tree<tkey, tvalue, compare, tag>::insert_or_assign(InputIt first, InputIt last)
{
    if (_logger)
    {
        _logger->log("Starting range insert_or_assign", logger::severity::debug);
    }

    for (auto it = first; it != last; ++it)
    {
        if constexpr (std::is_rvalue_reference_v<decltype(*it)>)
        {
            insert_or_assign(std::move(*it));
        }
        else
        {
            insert_or_assign(*it);
        }
    }

    if (_logger)
    {
        _logger->log("Range insert_or_assign completed", logger::severity::debug);
    }
}

template <typename tkey, typename tvalue, compator<tkey> compare, typename tag>
template <class... Args>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator binary_search_tree<
    tkey, tvalue, compare, tag>::emplace_or_assign(Args &&...args)
{

    value_type temp(std::forward<Args>(args)...);

    node* current = _root;
    node* parent = nullptr;


    while (current != nullptr)
    {
        parent = current;
        if (compare_keys(temp.first, current->data.first))
        {
            current = current->left_subtree;
        }
        else if (compare_keys(current->data.first, temp.first))
        {
            current = current->right_subtree;
        }
        else
        {

            current->data.second = std::move(temp.second);
            return infix_iterator(current);
        }
    }


    node* new_node = __detail::bst_impl<tkey, tvalue, compare, tag>::create_node(*this, parent, std::move(temp));


    if (parent == nullptr)
    {
        _root = new_node;
    }
    else if (compare_keys(new_node->data.first, parent->data.first))
    {
        parent->left_subtree = new_node;
    }
    else
    {
        parent->right_subtree = new_node;
    }

    ++_size;

    if (_logger)
    {
        _logger->log("Emplaced or assigned new node", logger::severity::debug);
    }
    __detail::bst_impl<tkey, tvalue, compare, tag>::post_insert(*this, &new_node);
    return infix_iterator(new_node);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
void binary_search_tree<tkey, tvalue, compare, tag>::swap(binary_search_tree& other) noexcept
{
    if (this == &other)
    {
        return;
    }

    node* this_root = _root;
    pp_allocator<value_type> this_alloc = _allocator;
    logger* this_logger = _logger;

    try {
        _root = other._root;
        _allocator = other._allocator;
        _logger = other._logger;

        other._root = this_root;
        other._allocator = this_alloc;
        other._logger = this_logger;

        if (_logger)
        {
            _logger->log("Swapped trees (this)", logger::severity::debug);
        }
        if (other._logger)
        {
            other._logger->log("Swapped trees (other)", logger::severity::debug);
        }
    }
    catch (...) {
        _root = this_root;
        _allocator = this_alloc;
        _logger = this_logger;

        std::terminate();
    }
}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
bool binary_search_tree<tkey, tvalue, compare, tag>::contains(const tkey& key) const
{
    node* current = _root;
    while (current != nullptr)
    {
        if (compare::operator()(key, current->data.first))
        {
            current = current->left_subtree;
        }
        else if (compare::operator()(current->data.first, key))
        {
            current = current->right_subtree;
        }
        else
        {
            return true;
        }
    }
    return false;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::find(const tkey& key)
{
    node* current = _root;
    while (current != nullptr)
    {
        if (compare::operator()(key, current->data.first))
        {
            current = current->left_subtree;
        }
        else if (compare::operator()(current->data.first, key))
        {
            current = current->right_subtree;
        }
        else
        {
            return infix_iterator(current);
        }
    }
    return end();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::find(const tkey& key) const
{
    const node* current = _root;
    while (current != nullptr)
    {
        if (compare::operator()(key, current->data.first))
        {
            current = current->left_subtree;
        }
        else if (compare::operator()(current->data.first, key))
        {
            current = current->right_subtree;
        }
        else
        {
            return infix_const_iterator(current);
        }
    }
    return cend();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::lower_bound(const tkey& key)
{
    node* current = _root;
    node* result = nullptr;

    while (current != nullptr)
    {
        if (!compare::operator()(current->data.first, key))
        {
            result = current;
            current = current->left_subtree;
        }
        else
        {
            current = current->right_subtree;
        }
    }
    return infix_iterator(result);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::lower_bound(const tkey& key) const
{
    node* current = _root;
    node* result = nullptr;

    while (current != nullptr)
    {
        if (!compare::operator()(current->data.first, key))
        {
            result = current;
            current = current->left_subtree;
        }
        else
        {
            current = current->right_subtree;
        }
    }
    return infix_const_iterator(result);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::upper_bound(const tkey& key)
{
    node* current = _root;
    node* result = nullptr;

    while (current != nullptr)
    {
        if (compare::operator()(key, current->data.first))
        {
            result = current;
            current = current->left_subtree;
        }
        else
        {
            current = current->right_subtree;
        }
    }
    return infix_iterator(result);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::upper_bound(const tkey& key) const
{
    node* current = _root;
    node* result = nullptr;

    while (current != nullptr)
    {
        if (compare::operator()(key, current->data.first))
        {
            result = current;
            current = current->left_subtree;
        }
        else
        {
            current = current->right_subtree;
        }
    }
    return infix_const_iterator(result);
}

template <typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator binary_search_tree<
    tkey, tvalue, compare, tag>::erase(infix_iterator pos)
{
    if (pos == end())
    {
        return end();
    }

    auto to_delete = pos;
    node *target = pos._data;
    node *next = nullptr;

    auto next_temp = ++pos;

    if (target->left_subtree)
    {
        next = target->left_subtree;
        while (next->right_subtree)
        {
            next = next->right_subtree;
        }
    }
    else
    {
        node *parent = target->parent;
        while (parent && target == parent->right_subtree)
        {
            target = parent;
            parent = parent->parent;
        }
        next = parent;
    }

    __detail::bst_impl<tkey, tvalue, compare, tag>::erase(*this, &to_delete._data);

    auto iter = infix_iterator(next);

    return infix_iterator(next);
}

template <typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator binary_search_tree<
    tkey, tvalue, compare, tag>::erase(infix_const_iterator pos)
{
    if (pos == end())
    {
        throw std::out_of_range("Iterator out of range");
    }
    return __detail::bst_impl<tkey, tvalue, compare, tag>::erase(*this, &pos._base._data);
}

template <typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator binary_search_tree<
    tkey, tvalue, compare, tag>::erase(infix_iterator first, infix_iterator last)
{
    auto it = first;
    while (it != last)
    {
        it = erase(it);
    }
    return infix_iterator(it._base._data);
}

template <typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator binary_search_tree<
    tkey, tvalue, compare, tag>::erase(infix_const_iterator first, infix_const_iterator last)
{
    auto it = first;
    while (it != last)
    {
        it = erase(it);
    }
    return infix_iterator(it._base._data);
}

template <typename tkey, typename tvalue, compator<tkey> compare, typename tag>
size_t binary_search_tree<tkey, tvalue, compare, tag>::erase(const tkey &key)
{

    auto it = find(key);
    if (it == end())
    {
        return 0;
    }
    erase(it);
    return 1;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::begin() noexcept
{
    if (_root == nullptr)
    {
        return infix_iterator(nullptr);
    }

    node* current = _root;
    while (current->left_subtree != nullptr)
    {
        current = current->left_subtree;
    }

    return infix_iterator(current);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::end() noexcept
{
    if (_root == nullptr)
    {
        return infix_iterator(nullptr);
    }

    node* current = _root;
    while (current->right_subtree != nullptr)
    {
        current = current->right_subtree;
    }

    return infix_iterator(nullptr,current);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>:: begin() const noexcept
{
    if (_root == nullptr)
    {
        return infix_const_iterator(nullptr);
    }

    node* current = _root;
    while (current->left_subtree != nullptr)
    {
        current = current->left_subtree;
    }

    return infix_const_iterator(current);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::end() const noexcept
{
    if (_root == nullptr)
    {
        return infix_const_iterator(nullptr);
    }

    node* current = _root;
    while (current->right_subtree != nullptr)
    {
        current = current->right_subtree;
    }

    return infix_const_iterator(nullptr,current);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::cbegin() const noexcept
{
    return begin();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::cend() const noexcept
{
    return end();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rbegin() noexcept
{
    if (_root == nullptr)
    {
        return infix_reverse_iterator(nullptr);
    }

    node* current = _root;
    while (current->right_subtree != nullptr)
    {
        current = current->right_subtree;
    }

    return infix_reverse_iterator(current);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rend() noexcept
{
    if (_root == nullptr)
    {
        return infix_reverse_iterator(nullptr);
    }

    node* current = _root;
    while (current->left_subtree != nullptr)
    {
        current = current->left_subtree;
    }

    return infix_reverse_iterator(nullptr,current);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rbegin() const noexcept
{
    if (_root == nullptr)
    {
        return infix_const_reverse_iterator(nullptr);
    }

    node* current = _root;
    while (current->right_subtree != nullptr)
    {
        current = current->right_subtree;
    }

    return infix_const_reverse_iterator(current);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rend() const noexcept
{
    if (_root == nullptr)
    {
        return infix_const_reverse_iterator(nullptr);
    }

    node* current = _root;
    while (current->left_subtree != nullptr)
    {
        current = current->left_subtree;
    }

    return infix_const_reverse_iterator(nullptr,current);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::crbegin() const noexcept
{
    return rbegin();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::crend() const noexcept
{
    return rend();
}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::begin_prefix() noexcept
{
    if (_root == nullptr)
    {
        return prefix_iterator(nullptr);
    }
    return prefix_iterator(_root);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::end_prefix() noexcept
{
    if (_root == nullptr)
    {
        return prefix_iterator(nullptr);
    }
    node* current = _root;
    while (current->right_subtree != nullptr)
    {
        current = current->right_subtree;
    }
    return prefix_iterator(nullptr, current);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::begin_prefix() const noexcept
{
    if (_root == nullptr)
    {
        return prefix_const_iterator(nullptr);
    }
    return prefix_const_iterator(_root);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::end_prefix() const noexcept
{
    if (_root == nullptr)
    {
        return prefix_const_iterator(nullptr);
    }
    node* current = _root;
    while (current->right_subtree != nullptr)
    {
        current = current->right_subtree;
    }
    return prefix_const_iterator(nullptr, current);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::cbegin_prefix() const noexcept
{
    return begin_prefix();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::cend_prefix() const noexcept
{
    return end_prefix();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rbegin_prefix() noexcept
{
    if (_root == nullptr)
    {
        return prefix_reverse_iterator(nullptr);
    }
    node* current = _root;
    while (current->right_subtree != nullptr)
    {
        current = current->right_subtree;
    }
    return prefix_reverse_iterator(current);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rend_prefix() noexcept
{
    if (_root == nullptr)
    {
        return prefix_reverse_iterator(nullptr);
    }
    return prefix_reverse_iterator(nullptr, _root);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rbegin_prefix() const noexcept
{
    if (_root == nullptr)
    {
        return prefix_const_reverse_iterator(nullptr);
    }
    node* current = _root;
    while (current->right_subtree != nullptr)
    {
        current = current->right_subtree;
    }
    return prefix_const_reverse_iterator(current);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rend_prefix() const noexcept
{
    if (_root == nullptr)
    {
        return prefix_const_reverse_iterator(nullptr);
    }
    return prefix_const_reverse_iterator(nullptr, _root);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::crbegin_prefix() const noexcept
{
    return rbegin_prefix();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::prefix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::crend_prefix() const noexcept
{
    return rend_prefix();
}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::begin_infix() noexcept
{
    return begin();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::end_infix() noexcept
{
    return end();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::begin_infix() const noexcept
{
    return begin();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::end_infix() const noexcept
{
    return end();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::cbegin_infix() const noexcept
{
    return cbegin();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::cend_infix() const noexcept
{
    return cend();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rbegin_infix() noexcept
{
    return rbegin();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rend_infix() noexcept
{
    return rend();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rbegin_infix() const noexcept
{
    return rbegin();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rend_infix() const noexcept
{
    return rend();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::crbegin_infix() const noexcept
{
    return crbegin();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::infix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::crend_infix() const noexcept
{
    return crend();
}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::begin_postfix() noexcept
{
    if (_root == nullptr)
    {
        return postfix_iterator(nullptr);
    }

    node* current = _root;
    while (current->left_subtree != nullptr || current->right_subtree != nullptr)
    {
        if (current->left_subtree != nullptr)
            current = current->left_subtree;
        else
            current = current->right_subtree;
    }
    return postfix_iterator(current);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_iterator
binary_search_tree<tkey, tvalue, compare, tag>::end_postfix() noexcept
{
    if (_root == nullptr)
    {
        return postfix_iterator(nullptr);
    }
    return postfix_iterator(nullptr, _root);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::begin_postfix() const noexcept
{
    if (_root == nullptr)
    {
        return postfix_const_iterator(nullptr);
    }

    node* current = _root;
    while (current->left_subtree != nullptr || current->right_subtree != nullptr)
    {
        if (current->left_subtree != nullptr)
            current = current->left_subtree;
        else
            current = current->right_subtree;
    }
    return postfix_const_iterator(current);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::end_postfix() const noexcept
{
    if (_root == nullptr)
    {
        return postfix_const_iterator(nullptr);
    }
    return postfix_const_iterator(nullptr, _root);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::cbegin_postfix() const noexcept
{
    return begin_postfix();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_iterator
binary_search_tree<tkey, tvalue, compare, tag>::cend_postfix() const noexcept
{
    return end_postfix();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rbegin_postfix() noexcept
{
    if (_root == nullptr)
    {
        return postfix_reverse_iterator(nullptr);
    }
    return postfix_reverse_iterator(_root);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rend_postfix() noexcept
{
    if (_root == nullptr)
    {
        return postfix_reverse_iterator(nullptr);
    }
    node* current = _root;
    while (current->left_subtree != nullptr)
    {
        current = current->left_subtree;
    }
    return postfix_reverse_iterator(nullptr,current);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rbegin_postfix() const noexcept
{
    if (_root == nullptr)
    {
        return postfix_const_reverse_iterator(nullptr);
    }
    return postfix_const_reverse_iterator(_root);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::rend_postfix() const noexcept
{
    if (_root == nullptr)
    {
        return postfix_const_reverse_iterator(nullptr);
    }
    node* current = _root;
    while (current->left_subtree != nullptr)
    {
        current = current->left_subtree;
    }
    return postfix_const_reverse_iterator(nullptr,current);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::crbegin_postfix() const noexcept
{
    return rbegin_postfix();
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
typename binary_search_tree<tkey, tvalue, compare, tag>::postfix_const_reverse_iterator
binary_search_tree<tkey, tvalue, compare, tag>::crend_postfix() const noexcept
{
    return rend_postfix();
}


template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
void binary_search_tree<tkey, tvalue, compare, tag>::small_left_rotation(node *&subtree_root) noexcept
{
    if (subtree_root == nullptr || subtree_root->right_subtree == nullptr) {
        return;
    }

    node* new_root = subtree_root->right_subtree;
    subtree_root->right_subtree = new_root->left_subtree;

    if (new_root->left_subtree != nullptr) {
        new_root->left_subtree->parent = subtree_root;
    }


    node* original_parent = subtree_root->parent;


    new_root->left_subtree = subtree_root;
    subtree_root->parent = new_root;


    new_root->parent = original_parent;

    if (original_parent != nullptr) {
        if (subtree_root == original_parent->left_subtree) {
            original_parent->left_subtree = new_root;
        } else {
            original_parent->right_subtree = new_root;
        }
    }

    subtree_root = new_root;
}
template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
void binary_search_tree<tkey, tvalue, compare, tag>::small_right_rotation(node *&subtree_root) noexcept
{
    if (subtree_root == nullptr || subtree_root->left_subtree == nullptr)
    {
        return;
    }

    node* new_root = subtree_root->left_subtree;
    subtree_root->left_subtree = new_root->right_subtree;


    if (new_root->right_subtree != nullptr)
    {
        new_root->right_subtree->parent = subtree_root;
    }


    node* original_parent = subtree_root->parent;


    new_root->right_subtree = subtree_root;
    subtree_root->parent = new_root;


    new_root->parent = original_parent;


    if (original_parent != nullptr)
    {
        if (subtree_root == original_parent->left_subtree)
        {
            original_parent->left_subtree = new_root;
        }
        else
        {
            original_parent->right_subtree = new_root;
        }
    }

    subtree_root = new_root;
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
void binary_search_tree<tkey, tvalue, compare, tag>::big_left_rotation(node *&subtree_root) noexcept
{
    if (subtree_root == nullptr || subtree_root->right_subtree == nullptr || subtree_root->right_subtree->left_subtree == nullptr)
    {
        return;
    }

    small_right_rotation(subtree_root->right_subtree);
    small_left_rotation(subtree_root);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
void binary_search_tree<tkey, tvalue, compare, tag>::big_right_rotation(node *&subtree_root) noexcept
{
    if (subtree_root == nullptr || subtree_root->left_subtree == nullptr || subtree_root->left_subtree->right_subtree == nullptr)
    {
        return;
    }

    small_left_rotation(subtree_root->left_subtree);
    small_right_rotation(subtree_root);
}

template<typename tkey, typename tvalue, compator<tkey> compare, typename tag>
void binary_search_tree<tkey, tvalue, compare, tag>::double_left_rotation(node *&subtree_root) noexcept
{
    if (subtree_root == nullptr || subtree_root->left_subtree == nullptr)
    {
        return;
    }
    small_left_rotation(subtree_root->left_subtree);
    small_left_rotation(subtree_root);
}


template <typename tkey, typename tvalue, compator<tkey> compare, typename tag>
void binary_search_tree<tkey, tvalue, compare, tag>::double_right_rotation(node *&subtree_root) noexcept
{
    if (subtree_root == nullptr || subtree_root->right_subtree == nullptr)
    {
        return;
    }
    small_right_rotation(subtree_root->right_subtree);
    small_right_rotation(subtree_root);
}


namespace __detail {
    template<typename tkey, typename tvalue, typename compare, typename tag>
    template<typename ...Args>
    typename binary_search_tree<tkey, tvalue, compare, tag>::node*
    bst_impl<tkey, tvalue, compare, tag>::create_node(binary_search_tree<tkey, tvalue, compare, tag>& cont, Args&& ...args)
    {
        using node_type = typename binary_search_tree<tkey, tvalue, compare, tag>::node;
        auto *new_node = cont._allocator.template new_object<node_type>(std::forward<Args>(args)...);
        return new_node;
    }

    template<typename tkey, typename tvalue, typename compare, typename tag>
    void bst_impl<tkey, tvalue, compare, tag>::delete_node(binary_search_tree<tkey, tvalue, compare, tag>& cont, binary_search_tree<tkey, tvalue, compare, tag>::node** node)
    {
        using node_type = typename binary_search_tree<tkey, tvalue, compare, tag>::node;
        if (node && *node)
        {
            cont._allocator.template delete_object<node_type>(*node);
            *node = nullptr;
        }
    }

    template<typename tkey, typename tvalue, typename compare, typename tag>
    void bst_impl<tkey, tvalue, compare, tag>::erase(binary_search_tree<tkey, tvalue, compare, tag>& cont, typename binary_search_tree<tkey, tvalue, compare, tag>::node** node_ptr)
    {
        using node_type = typename binary_search_tree<tkey, tvalue, compare, tag>::node;
        node_type* node = *node_ptr;

        if (!node) return;


        const bool has_left = node->left_subtree != nullptr;
        const bool has_right = node->right_subtree != nullptr;

        if (!has_left && !has_right)
        {

            if (node->parent)
            {
                if (node->parent->left_subtree == node)
                {
                    node->parent->left_subtree = nullptr;
                }
                else
                {
                    node->parent->right_subtree = nullptr;
                }
            }
            else
            {
                cont._root = nullptr;
            }
            delete_node(cont, node_ptr);
        }
        else if (has_left != has_right)
        {

            node_type* child = has_left ? node->left_subtree : node->right_subtree;
            child->parent = node->parent;

            if (node->parent)
            {
                if (node->parent->left_subtree == node)
                {
                    node->parent->left_subtree = child;
                }
                else
                {
                    node->parent->right_subtree = child;
                }
            }
            else
            {
                cont._root = child;
            }

            delete_node(cont, node_ptr);
            *node_ptr = child;
        }
        else
        {
            node_type** successor_ptr = &(node->left_subtree);
            while ((*successor_ptr)->right_subtree)
            {
                successor_ptr = &((*successor_ptr)->right_subtree);
            }

            node_type* successor = *successor_ptr;


            if (successor->parent->left_subtree == successor)
                successor->parent->left_subtree = successor->right_subtree;
            else
                successor->parent->right_subtree = successor->right_subtree;

            if (successor->right_subtree)
                successor->right_subtree->parent = successor->parent;


            successor->left_subtree = node->left_subtree;
            if (successor->left_subtree)
                successor->left_subtree->parent = successor;

            successor->right_subtree = node->right_subtree;
            if (successor->right_subtree)
                successor->right_subtree->parent = successor;

            successor->parent = node->parent;

            if (node->parent)
            {
                if (node->parent->left_subtree == node)
                    node->parent->left_subtree = successor;
                else
                    node->parent->right_subtree = successor;
            }
            else
            {
                cont._root = successor;
            }

            delete_node(cont, node_ptr);
            *node_ptr = successor;
        }
    }
}

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_BINARY_SEARCH_TREE_H