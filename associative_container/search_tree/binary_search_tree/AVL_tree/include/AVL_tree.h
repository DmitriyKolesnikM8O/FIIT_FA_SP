#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_AVL_TREE_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_AVL_TREE_H

#include <binary_search_tree.h>

namespace __detail
{
    class AVL_TAG;

    template<typename tkey, typename tvalue, typename compare>
    class bst_impl<tkey, tvalue, compare, AVL_TAG>
    {
        friend class binary_search_tree<tkey, tvalue, compare, AVL_TAG>;
        template<class ...Args>
        static binary_search_tree<tkey, tvalue, compare, AVL_TAG>::node* create_node(binary_search_tree<tkey, tvalue, compare, AVL_TAG>& cont, Args&& ...args);

        static void delete_node(binary_search_tree<tkey, tvalue, compare, AVL_TAG>& cont, binary_search_tree<tkey, tvalue, compare, AVL_TAG>::node**);


        static void post_search(binary_search_tree<tkey, tvalue, compare, AVL_TAG>::node**){}


        static void post_insert(binary_search_tree<tkey, tvalue, compare, AVL_TAG>& cont, binary_search_tree<tkey, tvalue, compare, AVL_TAG>::node**);

        static void erase(binary_search_tree<tkey, tvalue, compare, AVL_TAG>& cont, binary_search_tree<tkey, tvalue, compare, AVL_TAG>::node**);

        static void swap(binary_search_tree<tkey, tvalue, compare, AVL_TAG>& lhs, binary_search_tree<tkey, tvalue, compare, AVL_TAG>& rhs) noexcept;
    };
}

