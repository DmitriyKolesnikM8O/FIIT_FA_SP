#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BOUNDARY_TAGS_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BOUNDARY_TAGS_H

#include <allocator_test_utils.h>
#include <allocator_with_fit_mode.h>
#include <pp_allocator.h>
#include <logger_guardant.h>
#include <typename_holder.h>
#include <iterator>
#include <mutex>

class allocator_boundary_tags final :
    public smart_mem_resource,
    public allocator_test_utils,
    public allocator_with_fit_mode,
    private logger_guardant,
    private typename_holder
{

private:

    /**
     * TODO: You must improve it for alignment support
     */
    static constexpr const size_t allocator_metadata_size = sizeof(logger*) + sizeof(memory_resource*) + sizeof(allocator_with_fit_mode::fit_mode) +
                                                            sizeof(size_t) + sizeof(std::mutex) + sizeof(void*);

    static constexpr const size_t occupied_block_metadata_size = sizeof(size_t) + sizeof(void*) + sizeof(void*) + sizeof(void*);

    static constexpr const size_t free_block_metadata_size = 0;

    void *_trusted_memory;

    logger* _logger = nullptr;

    // Member mutex for synchronization instead of global mutex
    mutable std::mutex _mutex;

    // Instance member for fit mode instead of static variable
    allocator_with_fit_mode::fit_mode _current_fit_mode = allocator_with_fit_mode::fit_mode::first_fit;

public:

    ~allocator_boundary_tags() override;

    allocator_boundary_tags(allocator_boundary_tags const &other);

    allocator_boundary_tags &operator=(allocator_boundary_tags const &other);

    allocator_boundary_tags(
        allocator_boundary_tags &&other) noexcept;

    allocator_boundary_tags &operator=(
        allocator_boundary_tags &&other) noexcept;

public:

    explicit allocator_boundary_tags(
            size_t space_size,
            std::pmr::memory_resource *parent_allocator = nullptr,
            logger *logger = nullptr,
            allocator_with_fit_mode::fit_mode allocate_fit_mode = allocator_with_fit_mode::fit_mode::first_fit);

public:

    [[nodiscard]] void *do_allocate_sm(
        size_t bytes) override;

    void do_deallocate_sm(
        void *at) override;

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override;

private:
    // Helper method for allocating empty blocks (size 0)
    [[nodiscard]] void *do_allocate_empty_block();

public:

    inline void set_fit_mode(
        allocator_with_fit_mode::fit_mode mode) override;

    inline allocator_with_fit_mode::fit_mode get_fit_mode() const;

public:

    std::vector<allocator_test_utils::block_info> get_blocks_info() const override;

private:

    std::vector<allocator_test_utils::block_info> get_blocks_info_inner() const override;

    // Вспомогательная функция для вывода информации о блоках памяти
    inline void log_blocks_state() const;

/** TODO: Highly recommended for helper functions to return references */

    inline logger *get_logger() const override;

    inline std::string get_typename() const noexcept override;

    const std::mutex& mutex() const;

    void set_logger(logger* log);

    class boundary_iterator
    {
        void* _occupied_ptr;
        bool _occupied;
        void* _trusted_memory;

    public:

        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = void*;
        using reference = void*&;
        using pointer = void**;
        using difference_type = ptrdiff_t;

        bool operator==(const boundary_iterator&) const noexcept;

        bool operator!=(const boundary_iterator&) const noexcept;

        boundary_iterator& operator++() & noexcept;

        boundary_iterator& operator--() & noexcept;

        boundary_iterator operator++(int n);

        boundary_iterator operator--(int n);

        size_t size() const noexcept;

        bool occupied() const noexcept;

        void* operator*() const noexcept;

        void* get_ptr() const noexcept;

        boundary_iterator();

        boundary_iterator(void* trusted);
    };

    friend class boundary_iterator;

    boundary_iterator begin() const noexcept;

    boundary_iterator end() const noexcept;
};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BOUNDARY_TAGS_H