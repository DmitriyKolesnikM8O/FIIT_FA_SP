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

    struct block_metadata
    {
     
        size_t block_size_;
        block_metadata* next_ = nullptr;
        block_metadata* prev_ = nullptr;
     
        void* tm_ptr_;

        std::byte* block_end() noexcept
        {
            return reinterpret_cast<std::byte*>(this) + sizeof(block_metadata) + block_size_;
        }

        const std::byte* block_end() const noexcept
        {
            return reinterpret_cast<const std::byte*>(this) + sizeof(block_metadata) + block_size_;
        }
    };

    struct allocator_metadata
    {
        logger* logger_;
     
     
        fit_mode fit_mode_;
     
        size_t mem_size_;
     
        std::mutex mutex_;
     
        block_metadata* first_block_;
     
        memory_resource* allocator_;

        const std::byte* allocator_end() const noexcept
        {
            return reinterpret_cast<const std::byte*>(this) + sizeof(allocator_metadata) + mem_size_;
        }
    };

    static constexpr const size_t occupied_block_metadata_size = sizeof(size_t) + sizeof(void*) + sizeof(void*) + sizeof(void*);
    void *_trusted_memory;

public:

    ~allocator_boundary_tags() override;

    allocator_boundary_tags(allocator_boundary_tags const &other) = delete;

    allocator_boundary_tags &operator=(allocator_boundary_tags const &other) = delete;

    

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

public:
    inline void set_first_block(void *block);
    inline void set_fit_mode(
        allocator_with_fit_mode::fit_mode mode) override;

public:

    std::vector<allocator_test_utils::block_info> get_blocks_info() const override;

private:

    std::vector<allocator_test_utils::block_info> get_blocks_info_inner() const override;



    inline logger *get_logger() const override;

    inline std::string get_typename() const noexcept override;

    inline allocator_metadata& get_allocator_metadata() const noexcept;

    static inline allocator_metadata& get_allocator_metadata(void* trusted) noexcept;

    static inline const allocator_metadata& get_allocator_metadata(const void* trusted) noexcept;

    inline block_metadata* get_block_first_fit(size_t size) const noexcept;

    inline block_metadata* get_block_best_fit(size_t size) const noexcept;

    inline block_metadata* get_block_worst_fit(size_t size) const noexcept;

    inline size_t get_next_free_block_size(const block_metadata* block) const noexcept;

    static inline size_t get_next_free_block_size(void* trusted, const block_metadata* block) noexcept;

    inline size_t get_available_memory() const noexcept;

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