template<typename tkey, typename tvalue, compator<tkey> compare = std::less<tkey>>
class AVL_tree final:
    public binary_search_tree<tkey, tvalue, compare, __detail::AVL_TAG>
{

    using parent = binary_search_tree<tkey, tvalue, compare, __detail::AVL_TAG>;
    friend class __detail::bst_impl<tkey, tvalue, compare, __detail::AVL_TAG>;
private:
    
    struct node final: public parent::node
    {
        size_t height;

        void recalculate_height() noexcept;

        short get_balance() const noexcept;

        template<class ...Args>
        node(parent::node* par, Args&&... args);

        ~node() noexcept override =default;
    };

public:

    using value_type = parent::value_type;



    class prefix_iterator : public parent::prefix_iterator
    {
    public:

        using value_type = parent::prefix_iterator::value_type;
        using difference_type = parent::prefix_iterator::difference_type;
        using pointer = parent::prefix_iterator::pointer;
        using reference = parent::prefix_iterator::reference;
        using iterator_category = parent::prefix_iterator::iterator_category;

        explicit prefix_iterator(parent::node* n = nullptr) noexcept;
        prefix_iterator(parent::prefix_iterator) noexcept;

        size_t get_height() const noexcept;
        size_t get_balance() const noexcept;

        using parent::prefix_iterator::depth;
        using parent::prefix_iterator::operator*;
        using parent::prefix_iterator::operator==;
        using parent::prefix_iterator::operator!=;
        using parent::prefix_iterator::operator++;
        using parent::prefix_iterator::operator--;
        using parent::prefix_iterator::operator->;
    };

    class prefix_const_iterator : public parent::prefix_const_iterator
    {
    public:

        using value_type = parent::prefix_const_iterator::value_type;
        using difference_type = parent::prefix_const_iterator::difference_type;
        using pointer = parent::prefix_const_iterator::pointer;
        using reference = parent::prefix_const_iterator::reference;
        using iterator_category = parent::prefix_const_iterator::iterator_category;

        explicit prefix_const_iterator(parent::node* n = nullptr) noexcept;
        prefix_const_iterator(parent::prefix_const_iterator) noexcept;

        size_t get_height() const noexcept;
        size_t get_balance() const noexcept;

        prefix_const_iterator(prefix_iterator) noexcept;

        using parent::prefix_const_iterator::depth;
        using parent::prefix_const_iterator::operator*;
        using parent::prefix_const_iterator::operator==;
        using parent::prefix_const_iterator::operator!=;
        using parent::prefix_const_iterator::operator++;
        using parent::prefix_const_iterator::operator--;
        using parent::prefix_const_iterator::operator->;
    };

    class prefix_reverse_iterator : public parent::prefix_reverse_iterator
    {
    public:

        using value_type = parent::prefix_reverse_iterator::value_type;
        using difference_type = parent::prefix_reverse_iterator::difference_type;
        using pointer = parent::prefix_reverse_iterator::pointer;
        using reference = parent::prefix_reverse_iterator::reference;
        using iterator_category = parent::prefix_reverse_iterator::iterator_category;

        explicit prefix_reverse_iterator(parent::node* n = nullptr) noexcept;
        prefix_reverse_iterator(parent::prefix_reverse_iterator) noexcept;

        size_t get_height() const noexcept;
        size_t get_balance() const noexcept;

        prefix_reverse_iterator(prefix_iterator) noexcept;
        operator prefix_iterator() const noexcept;
        prefix_iterator base() const noexcept;

        using parent::prefix_reverse_iterator::depth;
        using parent::prefix_reverse_iterator::operator*;
        using parent::prefix_reverse_iterator::operator==;
        using parent::prefix_reverse_iterator::operator!=;
        using parent::prefix_reverse_iterator::operator++;
        using parent::prefix_reverse_iterator::operator--;
        using parent::prefix_reverse_iterator::operator->;
    };

    class prefix_const_reverse_iterator : public parent::prefix_const_reverse_iterator
    {
    public:

        using value_type = parent::prefix_const_reverse_iterator::value_type;
        using difference_type = parent::prefix_const_reverse_iterator::difference_type;
        using pointer = parent::prefix_const_reverse_iterator::pointer;
        using reference = parent::prefix_const_reverse_iterator::reference;
        using iterator_category = parent::prefix_const_reverse_iterator::iterator_category;

        explicit prefix_const_reverse_iterator(parent::node* n = nullptr) noexcept;
        prefix_const_reverse_iterator(parent::prefix_const_reverse_iterator) noexcept;

        size_t get_height() const noexcept;
        size_t get_balance() const noexcept;

        prefix_const_reverse_iterator(prefix_const_iterator) noexcept;
        operator prefix_const_iterator() const noexcept;
        prefix_const_iterator base() const noexcept;

        using parent::prefix_const_reverse_iterator::depth;
        using parent::prefix_const_reverse_iterator::operator*;
        using parent::prefix_const_reverse_iterator::operator==;
        using parent::prefix_const_reverse_iterator::operator!=;
        using parent::prefix_const_reverse_iterator::operator++;
        using parent::prefix_const_reverse_iterator::operator--;
        using parent::prefix_const_reverse_iterator::operator->;
    };

    class infix_iterator : public parent::infix_iterator
    {
        friend binary_search_tree<tkey, tvalue, compare, __detail::AVL_TAG>;
        friend class __detail::bst_impl<tkey, tvalue, compare, __detail::AVL_TAG>;
    public:

        using value_type = parent::infix_iterator::value_type;
        using difference_type = parent::infix_iterator::difference_type;
        using pointer = parent::infix_iterator::pointer;
        using reference = parent::infix_iterator::reference;
        using iterator_category = parent::infix_iterator::iterator_category;

        explicit infix_iterator(parent::node* n = nullptr) noexcept;
        infix_iterator(parent::infix_iterator) noexcept;

        size_t get_height() const noexcept;
        size_t get_balance() const noexcept;

        using parent::infix_iterator::depth;
        using parent::infix_iterator::operator*;
        using parent::infix_iterator::operator==;
        using parent::infix_iterator::operator!=;
        using parent::infix_iterator::operator++;
        using parent::infix_iterator::operator--;
        using parent::infix_iterator::operator->;
    };

    class infix_const_iterator : parent::infix_const_iterator
    {
    public:

        using value_type = parent::infix_const_iterator::value_type;
        using difference_type = parent::infix_const_iterator::difference_type;
        using pointer = parent::infix_const_iterator::pointer;
        using reference = parent::infix_const_iterator::reference;
        using iterator_category = parent::infix_const_iterator::iterator_category;

        explicit infix_const_iterator(parent::node* n = nullptr) noexcept;
        infix_const_iterator(parent::infix_const_iterator) noexcept;

        size_t get_height() const noexcept;
        size_t get_balance() const noexcept;

        infix_const_iterator(infix_iterator) noexcept;

        using parent::infix_const_iterator::depth;
        using parent::infix_const_iterator::operator*;
        using parent::infix_const_iterator::operator==;
        using parent::infix_const_iterator::operator!=;
        using parent::infix_const_iterator::operator++;
        using parent::infix_const_iterator::operator--;
        using parent::infix_const_iterator::operator->;
    };

    class infix_reverse_iterator : public parent::infix_reverse_iterator
    {
    public:

        using value_type = parent::infix_reverse_iterator::value_type;
        using difference_type = parent::infix_reverse_iterator::difference_type;
        using pointer = parent::infix_reverse_iterator::pointer;
        using reference = parent::infix_reverse_iterator::reference;
        using iterator_category = parent::infix_reverse_iterator::iterator_category;

        explicit infix_reverse_iterator(parent::node* n = nullptr) noexcept;
        infix_reverse_iterator(parent::infix_reverse_iterator) noexcept;

        size_t get_height() const noexcept;
        size_t get_balance() const noexcept;

        infix_reverse_iterator(infix_iterator) noexcept;
        operator infix_iterator() const noexcept;
        infix_iterator base() const noexcept;

        using parent::infix_reverse_iterator::depth;
        using parent::infix_reverse_iterator::operator*;
        using parent::infix_reverse_iterator::operator==;
        using parent::infix_reverse_iterator::operator!=;
        using parent::infix_reverse_iterator::operator++;
        using parent::infix_reverse_iterator::operator--;
        using parent::infix_reverse_iterator::operator->;
    };

    class infix_const_reverse_iterator : public parent::infix_const_reverse_iterator
    {
    public:

        using value_type = parent::infix_const_reverse_iterator::value_type;
        using difference_type = parent::infix_const_reverse_iterator::difference_type;
        using pointer = parent::infix_const_reverse_iterator::pointer;
        using reference = parent::infix_const_reverse_iterator::reference;
        using iterator_category = parent::infix_const_reverse_iterator::iterator_category;

        explicit infix_const_reverse_iterator(parent::node* n = nullptr) noexcept;
        infix_const_reverse_iterator(parent::infix_const_reverse_iterator) noexcept;

        size_t get_height() const noexcept;
        size_t get_balance() const noexcept;

        infix_const_reverse_iterator(infix_const_iterator) noexcept;
        operator infix_const_iterator() const noexcept;
        infix_const_iterator base() const noexcept;

        using parent::infix_const_reverse_iterator::depth;
        using parent::infix_const_reverse_iterator::operator*;
        using parent::infix_const_reverse_iterator::operator==;
        using parent::infix_const_reverse_iterator::operator!=;
        using parent::infix_const_reverse_iterator::operator++;
        using parent::infix_const_reverse_iterator::operator--;
        using parent::infix_const_reverse_iterator::operator->;
    };

    class postfix_iterator : public parent::postfix_iterator
    {
    public:

        using value_type = parent::postfix_iterator::value_type;
        using difference_type = parent::postfix_iterator::difference_type;
        using pointer = parent::postfix_iterator::pointer;
        using reference = parent::postfix_iterator::reference;
        using iterator_category = parent::postfix_iterator::iterator_category;

        explicit postfix_iterator(parent::node* n = nullptr) noexcept;
        postfix_iterator(parent::postfix_iterator) noexcept;

        size_t get_height() const noexcept;
        size_t get_balance() const noexcept;

        using parent::postfix_iterator::depth;
        using parent::postfix_iterator::operator*;
        using parent::postfix_iterator::operator==;
        using parent::postfix_iterator::operator!=;
        using parent::postfix_iterator::operator++;
        using parent::postfix_iterator::operator--;
        using parent::postfix_iterator::operator->;
    };

    class postfix_const_iterator : public parent::postfix_const_iterator
    {
    public:

        using value_type = parent::postfix_const_iterator::value_type;
        using difference_type = parent::postfix_const_iterator::difference_type;
        using pointer = parent::postfix_const_iterator::pointer;
        using reference = parent::postfix_const_iterator::reference;
        using iterator_category = parent::postfix_const_iterator::iterator_category;

        explicit postfix_const_iterator(parent::node* n = nullptr) noexcept;
        postfix_const_iterator(parent::postfix_const_iterator) noexcept;

        size_t get_height() const noexcept;
        size_t get_balance() const noexcept;

        postfix_const_iterator(postfix_iterator) noexcept;

        using parent::postfix_const_iterator::depth;
        using parent::postfix_const_iterator::operator*;
        using parent::postfix_const_iterator::operator==;
        using parent::postfix_const_iterator::operator!=;
        using parent::postfix_const_iterator::operator++;
        using parent::postfix_const_iterator::operator--;
        using parent::postfix_const_iterator::operator->;
    };

    class postfix_reverse_iterator : public parent::postfix_reverse_iterator
    {
    public:

        using value_type = parent::postfix_reverse_iterator::value_type;
        using difference_type = parent::postfix_reverse_iterator::difference_type;
        using pointer = parent::postfix_reverse_iterator::pointer;
        using reference = parent::postfix_reverse_iterator::reference;
        using iterator_category = parent::postfix_reverse_iterator::iterator_category;

        explicit postfix_reverse_iterator(parent::node* n = nullptr) noexcept;
        postfix_reverse_iterator(parent::postfix_reverse_iterator) noexcept;

        size_t get_height() const noexcept;
        size_t get_balance() const noexcept;

        postfix_reverse_iterator(postfix_iterator) noexcept;
        operator postfix_iterator() const noexcept;
        postfix_iterator base() const noexcept;

        using parent::postfix_reverse_iterator::depth;
        using parent::postfix_reverse_iterator::operator*;
        using parent::postfix_reverse_iterator::operator==;
        using parent::postfix_reverse_iterator::operator!=;
        using parent::postfix_reverse_iterator::operator++;
        using parent::postfix_reverse_iterator::operator--;
        using parent::postfix_reverse_iterator::operator->;
    };

    class postfix_const_reverse_iterator : public parent::postfix_const_reverse_iterator
    {
    public:

        using value_type = parent::postfix_const_reverse_iterator::value_type;
        using difference_type = parent::postfix_const_reverse_iterator::difference_type;
        using pointer = parent::postfix_const_reverse_iterator::pointer;
        using reference = parent::postfix_const_reverse_iterator::reference;
        using iterator_category = parent::postfix_const_reverse_iterator::iterator_category;

        explicit postfix_const_reverse_iterator(parent::node* n = nullptr) noexcept;
        postfix_const_reverse_iterator(parent::postfix_const_reverse_iterator) noexcept;

        size_t get_height() const noexcept;
        size_t get_balance() const noexcept;

        postfix_const_reverse_iterator(postfix_const_iterator) noexcept;
        operator postfix_const_iterator() const noexcept;
        postfix_const_iterator base() const noexcept;

        using parent::postfix_const_reverse_iterator::depth;
        using parent::postfix_const_reverse_iterator::operator*;
        using parent::postfix_const_reverse_iterator::operator==;
        using parent::postfix_const_reverse_iterator::operator!=;
        using parent::postfix_const_reverse_iterator::operator++;
        using parent::postfix_const_reverse_iterator::operator--;
        using parent::postfix_const_reverse_iterator::operator->;

    };

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
    


    explicit AVL_tree(
            const compare& comp = compare(),
            pp_allocator<value_type> alloc = pp_allocator<value_type>(),
            logger *logger = nullptr);

    explicit AVL_tree(
            pp_allocator<value_type> alloc,
            const compare& comp = compare(),
            logger *logger = nullptr);

    template<input_iterator_for_pair<tkey, tvalue> iterator>
    explicit AVL_tree(iterator begin, iterator end, const compare& cmp = compare(),
                                pp_allocator<value_type> alloc = pp_allocator<value_type>(),
                                logger* logger = nullptr);

    template<std::ranges::input_range Range>
    explicit AVL_tree(Range&& range, const compare& cmp = compare(),
            pp_allocator<value_type> alloc = pp_allocator<value_type>(),
            logger* logger = nullptr);


    AVL_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(),
            pp_allocator<value_type> alloc = pp_allocator<value_type>(),
            logger* logger = nullptr);

