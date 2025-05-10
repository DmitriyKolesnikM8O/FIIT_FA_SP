#include <not_implemented.h>
#include "../include/allocator_boundary_tags.h"
#include <format>

allocator_boundary_tags::~allocator_boundary_tags()
{
    auto& metadata = get_allocator_metadata();
    metadata.mutex_.~mutex();
    metadata.allocator_->deallocate(_trusted_memory, sizeof(allocator_metadata) + metadata.mem_size_, 1);
}

allocator_boundary_tags::allocator_boundary_tags(
    allocator_boundary_tags &&other) noexcept
{
    _trusted_memory = std::exchange(other._trusted_memory, nullptr);
}

allocator_boundary_tags &allocator_boundary_tags::operator=(
    allocator_boundary_tags &&other) noexcept
{
    if (this != &other)
    {
        std::swap(_trusted_memory, other._trusted_memory);
    }
    return *this;
}


allocator_boundary_tags::allocator_boundary_tags(
        size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        logger *logger,
        allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    if (space_size < sizeof(block_metadata))
    {
        throw std::logic_error("`space_size` is not enough to fit a single block");
    }

    const auto allocator = parent_allocator != nullptr ? parent_allocator : std::pmr::get_default_resource();

    _trusted_memory = allocator->allocate(sizeof(allocator_metadata) + space_size, 1);

    const auto metadata = static_cast<allocator_metadata*>(_trusted_memory);

    metadata->logger_ = logger;
    metadata->fit_mode_ = allocate_fit_mode;
    metadata->mem_size_ = space_size;
    metadata->first_block_ = nullptr;
    metadata->allocator_ = allocator;

    std::construct_at(&metadata->mutex_);
}

[[nodiscard]] void *allocator_boundary_tags::do_allocate_sm(
    size_t size)
{
    size_t total_size = size + sizeof(block_metadata);
    debug_with_guard(std::format("[*] allocating {} bytes", total_size));

    auto& metadata = get_allocator_metadata();

    std::lock_guard lock(metadata.mutex_);

    block_metadata* block = nullptr;

    switch (metadata.fit_mode_)
    {
    case fit_mode::first_fit:
        block = get_block_first_fit(total_size);
        break;
    case fit_mode::the_best_fit:
        block = get_block_best_fit(total_size);
        break;
    case fit_mode::the_worst_fit:
        block = get_block_worst_fit(total_size);
        break;
    }

    if (block == nullptr)
    {
        error_with_guard(std::format(
            "[!] out of memory: requested {} bytes", total_size));
        throw std::bad_alloc();
    }

    const size_t free_block_size = get_next_free_block_size(block);

    if (free_block_size < total_size + sizeof(block_metadata))
    {
        warning_with_guard(std::format(
            "[*] changing block size to {} bytes", free_block_size));
        total_size = free_block_size;
    }

    block_metadata* free_block;
    bool iter_begin = block == _trusted_memory;

    if (iter_begin)
    {
        free_block = reinterpret_cast<block_metadata*>(
            static_cast<std::byte*>(_trusted_memory) + sizeof(allocator_metadata));
    }
    else
    {
        free_block = reinterpret_cast<block_metadata*>(block->block_end());
    }

    free_block->block_size_ = total_size - sizeof(block_metadata);
    free_block->prev_ = block;
    free_block->next_ = iter_begin ? metadata.first_block_ : block->next_;
    free_block->tm_ptr_ = _trusted_memory;


    if (free_block->next_)
    {
        free_block->next_->prev_ = free_block;
    }

    if (iter_begin)
    {
        metadata.first_block_ = free_block;
    }
    else
    {
        free_block->prev_->next_ = free_block;
    }

    debug_with_guard(std::format(
        "[+] allocated {} bytes at {:p}",
        total_size, static_cast<void*>(free_block + 1)));
    information_with_guard(std::format(
        "[*] available memory: {}", get_available_memory()));
    debug_with_guard(print_blocks());

    return free_block + 1;
}

inline void allocator_boundary_tags::set_first_block(void *block)
{
    *reinterpret_cast<void**>(
            static_cast<char*>(_trusted_memory) + sizeof(logger*) + sizeof(std::pmr::memory_resource*) +
            sizeof(allocator_with_fit_mode::fit_mode) + sizeof(size_t) + sizeof(std::mutex)) = block;
}

