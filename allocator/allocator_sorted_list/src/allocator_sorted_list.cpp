#include "../include/allocator_sorted_list.h"
#include <functional>
#include <algorithm>
#include <sstream>
#include <cstring>

allocator_sorted_list::allocator_sorted_list(
        size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        logger *logger,
        allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    if (space_size <= allocator_metadata_size)
    {
        throw std::bad_alloc();
    }

    
    _trusted_memory = parent_allocator
        ? parent_allocator->allocate(space_size)
        : ::operator new(space_size);

    
    auto memory_ptr = static_cast<char*>(_trusted_memory);
    *reinterpret_cast<class logger**>(memory_ptr) = logger;
    memory_ptr += sizeof(logger);

    
    *reinterpret_cast<std::pmr::memory_resource**>(memory_ptr) = parent_allocator;
    memory_ptr += sizeof(std::pmr::memory_resource*);

    
    *reinterpret_cast<fit_mode*>(memory_ptr) = allocate_fit_mode;
    memory_ptr += sizeof(fit_mode);

    
    *reinterpret_cast<size_t*>(memory_ptr) = space_size;
    memory_ptr += sizeof(size_t);

    
    new (memory_ptr) std::mutex();
    memory_ptr += sizeof(std::mutex);


    size_t available_size = space_size - allocator_metadata_size;
    char* first_block = static_cast<char*>(_trusted_memory) + allocator_metadata_size;

    
    *reinterpret_cast<void**>(first_block) = nullptr;
    
    *reinterpret_cast<size_t*>(first_block + sizeof(void*)) = available_size - block_metadata_size;

    
    *reinterpret_cast<void**>(memory_ptr) = first_block;

    if (logger)
    {
        logger->log("allocator_sorted_list constructor created with size: " + std::to_string(space_size),
            logger::severity::debug);
        logger->log("Available memory: " + std::to_string(available_size - block_metadata_size),
            logger::severity::information);
        logger->log("Memory state: avail " + std::to_string(available_size - block_metadata_size),
            logger::severity::debug);
    }
}

allocator_sorted_list::~allocator_sorted_list()
{
    auto logger_ptr = get_logger();
    if (logger_ptr)
    {
        logger_ptr->log("allocator_sorted_list destructor called", logger::severity::debug);
    }

    
    auto memory_ptr = static_cast<char*>(_trusted_memory);
    memory_ptr += sizeof(logger*);
    auto parent_allocator = *reinterpret_cast<std::pmr::memory_resource**>(memory_ptr);
    memory_ptr += sizeof(std::pmr::memory_resource*);
    memory_ptr += sizeof(fit_mode);

    
    auto space_size = *reinterpret_cast<size_t*>(memory_ptr);
    memory_ptr += sizeof(size_t);

    
    std::mutex* mutex_ptr = reinterpret_cast<std::mutex*>(memory_ptr);
    mutex_ptr->~mutex();

    
    if (parent_allocator)
    {
        parent_allocator->deallocate(_trusted_memory, space_size);
    }
    else
    {
        ::operator delete(_trusted_memory);
    }

    if (logger_ptr)
    {
        logger_ptr->log("allocator_sorted_list destroyed", logger::severity::debug);
    }
}

allocator_sorted_list::allocator_sorted_list(allocator_sorted_list &&other) noexcept
{
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;
}

allocator_sorted_list::allocator_sorted_list(const allocator_sorted_list &other)
{
    auto memory_ptr = static_cast<char*>(other._trusted_memory);
    memory_ptr += sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode);

    
    auto space_size = *reinterpret_cast<size_t*>(memory_ptr);

    
    auto parent_resource = *reinterpret_cast<std::pmr::memory_resource**>(
        static_cast<char*>(other._trusted_memory) + sizeof(logger*));

    
    _trusted_memory = parent_resource
        ? parent_resource->allocate(space_size)
        : ::operator new(space_size);

    
    std::memcpy(_trusted_memory, other._trusted_memory, space_size);

    
    auto dest_memory_ptr = static_cast<char*>(_trusted_memory);
    dest_memory_ptr += sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode) + sizeof(size_t);

    
    std::mutex* mutex_ptr = reinterpret_cast<std::mutex*>(dest_memory_ptr);
    mutex_ptr->~mutex();
    new (dest_memory_ptr) std::mutex();
}