public:
    
    ~AVL_tree() noexcept final =default;
    
    AVL_tree(AVL_tree const &other);
    
    AVL_tree &operator=(AVL_tree const &other);
    
    AVL_tree(AVL_tree &&other) noexcept =default;
    
    AVL_tree &operator=(AVL_tree &&other) noexcept =default;

    void swap(parent& other) noexcept override;


    /** Only rebinds iterators
     */
    std::pair<infix_iterator, bool> insert(const value_type&);
    std::pair<infix_iterator, bool> insert(value_type&&);

    template<class ...Args>
    std::pair<infix_iterator, bool> emplace(Args&&...args);

    infix_iterator insert_or_assign(const value_type&);
    infix_iterator insert_or_assign(value_type&&);

    template<class ...Args>
    infix_iterator emplace_or_assign(Args&&...args);

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

    using parent::erase;
    using parent::insert;
    using parent::insert_or_assign;
};

template<typename compare, typename U, typename iterator>
explicit AVL_tree(iterator begin, iterator end, const compare& cmp = compare(),
                            pp_allocator<U> alloc = pp_allocator<U>(),
                            logger* logger = nullptr) -> AVL_tree<const typename std::iterator_traits<iterator>::value_type::first_type, typename std::iterator_traits<iterator>::value_type::second_type, compare>;

template<typename compare, typename U, std::ranges::forward_range Range>
explicit AVL_tree(Range&& range, const compare& cmp = compare(),
                            pp_allocator<U> alloc = pp_allocator<U>(),
                            logger* logger = nullptr) -> AVL_tree<const typename std::iterator_traits<typename std::ranges::iterator_t<Range>>::value_type::first_type, typename std::iterator_traits<typename std::ranges::iterator_t<Range>>::value_type::second_type, compare> ;

template<typename tkey, typename tvalue, typename compare, typename U>
AVL_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(),
                   pp_allocator<U> alloc = pp_allocator<U>(),
                   logger* logger = nullptr) -> AVL_tree<tkey, tvalue, compare>;

namespace __detail
{
    template<typename tkey, typename tvalue, typename compare>
    template<class ...Args>
    binary_search_tree<tkey, tvalue, compare, AVL_TAG>::node *bst_impl<tkey, tvalue, compare, AVL_TAG>::create_node(
            binary_search_tree <tkey, tvalue, compare, AVL_TAG> &cont, Args &&...args)
    {
        using node_type = typename AVL_tree<tkey, tvalue, compare>::node;
        auto *new_node = cont._allocator.template new_object<node_type>(std::forward<Args>(args)...);


        return new_node;
    }