void allocator_boundary_tags::do_deallocate_sm(
    void *at)
{
    debug_with_guard(std::format("[*] deallocating block {:p}", at));

    auto& metadata = get_allocator_metadata();

    std::lock_guard lock(metadata.mutex_);

    auto block = reinterpret_cast<block_metadata*>(
        static_cast<std::byte*>(at) - sizeof(block_metadata));

    if (block->tm_ptr_ != _trusted_memory)
    {
        error_with_guard(std::format(
            "[!] block doesn't belong to this allocator: {:p}", at));
        throw std::logic_error("unknown block");
    }

    debug_with_guard(get_dump(static_cast<char*>(at), block->block_size_));

    //Because this!
    // void *block_2 = static_cast<char *>(at) - occupied_block_metadata_size;
    // void *prev = *reinterpret_cast<void **>(block_2);
    //
    // void *next = *reinterpret_cast<void **>(static_cast<char *>(block_2) + sizeof(void *));
    //
    // if (prev)
    //         *reinterpret_cast<void **>(static_cast<char *>(prev) + sizeof(void *)) = next;
    // else
    //     set_first_block(next);
    //
    // if (next)
    //     *reinterpret_cast<void **>(next) = prev;
    

    if (block->prev_ == _trusted_memory)
    {
        metadata.first_block_ = block->next_;
    }
    else
    {
        block->prev_->next_ = block->next_;
    }

    if (block->next_)
    {
        block->next_->prev_ = block->prev_;
    }

    


    debug_with_guard("[+] block deallocated successfully");
    information_with_guard(std::format(
        "[*] available memory: {}", get_available_memory()));
    debug_with_guard(print_blocks());
}

inline void allocator_boundary_tags::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    std::string fit_mode_string;

    switch (mode)
    {
        case fit_mode::first_fit:
            fit_mode_string = "first_fit";
            break;
        case fit_mode::the_best_fit:
            fit_mode_string = "the_best_fit";
            break;
        case fit_mode::the_worst_fit:
            fit_mode_string = "the_worst_fit";
            break;
    }

    debug_with_guard(std::format(
        "[*] setting fit mode: {}", fit_mode_string));

    auto& metadata = get_allocator_metadata();
    std::lock_guard lock(metadata.mutex_);
    metadata.fit_mode_ = mode;
}


std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info() const
{
    auto& metadata = get_allocator_metadata();
    std::lock_guard lock(metadata.mutex_);
    return get_blocks_info_inner();
}

inline logger *allocator_boundary_tags::get_logger() const
{
    const auto& metadata = get_allocator_metadata();
    return metadata.logger_;
}

inline std::string allocator_boundary_tags::get_typename() const noexcept
{
    return "allocator_boundary_tags";
}

allocator_boundary_tags::allocator_metadata& allocator_boundary_tags::get_allocator_metadata() const noexcept
{
    return get_allocator_metadata(_trusted_memory);
}

allocator_boundary_tags::allocator_metadata& allocator_boundary_tags::get_allocator_metadata(void* trusted) noexcept
{
    return *static_cast<allocator_metadata*>(trusted);
}

const allocator_boundary_tags::allocator_metadata& allocator_boundary_tags::get_allocator_metadata(
    const void* trusted) noexcept
{
    return *static_cast<const allocator_metadata*>(trusted);
}

inline allocator_boundary_tags::block_metadata* allocator_boundary_tags::get_block_first_fit(size_t size) const noexcept
{
    for (auto it = begin(); it != end(); ++it)
    {
        if (!it.occupied() && it.size() >= size)
        {
            return static_cast<block_metadata*>(it.get_ptr());
        }
    }

    return nullptr;
}

inline allocator_boundary_tags::block_metadata* allocator_boundary_tags::get_block_best_fit(size_t size) const noexcept
{
    boundary_iterator result;

    for (auto it = begin(); it != end(); ++it)
    {
        if (!it.occupied() && it.size() >= size && (it.size() < result.size() || result.get_ptr() == nullptr))
        {
            result = it;
        }
    }

    return static_cast<block_metadata*>(result.get_ptr());
}

inline allocator_boundary_tags::block_metadata* allocator_boundary_tags::get_block_worst_fit(size_t size) const noexcept
{
    boundary_iterator result;

    for (auto it = begin(); it != end(); ++it)
    {
        if (!it.occupied() && it.size() >= size && it.size() > result.size())
        {
            result = it;
        }
    }

    return static_cast<block_metadata*>(result.get_ptr());
}

inline size_t allocator_boundary_tags::get_next_free_block_size(const block_metadata* block) const noexcept
{
    return get_next_free_block_size(_trusted_memory, block);
}