allocator_sorted_list &allocator_sorted_list::operator=(allocator_sorted_list &&other) noexcept
{
    if (this != &other)
    {
        
        this->~allocator_sorted_list();

        
        _trusted_memory = other._trusted_memory;
        other._trusted_memory = nullptr;
    }
    return *this;
}

allocator_sorted_list &allocator_sorted_list::operator=(const allocator_sorted_list &other)
{
    if (this != &other)
    {
        allocator_sorted_list temp(other);
        *this = std::move(temp);
    }
    return *this;
}

void *allocator_sorted_list::do_allocate_sm(size_t size)
{
    auto logger_ptr = get_logger();
    if (logger_ptr)
    {
        logger_ptr->log("allocator_sorted_list::do_allocate_sm called with size " + std::to_string(size),
            logger::severity::debug);
    }

    
    auto memory_ptr = static_cast<char*>(_trusted_memory);
    memory_ptr += sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode) + sizeof(size_t);
    std::mutex* mutex_ptr = reinterpret_cast<std::mutex*>(memory_ptr);

    
    std::lock_guard<std::mutex> lock(*mutex_ptr);

    
    fit_mode mode = *reinterpret_cast<fit_mode*>(
    static_cast<char*>(_trusted_memory) + sizeof(logger*) + sizeof(std::pmr::memory_resource*));
    void* selected_block = nullptr;
    void* prev_block = nullptr;
    size_t adjusted_size = size;

    
    if (adjusted_size % alignof(std::max_align_t) != 0)
    {
        adjusted_size = (adjusted_size / alignof(std::max_align_t) + 1) * alignof(std::max_align_t);
        if (logger_ptr)
        {
            logger_ptr->log("Size adjusted from " + std::to_string(size) + " to " + std::to_string(adjusted_size),
                logger::severity::warning);
        }
    }

    
    switch (mode)
    {
        case fit_mode::first_fit:
        {
    
            for (auto it = free_begin(); it != free_end(); ++it)
            {
                if (it.size() >= adjusted_size)
                {
                    selected_block = *it;
                    break;
                }
            }
            break;
        }
        case fit_mode::the_best_fit:
        {
    
            size_t min_size_diff = SIZE_MAX;
            for (auto it = free_begin(); it != free_end(); ++it)
            {
                if (it.size() >= adjusted_size && it.size() - adjusted_size < min_size_diff)
                {
                    min_size_diff = it.size() - adjusted_size;
                    selected_block = *it;

    
                    if (min_size_diff == 0)
                        break;
                }
            }
            break;
        }
        case fit_mode::the_worst_fit:
        {
    
            size_t max_size = 0;
            for (auto it = free_begin(); it != free_end(); ++it)
            {
                if (it.size() >= adjusted_size && it.size() > max_size)
                {
                    max_size = it.size();
                    selected_block = *it;
                }
            }
            break;
        }
    }

    
    if (!selected_block)
    {
        if (logger_ptr)
        {
            logger_ptr->log("Failed to allocate " + std::to_string(adjusted_size) + " bytes: no suitable block found",
                logger::severity::error);
        }
        throw std::bad_alloc();
    }

    
    void* next_free = *reinterpret_cast<void**>(selected_block);
    size_t block_size = *reinterpret_cast<size_t*>(static_cast<char*>(selected_block) + sizeof(void*));

    
    void* user_data = static_cast<char*>(selected_block) + block_metadata_size;

    
    if (block_size >= adjusted_size + block_metadata_size + 1)
    {
    
        void* new_free_block = static_cast<char*>(selected_block) + block_metadata_size + adjusted_size;

    
        *reinterpret_cast<void**>(new_free_block) = next_free;

    
        *reinterpret_cast<size_t*>(static_cast<char*>(new_free_block) + sizeof(void*)) =
            block_size - adjusted_size - block_metadata_size;

    
        *reinterpret_cast<size_t*>(static_cast<char*>(selected_block) + sizeof(void*)) = adjusted_size;

    
        memory_ptr += sizeof(std::mutex);
        void** free_list_head = reinterpret_cast<void**>(memory_ptr);

        if (*free_list_head == selected_block)
        {
            *free_list_head = new_free_block;
        }
        else
        {
            
            for (auto it = free_begin(); it != free_end(); ++it)
            {
                if (*reinterpret_cast<void**>(*it) == selected_block)
                {
                    *reinterpret_cast<void**>(*it) = new_free_block;
                    break;
                }
            }
        }
    }
    else
    {
        
        memory_ptr += sizeof(std::mutex);
        void** free_list_head = reinterpret_cast<void**>(memory_ptr);

        if (*free_list_head == selected_block)
        {
            *free_list_head = next_free;
        }
        else
        {
            
            for (auto it = free_begin(); it != free_end(); ++it)
            {
                if (*reinterpret_cast<void**>(*it) == selected_block)
                {
                    *reinterpret_cast<void**>(*it) = next_free;
                    break;
                }
            }
        }
    }

    
    *reinterpret_cast<void**>(selected_block) = nullptr;


    if (logger_ptr)
    {

        size_t available_memory = 0;
        for (auto it = free_begin(); it != free_end(); ++it)
        {
            available_memory += it.size();
        }
        logger_ptr->log("Available memory after allocation: " + std::to_string(available_memory),
            logger::severity::information);

        
        std::stringstream state;
        for (auto it = begin(); it != end(); ++it)
        {
            state << (it.occupied() ? "occup " : "avail ") << it.size() << "|";
        }
        logger_ptr->log("Memory state: " + state.str(), logger::severity::debug);

        logger_ptr->log("allocator_sorted_list::do_allocate_sm completed", logger::severity::debug);
    }

    return user_data;
}