    template<typename tkey, typename tvalue, typename compare>
    void bst_impl<tkey, tvalue, compare, AVL_TAG>::delete_node(
            binary_search_tree <tkey, tvalue, compare, AVL_TAG> &cont, binary_search_tree<tkey, tvalue, compare, AVL_TAG>::node** node)
    {
        using node_type = typename binary_search_tree<tkey, tvalue, compare, AVL_TAG>::node;
        if (node && *node)
        {
            cont._allocator.template delete_object<node_type>(*node);
            *node = nullptr;
        }
    }

    template<typename tkey, typename tvalue, typename compare>
    void bst_impl<tkey, tvalue, compare, AVL_TAG>::post_insert(
            binary_search_tree<tkey, tvalue, compare, AVL_TAG>& cont,
            typename binary_search_tree<tkey, tvalue, compare, AVL_TAG>::node** node) {
        if (node == nullptr || *node == nullptr)
            return;

        using avl_node = typename AVL_tree<tkey, tvalue, compare>::node;
        using node_type = typename binary_search_tree<tkey, tvalue, compare, AVL_TAG>::node;

        auto* current = static_cast<avl_node*>((*node)->parent);
        while (current != nullptr) {
            current->recalculate_height();
            short balance = current->get_balance();

            node_type *&subtree_ref = (current->parent == nullptr)
                ? cont._root
                : (current->parent->left_subtree == current
                    ? current->parent->left_subtree
                    : current->parent->right_subtree);

            if (balance > 1) {
                auto *right = static_cast<avl_node *>(current->right_subtree);
                if (right && right->get_balance() < 0) {
                    node_type *&right_ref = (right->parent->left_subtree == right)
                        ? right->parent->left_subtree
                        : right->parent->right_subtree;
                    cont.small_right_rotation(right_ref);
                    right->recalculate_height();
                }

                cont.small_left_rotation(subtree_ref);
                static_cast<avl_node *>(current)->recalculate_height();
                if (current->parent) {
                    static_cast<avl_node *>(current->parent)->recalculate_height();
                }

                current = static_cast<avl_node *>(subtree_ref);
            } else if (balance < -1) { // Left-heavy
                auto *left = static_cast<avl_node *>(current->left_subtree);
                if (left && left->get_balance() > 0) { // Left-Right case
                    node_type *&left_ref = (left->parent->left_subtree == left)
                        ? left->parent->left_subtree
                        : left->parent->right_subtree;
                    cont.small_left_rotation(left_ref);
                    left->recalculate_height();
                }

                cont.small_right_rotation(subtree_ref);
                static_cast<avl_node *>(current)->recalculate_height();
                if (current->parent) {
                    static_cast<avl_node *>(current->parent)->recalculate_height();
                }

                current = static_cast<avl_node *>(subtree_ref);
            }

            current = static_cast<avl_node *>(current->parent);
        }
    }


    template<typename tkey, typename tvalue, typename compare>
    void bst_impl<tkey, tvalue, compare, AVL_TAG>::erase(
    binary_search_tree<tkey, tvalue, compare, AVL_TAG> &cont,
    typename binary_search_tree<tkey, tvalue, compare, AVL_TAG>::node **node_ptr)
    {

        using node_type = typename binary_search_tree<tkey, tvalue, compare, AVL_TAG>::node;
        using avl_node = typename AVL_tree<tkey, tvalue, compare>::node;

        if (node_ptr == nullptr || *node_ptr == nullptr) return;

        node_type *node = *node_ptr;
        node_type *parent = node->parent;
        node_type *balance_start = parent;

        if (node->left_subtree == nullptr && node->right_subtree == nullptr) {
            if (parent) {
                if (parent->left_subtree == node) parent->left_subtree = nullptr;
                else parent->right_subtree = nullptr;
            } else {
                cont._root = nullptr;
            }
            delete_node(cont, node_ptr);
        }
        else if (node->left_subtree == nullptr || node->right_subtree == nullptr) {

            node_type *child = node->left_subtree ? node->left_subtree : node->right_subtree;
            child->parent = parent;

            if (parent) {
                if (parent->left_subtree == node) parent->left_subtree = child;
                else parent->right_subtree = child;
            } else {
                cont._root = child;
            }
            balance_start = child;
            delete_node(cont, node_ptr);
        }
        else {
            node_type *predecessor = node->left_subtree;
            while (predecessor->right_subtree) {
                predecessor = predecessor->right_subtree;
            }

            balance_start = predecessor;

            if (predecessor->parent != node) {
                predecessor->parent->right_subtree = predecessor->left_subtree;
                if (predecessor->left_subtree) {
                    predecessor->left_subtree->parent = predecessor->parent;
                }
                balance_start = predecessor->parent; // Обновляем точку начала
            }

            predecessor->left_subtree = (node->left_subtree == predecessor) ?
                                      predecessor->left_subtree : node->left_subtree;
            if (predecessor->left_subtree) {
                predecessor->left_subtree->parent = predecessor;
            }

            predecessor->right_subtree = node->right_subtree;
            if (predecessor->right_subtree) {
                predecessor->right_subtree->parent = predecessor;
            }

            predecessor->parent = node->parent;

            if (node->parent) {
                if (node->parent->left_subtree == node) {
                    node->parent->left_subtree = predecessor;
                } else {
                    node->parent->right_subtree = predecessor;
                }
            } else {
                cont._root = predecessor;
            }

            delete_node(cont, node_ptr);
        }

        --cont._size;
        if (balance_start) {
            auto *current = static_cast<avl_node *>(balance_start);

            while (current != nullptr) {
                current->recalculate_height();
                short balance = current->get_balance();

                node_type *&subtree_ref = (current->parent == nullptr)
                    ? cont._root
                    : (current->parent->left_subtree == current
                        ? current->parent->left_subtree
                        : current->parent->right_subtree);

                if (balance > 1) {
                    auto *right = static_cast<avl_node *>(current->right_subtree);
                    if (right && right->get_balance() < 0) {
                        node_type *&right_ref = (right->parent->left_subtree == right)
                            ? right->parent->left_subtree
                            : right->parent->right_subtree;
                        cont.small_right_rotation(right_ref);
                        right->recalculate_height();
                    }

                    cont.small_left_rotation(subtree_ref);
                    static_cast<avl_node *>(current)->recalculate_height();
                    if (current->parent) {
                        static_cast<avl_node *>(current->parent)->recalculate_height();
                    }

                    current = static_cast<avl_node *>(subtree_ref);
                } else if (balance < -1) {
                    auto *left = static_cast<avl_node *>(current->left_subtree);
                    if (left && left->get_balance() > 0) {
                        node_type *&left_ref = (left->parent->left_subtree == left)
                            ? left->parent->left_subtree
                            : left->parent->right_subtree;
                        cont.small_left_rotation(left_ref);
                        left->recalculate_height();
                    }

                    cont.small_right_rotation(subtree_ref);
                    static_cast<avl_node *>(current)->recalculate_height();
                    if (current->parent) {
                        static_cast<avl_node *>(current->parent)->recalculate_height();
                    }

                    current = static_cast<avl_node *>(subtree_ref);
                }

                current = static_cast<avl_node *>(current->parent);
            }
        }
    }


}