inline size_t allocator_boundary_tags::get_next_free_block_size(void* trusted, const block_metadata* block) noexcept
{
    const auto& metadata = get_allocator_metadata(trusted);



    if (block == trusted)
    {
        if (metadata.first_block_ == nullptr)
        {
            return metadata.mem_size_;
        }
        else
        {
            return reinterpret_cast<std::byte*>(metadata.first_block_)
                - static_cast<std::byte*>(trusted) - sizeof(allocator_metadata);
        }
    }



    if (block->next_ == nullptr)
    {
        return metadata.allocator_end() - block->block_end();
    }
    else
    {
        return reinterpret_cast<std::byte*>(block->next_) - block->block_end();
    }
}

size_t allocator_boundary_tags::get_available_memory() const noexcept
{
    size_t available_memory = 0;

    for (auto it = begin(); it != end(); ++it)
    {
        if (!it.occupied())
        {
            available_memory += it.size();
        }
    }

    return available_memory;
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::begin() const noexcept
{
    return {_trusted_memory};
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::end() const noexcept
{
    return {};
}

std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info_inner() const
{
    std::vector<allocator_test_utils::block_info> blocks;

    for (auto it = begin(); it != end(); ++it)
    {
        blocks.push_back({ it.size(), it.occupied() });
    }

    return blocks;
}

bool allocator_boundary_tags::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    return this == &other;
}

bool allocator_boundary_tags::boundary_iterator::operator==(
        const allocator_boundary_tags::boundary_iterator &other) const noexcept
{
    return _occupied_ptr == other._occupied_ptr;
}

bool allocator_boundary_tags::boundary_iterator::operator!=(
        const allocator_boundary_tags::boundary_iterator & other) const noexcept
{
    return !(*this == other);
}

allocator_boundary_tags::boundary_iterator &allocator_boundary_tags::boundary_iterator::operator++() & noexcept
{
    const auto metadata = static_cast<allocator_metadata*>(_trusted_memory);
    const auto block = static_cast<block_metadata*>(_occupied_ptr);

    if (_occupied)
    {
        const bool next_block_right_after = block->block_end() == reinterpret_cast<std::byte*>(block->next_);
        const bool last_block = block->block_end() == metadata->allocator_end();

        _occupied = next_block_right_after || (!block->next_ && last_block);

        if (_occupied)
        {
            _occupied_ptr = block->next_;
        }
    }
    else
    {
        _occupied = true;
        _occupied_ptr = _occupied_ptr == _trusted_memory ? metadata->first_block_ : block->next_;
    }

    return *this;
}

allocator_boundary_tags::boundary_iterator &allocator_boundary_tags::boundary_iterator::operator--() & noexcept
{
    const auto block = static_cast<block_metadata*>(_occupied_ptr);

    if (!_occupied)
    {
        _occupied = true;
    }
    else if (block->prev_ != _trusted_memory)
    {
        _occupied = block->prev_->block_end() == _occupied_ptr;
        _occupied_ptr = block->prev_;
    }

    return *this;
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::boundary_iterator::operator++(int n)
{
    const boundary_iterator tmp = *this;
    ++(*this);
    return tmp;
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::boundary_iterator::operator--(int n)
{
    const boundary_iterator tmp = *this;
    --(*this);
    return tmp;
}

size_t allocator_boundary_tags::boundary_iterator::size() const noexcept
{
    if (!_occupied_ptr)
    {
        return 0;
    }

    const auto block = static_cast<block_metadata*>(_occupied_ptr);

    if (_occupied)
    {
        return block->block_size_ + sizeof(block_metadata);
    } else
    {
        return get_next_free_block_size(_trusted_memory, block);
    }
}

bool allocator_boundary_tags::boundary_iterator::occupied() const noexcept
{
    return _occupied;
}

void* allocator_boundary_tags::boundary_iterator::operator*() const noexcept
{
    return _occupied ? _occupied_ptr : nullptr;
}

allocator_boundary_tags::boundary_iterator::boundary_iterator()
    : _occupied_ptr(nullptr), _occupied(false), _trusted_memory(nullptr)
{
}

allocator_boundary_tags::boundary_iterator::boundary_iterator(void *trusted)
    : _trusted_memory(trusted)
{
    const auto maybe_first_block = static_cast<std::byte*>(trusted) + sizeof(allocator_metadata);
    const auto first_allocator_block = reinterpret_cast<std::byte*>(get_allocator_metadata(trusted).first_block_);

    _occupied = maybe_first_block == first_allocator_block;

    if (_occupied)
    {
        _occupied_ptr = maybe_first_block;
    }
    else
    {
        _occupied_ptr = trusted;
    }
}

void *allocator_boundary_tags::boundary_iterator::get_ptr() const noexcept
{
    return _occupied_ptr;
}