void allocator_sorted_list::do_deallocate_sm(void *at)
{
    auto logger_ptr = get_logger();
    if (logger_ptr)
    {
        logger_ptr->log("allocator_sorted_list::do_deallocate_sm called", logger::severity::debug);
    }

    if (!at)
    {
        if (logger_ptr)
        {
            logger_ptr->log("Attempt to deallocate nullptr", logger::severity::warning);
        }
        return;
    }

    
    auto memory_ptr = static_cast<char*>(_trusted_memory);
    memory_ptr += sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode) + sizeof(size_t);
    std::mutex* mutex_ptr = reinterpret_cast<std::mutex*>(memory_ptr);

    
    std::lock_guard<std::mutex> lock(*mutex_ptr);

    
    void* block_ptr = static_cast<char*>(at) - block_metadata_size;

    
    char* mem_start = static_cast<char*>(_trusted_memory) + allocator_metadata_size;
    memory_ptr += sizeof(std::mutex);
    size_t space_size = *reinterpret_cast<size_t*>(static_cast<char*>(_trusted_memory) +
                                                 sizeof(logger*) +
                                                 sizeof(std::pmr::memory_resource*) +
                                                 sizeof(fit_mode));
    char* mem_end = static_cast<char*>(_trusted_memory) + space_size;

    if (block_ptr < mem_start || block_ptr >= mem_end)
    {
        if (logger_ptr)
        {
            logger_ptr->log("Attempt to deallocate memory not owned by this allocator", logger::severity::error);
        }
        throw std::invalid_argument("Memory block does not belong to this allocator");
    }

    
    size_t block_size = *reinterpret_cast<size_t*>(static_cast<char*>(block_ptr) + sizeof(void*));

    
    void** free_list_head = reinterpret_cast<void**>(memory_ptr);

    
    void* prev_free = nullptr;
    void* curr_free = *free_list_head;

    
    while (curr_free && curr_free < block_ptr)
    {
        prev_free = curr_free;
        curr_free = *reinterpret_cast<void**>(curr_free);
    }

    
    if (prev_free)
    {
    
        *reinterpret_cast<void**>(block_ptr) = *reinterpret_cast<void**>(prev_free);
        *reinterpret_cast<void**>(prev_free) = block_ptr;
    }
    else
    {
    
        *reinterpret_cast<void**>(block_ptr) = *free_list_head;
        *free_list_head = block_ptr;
    }

    
    
    if (prev_free &&
        static_cast<char*>(prev_free) + block_metadata_size +
        *reinterpret_cast<size_t*>(static_cast<char*>(prev_free) + sizeof(void*)) == block_ptr)
    {
    
        *reinterpret_cast<size_t*>(static_cast<char*>(prev_free) + sizeof(void*)) +=
            block_metadata_size + block_size;

        
        *reinterpret_cast<void**>(prev_free) = *reinterpret_cast<void**>(block_ptr);

        
        block_ptr = prev_free;
        block_size = *reinterpret_cast<size_t*>(static_cast<char*>(block_ptr) + sizeof(void*));
    }

    
    void* next_free = *reinterpret_cast<void**>(block_ptr);
    if (next_free &&
        static_cast<char*>(block_ptr) + block_metadata_size + block_size == next_free)
    {
        
        *reinterpret_cast<size_t*>(static_cast<char*>(block_ptr) + sizeof(void*)) +=
            block_metadata_size + *reinterpret_cast<size_t*>(static_cast<char*>(next_free) + sizeof(void*));

        
        *reinterpret_cast<void**>(block_ptr) = *reinterpret_cast<void**>(next_free);
    }

    
    if (logger_ptr)
    {
        
        size_t available_memory = 0;
        for (auto it = free_begin(); it != free_end(); ++it)
        {
            available_memory += it.size();
        }
        logger_ptr->log("Available memory after deallocation: " + std::to_string(available_memory),
            logger::severity::information);

        
        std::stringstream state;
        for (auto it = begin(); it != end(); ++it)
        {
            state << (it.occupied() ? "occup " : "avail ") << it.size() << "|";
        }
        logger_ptr->log("Memory state: " + state.str(), logger::severity::debug);

        logger_ptr->log("allocator_sorted_list::do_deallocate_sm completed", logger::severity::debug);
    }
}