template<typename tkey, typename tvalue, typename compare>
void __detail::bst_impl<tkey, tvalue, compare, __detail::AVL_TAG>::swap(binary_search_tree<tkey, tvalue, compare, AVL_TAG> &lhs,
                                                          binary_search_tree<tkey, tvalue, compare, AVL_TAG> &rhs) noexcept
{
    using std::swap;
    swap(lhs.root, rhs.root);
    swap(lhs._logger, rhs._logger);
    swap(lhs._size, rhs._size);
    swap(lhs._allocator, rhs._allocator);
}



template<typename tkey, typename tvalue, compator<tkey> compare>
void AVL_tree<tkey, tvalue, compare>::node::recalculate_height() noexcept
{
    const size_t left_height = this->left_subtree ? static_cast<node*>(this->left_subtree)->height : 0;
    const size_t right_height = this->right_subtree ? static_cast<node*>(this->right_subtree)->height : 0;
    this->height = 1 + std::max(left_height, right_height);
}

template<typename tkey, typename tvalue, compator<tkey> compare>
short AVL_tree<tkey, tvalue, compare>::node::get_balance() const noexcept
{
    const size_t left_height = this->left_subtree ? static_cast<node *>(this->left_subtree)->height : 0;
    const size_t right_height = this->right_subtree ? static_cast<node *>(this->right_subtree)->height : 0;
    return static_cast<short>(right_height - left_height);
}

template<typename tkey, typename tvalue, compator<tkey> compare>
template<class ...Args>
AVL_tree<tkey, tvalue, compare>::node::node(parent::node* par, Args&&... args)
    : parent::node(par, std::forward<Args>(args)...), height(1)
{
}





template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::prefix_iterator::prefix_iterator(parent::node* n) noexcept : parent::prefix_iterator(n)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::prefix_iterator::prefix_iterator(parent::prefix_iterator it) noexcept : parent::prefix_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
size_t AVL_tree<tkey, tvalue, compare>::prefix_iterator::get_height() const noexcept
{
    return static_cast<node*>(this->_data)->height;
}

template<typename tkey, typename tvalue, compator<tkey> compare>
size_t AVL_tree<tkey, tvalue, compare>::prefix_iterator::get_balance() const noexcept
{
    return static_cast<node*>(this->_data)->get_balance();
}





template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::prefix_const_iterator::prefix_const_iterator(parent::node* n) noexcept : parent::prefix_const_iterator(n)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::prefix_const_iterator::prefix_const_iterator(parent::prefix_const_iterator it) noexcept : parent::prefix_const_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::prefix_const_iterator::prefix_const_iterator(prefix_iterator it) noexcept : parent::prefix_const_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
size_t AVL_tree<tkey, tvalue, compare>::prefix_const_iterator::get_height() const noexcept
{
    return prefix_iterator(this->_base).get_height();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
size_t AVL_tree<tkey, tvalue, compare>::prefix_const_iterator::get_balance() const noexcept
{
    return prefix_iterator(this->_base)->get_balance();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::prefix_reverse_iterator::prefix_reverse_iterator(parent::node* n) noexcept : parent::prefix_reverse_iterator(n)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::prefix_reverse_iterator::prefix_reverse_iterator(parent::prefix_reverse_iterator it) noexcept : parent::prefix_reverse_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
size_t AVL_tree<tkey, tvalue, compare>::prefix_reverse_iterator::get_height() const noexcept
{
    return prefix_iterator(this->_base)->height;
}

template<typename tkey, typename tvalue, compator<tkey> compare>
size_t AVL_tree<tkey, tvalue, compare>::prefix_reverse_iterator::get_balance() const noexcept
{
    return prefix_iterator(this->_base)->get_balance();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::prefix_reverse_iterator::prefix_reverse_iterator(prefix_iterator it) noexcept : parent::prefix_reverse_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::prefix_reverse_iterator::operator AVL_tree<tkey, tvalue, compare>::prefix_iterator() const noexcept
{
    return parent::prefix_reverse_iterator::operator prefix_iterator();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::prefix_iterator
AVL_tree<tkey, tvalue, compare>::prefix_reverse_iterator::base() const noexcept
{
    return parent::prefix_reverse_iterator::base();
}


template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::prefix_const_reverse_iterator::prefix_const_reverse_iterator(parent::node* n) noexcept : parent::prefix_const_reverse_iterator(n)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::prefix_const_reverse_iterator::prefix_const_reverse_iterator(parent::prefix_const_reverse_iterator it) noexcept : parent::prefix_const_reverse_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
size_t AVL_tree<tkey, tvalue, compare>::prefix_const_reverse_iterator::get_height() const noexcept
{
    return prefix_iterator(this->_base)->height;
}

template<typename tkey, typename tvalue, compator<tkey> compare>
size_t AVL_tree<tkey, tvalue, compare>::prefix_const_reverse_iterator::get_balance() const noexcept
{
    return prefix_iterator(this->_base)->get_balance();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::prefix_const_reverse_iterator::prefix_const_reverse_iterator(prefix_const_iterator it) noexcept : parent::prefix_const_reverse_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::prefix_const_reverse_iterator::operator AVL_tree<tkey, tvalue, compare>::prefix_const_iterator() const noexcept
{
    return parent::prefix_const_reverse_iterator::operator prefix_const_iterator();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::prefix_const_iterator
AVL_tree<tkey, tvalue, compare>::prefix_const_reverse_iterator::base() const noexcept
{
    return parent::prefix_const_reverse_iterator::base();
}


template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::infix_iterator::infix_iterator(parent::node* n) noexcept : parent::infix_iterator(n)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::infix_iterator::infix_iterator(parent::infix_iterator it) noexcept : parent::infix_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
size_t AVL_tree<tkey, tvalue, compare>::infix_iterator::get_height() const noexcept
{
    return static_cast<node*>(this->_data)->height;
}

template<typename tkey, typename tvalue, compator<tkey> compare>
size_t AVL_tree<tkey, tvalue, compare>::infix_iterator::get_balance() const noexcept
{
    return static_cast<node*>(this->_data)->get_balance();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::infix_const_iterator::infix_const_iterator(parent::node* n) noexcept : parent::infix_const_iterator(n)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::infix_const_iterator::infix_const_iterator(parent::infix_const_iterator it) noexcept : parent::infix_const_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
size_t AVL_tree<tkey, tvalue, compare>::infix_const_iterator::get_height() const noexcept
{
    return infix_iterator(this->_base).get_height();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
size_t AVL_tree<tkey, tvalue, compare>::infix_const_iterator::get_balance() const noexcept
{
    return infix_iterator(this->_base)->get_balance();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::infix_const_iterator::infix_const_iterator(infix_iterator it) noexcept : parent::infix_const_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::infix_reverse_iterator::infix_reverse_iterator(parent::node* n) noexcept : parent::infix_reverse_iterator(n)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::infix_reverse_iterator::infix_reverse_iterator(parent::infix_reverse_iterator it) noexcept : parent::infix_reverse_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
size_t AVL_tree<tkey, tvalue, compare>::infix_reverse_iterator::get_height() const noexcept
{
    return infix_iterator(this->_base)->get_height();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
size_t AVL_tree<tkey, tvalue, compare>::infix_reverse_iterator::get_balance() const noexcept
{
    return infix_iterator(this->_base)->get_balance();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::infix_reverse_iterator::infix_reverse_iterator(infix_iterator it) noexcept : parent::infix_reverse_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::infix_reverse_iterator::operator AVL_tree<tkey, tvalue, compare>::infix_iterator() const noexcept
{
    return parent::infix_reverse_iterator::operator infix_iterator();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_iterator AVL_tree<tkey, tvalue, compare>::infix_reverse_iterator::base() const noexcept
{
    return parent::infix_reverse_iterator::base();
}


template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::infix_const_reverse_iterator::infix_const_reverse_iterator(parent::node* n) noexcept : parent::infix_const_reverse_iterator(n)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::infix_const_reverse_iterator::infix_const_reverse_iterator(parent::infix_const_reverse_iterator it) noexcept : parent::infix_const_reverse_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
size_t AVL_tree<tkey, tvalue, compare>::infix_const_reverse_iterator::get_height() const noexcept
{
    return infix_iterator(this->_base)->get_height();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
size_t AVL_tree<tkey, tvalue, compare>::infix_const_reverse_iterator::get_balance() const noexcept
{
    return infix_iterator(this->_base)->get_balance();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::infix_const_reverse_iterator::infix_const_reverse_iterator(infix_const_iterator it) noexcept : parent::infix_const_reverse_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::infix_const_reverse_iterator::operator AVL_tree<tkey, tvalue, compare>::infix_const_iterator() const noexcept
{
    return parent::infix_const_reverse_iterator::operator infix_const_iterator();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_const_iterator AVL_tree<tkey, tvalue, compare>::infix_const_reverse_iterator::base() const noexcept
{
    return parent::infix_const_reverse_iterator::base();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::postfix_iterator::postfix_iterator(parent::node* n) noexcept : parent::postfix_iterator(n)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::postfix_iterator::postfix_iterator(parent::postfix_iterator it) noexcept : parent::postfix_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
size_t AVL_tree<tkey, tvalue, compare>::postfix_iterator::get_height() const noexcept
{
    return static_cast<node*>(this->_data)->height;
}

template<typename tkey, typename tvalue, compator<tkey> compare>
size_t AVL_tree<tkey, tvalue, compare>::postfix_iterator::get_balance() const noexcept
{
    return static_cast<node*>(this->_data)->get_balance();
}


template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::postfix_const_iterator::postfix_const_iterator(parent::node* n) noexcept : parent::postfix_const_iterator(n)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::postfix_const_iterator::postfix_const_iterator(parent::postfix_const_iterator it) noexcept : parent::postfix_const_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
size_t AVL_tree<tkey, tvalue, compare>::postfix_const_iterator::get_height() const noexcept
{
    return postfix_iterator(this->_base).get_height();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
size_t AVL_tree<tkey, tvalue, compare>::postfix_const_iterator::get_balance() const noexcept
{
    return postfix_iterator(this->_base)->get_balance();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::postfix_const_iterator::postfix_const_iterator(postfix_iterator it) noexcept : parent::postfix_const_iterator(it)
{
}


template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::postfix_reverse_iterator::postfix_reverse_iterator(parent::node* n) noexcept : parent::postfix_reverse_iterator(n)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::postfix_reverse_iterator::postfix_reverse_iterator(parent::postfix_reverse_iterator it) noexcept : parent::postfix_reverse_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
size_t AVL_tree<tkey, tvalue, compare>::postfix_reverse_iterator::get_height() const noexcept
{
    return postfix_iterator(this->_base).get_height();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
size_t AVL_tree<tkey, tvalue, compare>::postfix_reverse_iterator::get_balance() const noexcept
{
    return postfix_iterator(this->_base).get_balance();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::postfix_reverse_iterator::postfix_reverse_iterator(postfix_iterator it) noexcept : parent::postfix_reverse_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::postfix_reverse_iterator::operator AVL_tree<tkey, tvalue, compare>::postfix_iterator() const noexcept
{
    return parent::postfix_reverse_iterator::operator postfix_iterator();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::postfix_iterator AVL_tree<tkey, tvalue, compare>::postfix_reverse_iterator::base() const noexcept
{
    return parent::postfix_reverse_iterator::base();
}


template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::postfix_const_reverse_iterator::postfix_const_reverse_iterator(parent::node* n) noexcept : parent::postfix_reverse_iterator(n)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::postfix_const_reverse_iterator::postfix_const_reverse_iterator(parent::postfix_const_reverse_iterator it) noexcept : parent::postfix_reverse_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
size_t AVL_tree<tkey, tvalue, compare>::postfix_const_reverse_iterator::get_height() const noexcept
{
    return postfix_iterator(this->_base).get_height();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
size_t AVL_tree<tkey, tvalue, compare>::postfix_const_reverse_iterator::get_balance() const noexcept
{
    return postfix_iterator(this->_base).get_balance();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::postfix_const_reverse_iterator::postfix_const_reverse_iterator(postfix_const_iterator it) noexcept : parent::postfix_const_reverse_iterator(it)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::postfix_const_reverse_iterator::operator AVL_tree<tkey, tvalue, compare>::postfix_const_iterator() const noexcept
{
    return parent::postfix_const_reverse_iterator::operator postfix_const_iterator();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::postfix_const_iterator AVL_tree<tkey, tvalue, compare>::postfix_const_reverse_iterator::base() const noexcept
{
    return parent::postfix_const_reverse_iterator::base();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_iterator AVL_tree<tkey, tvalue, compare>::begin() noexcept
{
    return parent::begin();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_iterator AVL_tree<tkey, tvalue, compare>::end() noexcept
{
    return parent::end();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_const_iterator AVL_tree<tkey, tvalue, compare>::begin() const noexcept
{
    return parent::begin();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_const_iterator AVL_tree<tkey, tvalue, compare>::end() const noexcept
{
    return parent::end();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_const_iterator AVL_tree<tkey, tvalue, compare>::cbegin() const noexcept
{
    return parent::cbegin();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_const_iterator AVL_tree<tkey, tvalue, compare>::cend() const noexcept
{
    return parent::cend();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_reverse_iterator AVL_tree<tkey, tvalue, compare>::rbegin() noexcept
{
    return parent::rbegin();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_reverse_iterator AVL_tree<tkey, tvalue, compare>::rend() noexcept
{
    return parent::rend();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_const_reverse_iterator AVL_tree<tkey, tvalue, compare>::rbegin() const noexcept
{
    return parent::rbegin();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_const_reverse_iterator AVL_tree<tkey, tvalue, compare>::rend() const noexcept
{
    return parent::rend();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_const_reverse_iterator AVL_tree<tkey, tvalue, compare>::crbegin() const noexcept
{
    return parent::crbegin();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_const_reverse_iterator AVL_tree<tkey, tvalue, compare>::crend() const noexcept
{
    return parent::crend();
}


template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::prefix_iterator AVL_tree<tkey, tvalue, compare>::begin_prefix() noexcept
{
    return parent::begin_prefix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::prefix_iterator AVL_tree<tkey, tvalue, compare>::end_prefix() noexcept
{
    return parent::end_prefix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::prefix_const_iterator AVL_tree<tkey, tvalue, compare>::begin_prefix() const noexcept
{
    return parent::begin_prefix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::prefix_const_iterator AVL_tree<tkey, tvalue, compare>::end_prefix() const noexcept
{
    return parent::end_prefix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::prefix_const_iterator AVL_tree<tkey, tvalue, compare>::cbegin_prefix() const noexcept
{
    return parent::cbegin_prefix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::prefix_const_iterator AVL_tree<tkey, tvalue, compare>::cend_prefix() const noexcept
{
    return parent::cend_prefix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::prefix_reverse_iterator AVL_tree<tkey, tvalue, compare>::rbegin_prefix() noexcept
{
    return parent::rbegin_prefix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::prefix_reverse_iterator AVL_tree<tkey, tvalue, compare>::rend_prefix() noexcept
{
    return parent::rend_prefix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::prefix_const_reverse_iterator AVL_tree<tkey, tvalue, compare>::rbegin_prefix() const noexcept
{
    return parent::rbegin_prefix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::prefix_const_reverse_iterator AVL_tree<tkey, tvalue, compare>::rend_prefix() const noexcept
{
    return parent::rend_prefix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::prefix_const_reverse_iterator AVL_tree<tkey, tvalue, compare>::crbegin_prefix() const noexcept
{
    return parent::crbegin_prefix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::prefix_const_reverse_iterator AVL_tree<tkey, tvalue, compare>::crend_prefix() const noexcept
{
    return parent::crend_prefix();
}

// region infix iterators
template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_iterator AVL_tree<tkey, tvalue, compare>::begin_infix() noexcept
{
    return parent::begin_infix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_iterator AVL_tree<tkey, tvalue, compare>::end_infix() noexcept
{
    return parent::end_infix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_const_iterator AVL_tree<tkey, tvalue, compare>::begin_infix() const noexcept
{
    return parent::begin_infix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_const_iterator AVL_tree<tkey, tvalue, compare>::end_infix() const noexcept
{
    return parent::end_infix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_const_iterator AVL_tree<tkey, tvalue, compare>::cbegin_infix() const noexcept
{
    return parent::cbegin_infix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_const_iterator AVL_tree<tkey, tvalue, compare>::cend_infix() const noexcept
{
    return parent::cend_infix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_reverse_iterator AVL_tree<tkey, tvalue, compare>::rbegin_infix() noexcept
{
    return parent::rbegin_infix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_reverse_iterator AVL_tree<tkey, tvalue, compare>::rend_infix() noexcept
{
    return parent::rend_infix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_const_reverse_iterator AVL_tree<tkey, tvalue, compare>::rbegin_infix() const noexcept
{
    return parent::rbegin_infix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_const_reverse_iterator AVL_tree<tkey, tvalue, compare>::rend_infix() const noexcept
{
    return parent::rend_infix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_const_reverse_iterator AVL_tree<tkey, tvalue, compare>::crbegin_infix() const noexcept
{
    return parent::crbegin_infix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_const_reverse_iterator AVL_tree<tkey, tvalue, compare>::crend_infix() const noexcept
{
    return parent::crend_infix();
}

// region postfix iterators
template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::postfix_iterator AVL_tree<tkey, tvalue, compare>::begin_postfix() noexcept
{
    return parent::begin_postfix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::postfix_iterator AVL_tree<tkey, tvalue, compare>::end_postfix() noexcept
{
    return parent::end_postfix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::postfix_const_iterator AVL_tree<tkey, tvalue, compare>::begin_postfix() const noexcept
{
    return parent::begin_postfix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::postfix_const_iterator AVL_tree<tkey, tvalue, compare>::end_postfix() const noexcept
{
    return parent::end_postfix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::postfix_const_iterator AVL_tree<tkey, tvalue, compare>::cbegin_postfix() const noexcept
{
    return parent::cbegin_postfix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::postfix_const_iterator AVL_tree<tkey, tvalue, compare>::cend_postfix() const noexcept
{
    return parent::cend_postfix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::postfix_reverse_iterator AVL_tree<tkey, tvalue, compare>::rbegin_postfix() noexcept
{
    return parent::rbegin_postfix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::postfix_reverse_iterator AVL_tree<tkey, tvalue, compare>::rend_postfix() noexcept
{
    return parent::rend_postfix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::postfix_const_reverse_iterator AVL_tree<tkey, tvalue, compare>::rbegin_postfix() const noexcept
{
    return parent::rbegin_postfix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::postfix_const_reverse_iterator AVL_tree<tkey, tvalue, compare>::rend_postfix() const noexcept
{
    return parent::rend_postfix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::postfix_const_reverse_iterator AVL_tree<tkey, tvalue, compare>::crbegin_postfix() const noexcept
{
    return parent::crbegin_postfix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::postfix_const_reverse_iterator AVL_tree<tkey, tvalue, compare>::crend_postfix() const noexcept
{
    return parent::crend_postfix();
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::AVL_tree(
        const compare& comp,
        pp_allocator<value_type> alloc,
        logger* logger) : parent(comp, alloc, logger)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::AVL_tree(
        pp_allocator<value_type> alloc,
        const compare& comp,
        logger* logger) : parent(comp, alloc, logger)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
template<input_iterator_for_pair<tkey, tvalue> iterator>
AVL_tree<tkey, tvalue, compare>::AVL_tree(
        iterator begin, iterator end,
        const compare& cmp,
        pp_allocator<value_type> alloc,
        logger* logger) : parent(cmp, alloc, logger)
{
    for (auto it = begin; it != end; ++it)
    {
        this->emplace(*it);
    }
}

template<typename tkey, typename tvalue, compator<tkey> compare>
template<std::ranges::input_range Range>
AVL_tree<tkey, tvalue, compare>::AVL_tree(
        Range&& range,
        const compare& cmp,
        pp_allocator<value_type> alloc,
        logger* logger) : parent(range, cmp, alloc, logger)
{
    for (auto &&element : range)
    {
        emplace(std::forward<decltype(element)>(element));
    }
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::AVL_tree(std::initializer_list<std::pair<tkey, tvalue>> data,
                                          const compare& cmp, pp_allocator<value_type> alloc,
                                          logger* logger) : parent(cmp, alloc, logger)
{
    for (const auto &element : data)
    {
        emplace(element.first, element.second);
    }
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>::AVL_tree(const AVL_tree& other) : parent(other)
{
}

template<typename tkey, typename tvalue, compator<tkey> compare>
AVL_tree<tkey, tvalue, compare>& AVL_tree<tkey, tvalue, compare>::operator=(const AVL_tree& other)
{
    if (this != &other) {
        parent::operator=(other);
    }
    return *this;
}

template<typename tkey, typename tvalue, compator<tkey> compare>
void AVL_tree<tkey, tvalue, compare>::swap(parent& other) noexcept
{
    if (this != &other)
    {
        parent::swap(other);
    }
}


template<typename tkey, typename tvalue, compator<tkey> compare>
std::pair<typename AVL_tree<tkey, tvalue, compare>::infix_iterator, bool>
AVL_tree<tkey, tvalue, compare>::insert(const value_type& value)
{
    auto result = parent::insert(value);
    return result;
}

template<typename tkey, typename tvalue, compator<tkey> compare>
std::pair<typename AVL_tree<tkey, tvalue, compare>::infix_iterator, bool>
AVL_tree<tkey, tvalue, compare>::insert(value_type&& value)
{
    auto result = parent::insert(std::move(value));
    return result;
}

template<typename tkey, typename tvalue, compator<tkey> compare>
template<class ...Args>
std::pair<typename AVL_tree<tkey, tvalue, compare>::infix_iterator, bool>
AVL_tree<tkey, tvalue, compare>::emplace(Args&&... args)
{
    return parent::emplace(std::forward<Args>(args)...);
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_iterator
AVL_tree<tkey, tvalue, compare>::insert_or_assign(const value_type& value)
{
    auto result = parent::insert_or_assign(value);
    return result;
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_iterator
AVL_tree<tkey, tvalue, compare>::insert_or_assign(value_type&& value)
{
    auto result = parent::insert_or_assign(std::move(value));
    return result;
}

template<typename tkey, typename tvalue, compator<tkey> compare>
template<class ...Args>
typename AVL_tree<tkey, tvalue, compare>::infix_iterator
AVL_tree<tkey, tvalue, compare>::emplace_or_assign(Args&&... args)
{
    auto it = binary_search_tree<tkey, tvalue, compare, __detail::AVL_TAG>::emplace_or_assign(std::forward<Args>(args)...);
    return it;
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_iterator
AVL_tree<tkey, tvalue, compare>::find(const tkey& key)
{
    return parent::find(key);
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_const_iterator
AVL_tree<tkey, tvalue, compare>::find(const tkey& key) const
{
    return parent::find(key);
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_iterator
AVL_tree<tkey, tvalue, compare>::lower_bound(const tkey& key)
{
    return parent::lower_bound(key);
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_const_iterator
AVL_tree<tkey, tvalue, compare>::lower_bound(const tkey& key) const
{
    return parent::lower_bound(key);
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_iterator
AVL_tree<tkey, tvalue, compare>::upper_bound(const tkey& key)
{
    return parent::upper_bound(key);
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_const_iterator
AVL_tree<tkey, tvalue, compare>::upper_bound(const tkey& key) const
{
    return parent::upper_bound(key);
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_iterator
AVL_tree<tkey, tvalue, compare>::erase(infix_iterator pos)
{
    return parent::erase(pos);
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_iterator
AVL_tree<tkey, tvalue, compare>::erase(infix_const_iterator pos)
{
    return parent::erase(pos);
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_iterator
AVL_tree<tkey, tvalue, compare>::erase(infix_iterator first, infix_iterator last)
{
    return parent::erase(first, last);
}

template<typename tkey, typename tvalue, compator<tkey> compare>
typename AVL_tree<tkey, tvalue, compare>::infix_iterator
AVL_tree<tkey, tvalue, compare>::erase(infix_const_iterator first, infix_const_iterator last)
{
    return parent::erase(first, last);
}


#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_AVL_TREE_H