bool allocator_sorted_list::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    auto logger_ptr = get_logger();
    if (logger_ptr)
    {
        logger_ptr->log("allocator_sorted_list::do_is_equal called", logger::severity::debug);
    }

    if (this == &other)
        return true;

    auto* other_alloc = dynamic_cast<const allocator_sorted_list*>(&other);
    return other_alloc && _trusted_memory == other_alloc->_trusted_memory;
}

inline void allocator_sorted_list::set_fit_mode(allocator_with_fit_mode::fit_mode mode)
{
    auto logger_ptr = get_logger();
    if (logger_ptr)
    {
        logger_ptr->log("allocator_sorted_list::set_fit_mode called", logger::severity::debug);
    }

    
    auto memory_ptr = static_cast<char*>(_trusted_memory);
    memory_ptr += sizeof(logger*) + sizeof(std::pmr::memory_resource*);

    
    *reinterpret_cast<fit_mode*>(memory_ptr) = mode;

    if (logger_ptr)
    {
        logger_ptr->log("Fit mode changed to " + std::to_string(static_cast<int>(mode)),
            logger::severity::information);
        logger_ptr->log("allocator_sorted_list::set_fit_mode completed", logger::severity::debug);
    }
}

std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info() const noexcept
{
    auto logger_ptr = get_logger();
    if (logger_ptr)
    {
        logger_ptr->log("allocator_sorted_list::get_blocks_info called", logger::severity::debug);
    }

    
    auto memory_ptr = static_cast<char*>(_trusted_memory);
    memory_ptr += sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode) + sizeof(size_t);
    std::mutex* mutex_ptr = reinterpret_cast<std::mutex*>(memory_ptr);

    
    std::lock_guard<std::mutex> lock(*mutex_ptr);

    return get_blocks_info_inner();
}

std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info_inner() const
{
    std::vector<allocator_test_utils::block_info> result;

    
    for (auto it = begin(); it != end(); ++it)
    {
        allocator_test_utils::block_info info;
        info.block_size = it.size();
        info.is_block_occupied = it.occupied();
        result.push_back(info);
    }

    return result;
}

inline logger *allocator_sorted_list::get_logger() const
{
    if (!_trusted_memory)
        return nullptr;

    
    return *reinterpret_cast<logger**>(_trusted_memory);
}

inline std::string allocator_sorted_list::get_typename() const
{
    return "allocator_sorted_list";
}



allocator_sorted_list::sorted_free_iterator allocator_sorted_list::free_begin() const noexcept
{
    if (!_trusted_memory)
        return sorted_free_iterator(nullptr);

    
    auto memory_ptr = static_cast<char*>(_trusted_memory);
    memory_ptr += sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode) + sizeof(size_t) + sizeof(std::mutex);

    return sorted_free_iterator(*reinterpret_cast<void**>(memory_ptr));
}

allocator_sorted_list::sorted_free_iterator allocator_sorted_list::free_end() const noexcept
{
    return sorted_free_iterator(nullptr);
}

allocator_sorted_list::sorted_iterator allocator_sorted_list::begin() const noexcept
{
    if (!_trusted_memory)
        return sorted_iterator(nullptr);

    return sorted_iterator(static_cast<char*>(_trusted_memory) + allocator_metadata_size);
}

allocator_sorted_list::sorted_iterator allocator_sorted_list::end() const noexcept
{
    if (!_trusted_memory)
        return sorted_iterator(nullptr);

    
    auto memory_ptr = static_cast<char*>(_trusted_memory);
    memory_ptr += sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode);
    auto space_size = *reinterpret_cast<size_t*>(memory_ptr);

    return sorted_iterator(static_cast<char*>(_trusted_memory) + space_size);
}

bool allocator_sorted_list::sorted_free_iterator::operator==(
        const allocator_sorted_list::sorted_free_iterator & other) const noexcept
{
    return _free_ptr == other._free_ptr;
}

bool allocator_sorted_list::sorted_free_iterator::operator!=(
        const allocator_sorted_list::sorted_free_iterator &other) const noexcept
{
    return _free_ptr != other._free_ptr;
}

allocator_sorted_list::sorted_free_iterator &allocator_sorted_list::sorted_free_iterator::operator++() & noexcept
{
    if (_free_ptr)
    {
        _free_ptr = *reinterpret_cast<void**>(_free_ptr);
    }
    return *this;
}

allocator_sorted_list::sorted_free_iterator allocator_sorted_list::sorted_free_iterator::operator++(int n)
{
    sorted_free_iterator tmp(*this);
    ++(*this);
    return tmp;
}

size_t allocator_sorted_list::sorted_free_iterator::size() const noexcept
{
    if (!_free_ptr)
        return 0;

    
    return *reinterpret_cast<size_t*>(static_cast<char*>(_free_ptr) + sizeof(void*));
}

void *allocator_sorted_list::sorted_free_iterator::operator*() const noexcept
{
    return _free_ptr;
}

allocator_sorted_list::sorted_free_iterator::sorted_free_iterator()
    : _free_ptr(nullptr)
{
}

allocator_sorted_list::sorted_free_iterator::sorted_free_iterator(void *trusted)
    : _free_ptr(trusted)
{
}

bool allocator_sorted_list::sorted_iterator::operator==(const allocator_sorted_list::sorted_iterator & other) const noexcept
{
    return _current_ptr == other._current_ptr;
}

bool allocator_sorted_list::sorted_iterator::operator!=(const allocator_sorted_list::sorted_iterator &other) const noexcept
{
    return _current_ptr != other._current_ptr;
}

allocator_sorted_list::sorted_iterator &allocator_sorted_list::sorted_iterator::operator++() & noexcept
{
    if (_current_ptr)
    {
        
        size_t block_size = *reinterpret_cast<size_t*>(static_cast<char*>(_current_ptr) + sizeof(void*));
        _current_ptr = static_cast<char*>(_current_ptr) + block_metadata_size + block_size;

        
        char* mem_start = static_cast<char*>(_trusted_memory);
        size_t mem_size = *reinterpret_cast<size_t*>(mem_start + sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(allocator_with_fit_mode::fit_mode));

        if (_current_ptr >= static_cast<char*>(mem_start) + mem_size)
        {
            _current_ptr = static_cast<char*>(mem_start) + mem_size; 
        }
    }
    return *this;
}

allocator_sorted_list::sorted_iterator allocator_sorted_list::sorted_iterator::operator++(int n)
{
    sorted_iterator tmp(*this);
    ++(*this);
    return tmp;
}

size_t allocator_sorted_list::sorted_iterator::size() const noexcept
{
    if (!_current_ptr)
        return 0;

    
    return *reinterpret_cast<size_t*>(static_cast<char*>(_current_ptr) + sizeof(void*));
}

void *allocator_sorted_list::sorted_iterator::operator*() const noexcept
{
    
    return static_cast<char*>(_current_ptr) + block_metadata_size;
}

allocator_sorted_list::sorted_iterator::sorted_iterator()
    : _free_ptr(nullptr), _current_ptr(nullptr), _trusted_memory(nullptr)
{
}

allocator_sorted_list::sorted_iterator::sorted_iterator(void *trusted)
    : _current_ptr(trusted), _trusted_memory(trusted)
{
    
    
    auto memory_ptr = static_cast<char*>(_trusted_memory);
    memory_ptr += sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(size_t) + sizeof(std::mutex);
    _free_ptr = *reinterpret_cast<void**>(memory_ptr);
}

bool allocator_sorted_list::sorted_iterator::occupied() const noexcept
{
    if (!_current_ptr)
        return false;

    
    

    
    void* free_block = _free_ptr;
    while (free_block)
    {
        if (free_block == _current_ptr)
            return false; 

        free_block = *reinterpret_cast<void**>(free_block);
    }

    return true;
}
