#include <not_implemented.h>
#include "../include/allocator_boundary_tags.h"
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <cstring>

struct block_header {
    size_t size;
    block_header* prev_block;
    block_header* next_block;
    void* parent_allocator;
};

bool is_block_occupied(block_header* block) {
    return (block->size & 1) == 1;
}

void set_block_occupied(block_header* block, bool occupied) {
    if (occupied) {
        block->size |= 1;
    } else {
        block->size &= ~size_t(1);
    }
}

size_t get_block_size(block_header* block) {
    return block->size & ~size_t(1);
}

void set_block_size(block_header* block, size_t size) {
    bool occupied = is_block_occupied(block);
    block->size = size;
    set_block_occupied(block, occupied);
}

const std::mutex& allocator_boundary_tags::mutex() const {
    return _mutex;
}

allocator_with_fit_mode::fit_mode allocator_boundary_tags::get_fit_mode() const {
    return _current_fit_mode;
}

void allocator_boundary_tags::set_logger(logger* log) {
    _logger = log;
}

logger* allocator_boundary_tags::get_logger() const {
    return _logger;
}

void* get_user_data(block_header* block) {
    return reinterpret_cast<char*>(block) + sizeof(block_header);
}

block_header* get_header_from_user_data(void* user_data) {
    return reinterpret_cast<block_header*>(
        static_cast<char*>(user_data) - sizeof(block_header));
}

size_t calculate_block_size(size_t user_size) {
    size_t size = user_size + sizeof(block_header);
    return size;
}

allocator_boundary_tags::~allocator_boundary_tags()
{
    try {
        if (_trusted_memory != nullptr) {
            if (get_logger() != nullptr) {
                get_logger()->log("~allocator_boundary_tags() called", logger::severity::debug);
            }

            void* original_memory = static_cast<char*>(_trusted_memory) - allocator_metadata_size;

            auto* block = static_cast<block_header*>(_trusted_memory);
            auto* parent_allocator = static_cast<std::pmr::memory_resource*>(block->parent_allocator);

            if (parent_allocator != nullptr) {
                parent_allocator->deallocate(original_memory, 0, 0);
            } else {
                auto* parent = dynamic_cast<std::pmr::memory_resource*>(const_cast<allocator_boundary_tags*>(this));
                parent->deallocate(original_memory, 0, 0);
            }

            _trusted_memory = nullptr;
        }
    } catch (...) {
        if (get_logger() != nullptr) {
            get_logger()->log("Exception caught in destructor", logger::severity::critical);
        }
    }
}

allocator_boundary_tags::allocator_boundary_tags(
    allocator_boundary_tags &&other) noexcept
{
    try {
        _trusted_memory = other._trusted_memory;
        other._trusted_memory = nullptr;

        _logger = other._logger;
        if (_logger != nullptr) {
            _logger->log("allocator_boundary_tags move constructor called", logger::severity::debug);
        }
        other._logger = nullptr;

        _current_fit_mode = other._current_fit_mode;
    } catch (...) {
        _trusted_memory = nullptr;
        _logger = nullptr;
    }
}

allocator_boundary_tags &allocator_boundary_tags::operator=(
    allocator_boundary_tags &&other) noexcept
{
    if (this != &other) {
        try {
            if (_logger != nullptr) {
                _logger->log("allocator_boundary_tags move assignment called", logger::severity::debug);
            }

            if (_trusted_memory != nullptr) {
                void* original_memory = static_cast<char*>(_trusted_memory) - allocator_metadata_size;

                auto* block = static_cast<block_header*>(_trusted_memory);
                auto* parent_allocator = static_cast<std::pmr::memory_resource*>(block->parent_allocator);

                if (parent_allocator != nullptr) {
                    parent_allocator->deallocate(original_memory, 0, 0);
                } else {
                    auto* parent = dynamic_cast<std::pmr::memory_resource*>(this);
                    parent->deallocate(original_memory, 0, 0);
                }

                _trusted_memory = nullptr;
            }

            _trusted_memory = other._trusted_memory;
            other._trusted_memory = nullptr;

            _logger = other._logger;
            other._logger = nullptr;

            _current_fit_mode = other._current_fit_mode;
        } catch (...) {
            _trusted_memory = nullptr;
            _logger = nullptr;
        }
    }
    return *this;
}

allocator_boundary_tags::allocator_boundary_tags(
        size_t space_size,
        memory_resource *parent_allocator,
        logger *logger,
        fit_mode allocate_fit_mode)
{
    try {
        if (parent_allocator == nullptr) {
            parent_allocator = std::pmr::get_default_resource();
        }

        set_logger(logger);
        set_fit_mode(allocate_fit_mode);

        if (get_logger() != nullptr) {
            get_logger()->log("allocator_boundary_tags constructor called", logger::severity::debug);
            get_logger()->log("space_size: " + std::to_string(space_size), logger::severity::debug);
        }

        if (space_size < sizeof(block_header) + 4) {
            if (get_logger() != nullptr) {
                get_logger()->log("Space size is too small", logger::severity::error);
            }
            throw std::invalid_argument("Space size is too small");
        }

        size_t total_memory_size = space_size + allocator_metadata_size;

        if (get_logger() != nullptr) {
            get_logger()->log("Allocating memory with size: " + std::to_string(total_memory_size) +
                             " (including " + std::to_string(allocator_metadata_size) + " bytes for metadata)",
                             logger::severity::debug);
        }

        void* allocated_memory = parent_allocator->allocate(total_memory_size, alignof(std::max_align_t));

        char* metadata_area = static_cast<char*>(allocated_memory);

        _trusted_memory = metadata_area + allocator_metadata_size;

        if (get_logger() != nullptr) {
            get_logger()->log("[DEBUG constructor] Initializing first block", logger::severity::debug);
        }

        auto first_block = static_cast<block_header*>(_trusted_memory);
        first_block->size = space_size;
        set_block_occupied(first_block, false);
        first_block->prev_block = nullptr;
        first_block->next_block = nullptr;
        first_block->parent_allocator = parent_allocator;

        if (get_logger() != nullptr) {
            get_logger()->log("[DEBUG constructor] First block initialized with size: " + std::to_string(space_size), logger::severity::debug);
        }
    }
    catch (const std::exception& e) {
        if (logger != nullptr) {
            logger->log("Exception during allocator initialization: " + std::string(e.what()), logger::severity::critical);
        }
        _trusted_memory = nullptr;
        throw;
    }
}

void *allocator_boundary_tags::do_allocate_empty_block() {
    try {
        if (get_logger() != nullptr) {
            get_logger()->log("Allocated empty block", logger::severity::debug);
        }

        size_t required_size = sizeof(block_header);

        if (_trusted_memory == nullptr) {
            throw std::bad_alloc();
        }

        std::lock_guard<std::mutex> lock(_mutex);

        block_header* current = static_cast<block_header*>(_trusted_memory);
        block_header* selected = nullptr;

        while (current != nullptr) {
            if (!is_block_occupied(current) && get_block_size(current) >= required_size) {
                selected = current;
                break;
            }
            current = current->next_block;
        }

        if (selected == nullptr) {
            if (get_logger() != nullptr) {
                get_logger()->log("Failed to allocate empty block: no suitable block found", logger::severity::error);
            }
            throw std::bad_alloc();
        }

        if (get_block_size(selected) > required_size) {
            size_t remaining_size = get_block_size(selected) - required_size;
            block_header* new_block = reinterpret_cast<block_header*>(
                reinterpret_cast<char*>(selected) + required_size);

            new_block->size = remaining_size;
            set_block_occupied(new_block, false);
            new_block->prev_block = selected;
            new_block->next_block = selected->next_block;
            new_block->parent_allocator = selected->parent_allocator;

            if (selected->next_block != nullptr) {
                selected->next_block->prev_block = new_block;
            }
            selected->next_block = new_block;

            set_block_size(selected, required_size);
        }

        set_block_occupied(selected, true);

        if (get_logger() != nullptr) {
            std::string blocks_state;
            auto blocks = get_blocks_info_inner();
            for (const auto& b : blocks) {
                blocks_state += (b.is_block_occupied ? "occup " : "avail ") + std::to_string(b.block_size) + "|";
            }
            if (!blocks_state.empty()) blocks_state.pop_back();
            get_logger()->log("Blocks state after allocating empty block: " + blocks_state, logger::severity::debug);
        }

        return get_user_data(selected);
    }
    catch (const std::bad_alloc&) {
        throw;
    }
    catch (const std::exception& e) {
        if (get_logger() != nullptr) {
            get_logger()->log("Exception during empty block allocation: " + std::string(e.what()), logger::severity::error);
        }
        throw std::bad_alloc();
    }
}

[[nodiscard]] void *allocator_boundary_tags::do_allocate_sm(
    size_t size)
{
    try {
        if (_trusted_memory == nullptr) {
            throw std::bad_alloc();
        }

        if (size == 0) {
            return do_allocate_empty_block();
        }

        if (get_logger() != nullptr) {
            get_logger()->log("do_allocate_sm called with size: " + std::to_string(size), logger::severity::debug);
        }

        size_t user_data_size = size;
        size_t required_size = calculate_block_size(user_data_size);

        if (get_logger() != nullptr) {
            get_logger()->log("[DEBUG do_allocate_sm] User data size: " + std::to_string(user_data_size) +
                             ", Required block size: " + std::to_string(required_size), logger::severity::debug);
        }

        std::lock_guard<std::mutex> lock(_mutex);

        block_header* current = static_cast<block_header*>(_trusted_memory);
        block_header* selected = nullptr;

        size_t best_fit_size = SIZE_MAX;
        size_t worst_fit_size = 0;

        switch (get_fit_mode()) {
            case fit_mode::first_fit:
            {
                while (current != nullptr) {
                    if (!is_block_occupied(current) && get_block_size(current) >= required_size) {
                        selected = current;
                        break;
                    }
                    current = current->next_block;
                }
                if (get_logger() != nullptr) {
                    get_logger()->log("[DEBUG do_allocate_sm] Using first-fit strategy", logger::severity::debug);
                }
                break;
            }

            case fit_mode::the_best_fit:
            {
                while (current != nullptr) {
                    if (!is_block_occupied(current) && get_block_size(current) >= required_size && get_block_size(current) < best_fit_size) {
                        selected = current;
                        best_fit_size = get_block_size(current);
                    }
                    current = current->next_block;
                }
                if (get_logger() != nullptr) {
                    get_logger()->log("[DEBUG do_allocate_sm] Using best-fit strategy", logger::severity::debug);
                }
                break;
            }

            case fit_mode::the_worst_fit:
            {
                while (current != nullptr) {
                    if (!is_block_occupied(current) && get_block_size(current) >= required_size && get_block_size(current) > worst_fit_size) {
                        selected = current;
                        worst_fit_size = get_block_size(current);
                    }
                    current = current->next_block;
                }
                if (get_logger() != nullptr) {
                    get_logger()->log("[DEBUG do_allocate_sm] Using worst-fit strategy", logger::severity::debug);
                }
                break;
            }
        }

        if (selected == nullptr) {
            if (get_logger() != nullptr) {
                get_logger()->log("Failed to allocate block of size " + std::to_string(size) + " bytes: no suitable block found", logger::severity::error);
            }
            throw std::bad_alloc();
        }

        if (get_logger() != nullptr) {
            get_logger()->log("[DEBUG do_allocate_sm] Found suitable block of size: " + std::to_string(get_block_size(selected)), logger::severity::debug);
        }

        if (get_block_size(selected) >= required_size + sizeof(block_header) + 4) {
            size_t remaining_size = get_block_size(selected) - required_size;

            if (get_logger() != nullptr) {
                get_logger()->log("[DEBUG do_allocate_sm] Splitting block. Original size: " + std::to_string(get_block_size(selected)) +
                                 ", New size: " + std::to_string(required_size) +
                                 ", Remaining size: " + std::to_string(remaining_size), logger::severity::debug);
            }

            block_header* new_block = reinterpret_cast<block_header*>(
                reinterpret_cast<char*>(selected) + required_size);

            new_block->size = remaining_size;
            set_block_occupied(new_block, false);
            new_block->prev_block = selected;
            new_block->next_block = selected->next_block;
            new_block->parent_allocator = selected->parent_allocator;

            if (selected->next_block != nullptr) {
                selected->next_block->prev_block = new_block;
            }
            selected->next_block = new_block;

            set_block_size(selected, required_size);
        }

        set_block_occupied(selected, true);

        if (get_logger() != nullptr) {
            std::string blocks_state;
            auto blocks = get_blocks_info_inner();
            for (const auto& b : blocks) {
                blocks_state += (b.is_block_occupied ? "occup " : "avail ") + std::to_string(b.block_size) + "|";
            }
            if (!blocks_state.empty()) blocks_state.pop_back();
            get_logger()->log("Blocks state: " + blocks_state, logger::severity::debug);

            size_t total_free = 0;
            for (const auto& b : blocks) {
                if (!b.is_block_occupied) {
                    total_free += b.block_size;
                }
            }
            get_logger()->log("Available memory after allocation: " + std::to_string(total_free),
                             logger::severity::information);
        }

        return get_user_data(selected);
    }
    catch (const std::bad_alloc&) {
        throw;
    }
    catch (const std::exception& e) {
        if (get_logger() != nullptr) {
            get_logger()->log("Exception during allocation: " + std::string(e.what()), logger::severity::error);
        }
        throw std::bad_alloc();
    }
}

void allocator_boundary_tags::do_deallocate_sm(
    void *at)
{
    try {
        if (at == nullptr || _trusted_memory == nullptr) {
            return;
        }

        if (get_logger() != nullptr) {
            get_logger()->log("do_deallocate_sm called", logger::severity::debug);
        }

        std::lock_guard<std::mutex> lock(_mutex);
        block_header* block = get_header_from_user_data(at);
        char* block_ptr = reinterpret_cast<char*>(block);
        char* memory_start = reinterpret_cast<char*>(_trusted_memory);

        if (block_ptr < memory_start) {
            if (get_logger() != nullptr) {
                get_logger()->log("Invalid deallocation address", logger::severity::error);
            }
            return;
        }

        if (get_logger() != nullptr) {
            get_logger()->log("[DEBUG do_deallocate_sm] Deallocating block of size: " + std::to_string(get_block_size(block)), logger::severity::debug);
        }

        set_block_occupied(block, false);

        if (block->next_block != nullptr && !is_block_occupied(block->next_block)) {
            block_header* next_block = block->next_block;

            if (get_logger() != nullptr) {
                get_logger()->log("[DEBUG do_deallocate_sm] Coalescing with next block. Current size: " + std::to_string(get_block_size(block)) +
                                 ", Next block size: " + std::to_string(get_block_size(next_block)), logger::severity::debug);
            }

            set_block_size(block, get_block_size(block) + get_block_size(next_block));

            block->next_block = next_block->next_block;
            if (next_block->next_block != nullptr) {
                next_block->next_block->prev_block = block;
            }
        }

        if (block->prev_block != nullptr && !is_block_occupied(block->prev_block)) {
            block_header* prev_block = block->prev_block;

            if (get_logger() != nullptr) {
                get_logger()->log("[DEBUG do_deallocate_sm] Coalescing with previous block. Current size: " + std::to_string(get_block_size(block)) +
                                 ", Previous block size: " + std::to_string(get_block_size(prev_block)), logger::severity::debug);
            }

            set_block_size(prev_block, get_block_size(prev_block) + get_block_size(block));

            prev_block->next_block = block->next_block;
            if (block->next_block != nullptr) {
                block->next_block->prev_block = prev_block;
            }
        }

        if (get_logger() != nullptr) {
            std::string blocks_state;
            auto blocks = get_blocks_info_inner();
            for (const auto& b : blocks) {
                blocks_state += (b.is_block_occupied ? "occup " : "avail ") + std::to_string(b.block_size) + "|";
            }
            if (!blocks_state.empty()) blocks_state.pop_back();
            get_logger()->log("Blocks state after deallocation: " + blocks_state, logger::severity::debug);

            size_t total_free = 0;
            for (const auto& b : blocks) {
                if (!b.is_block_occupied) {
                    total_free += b.block_size;
                }
            }
            get_logger()->log("Available memory after deallocation: " + std::to_string(total_free),
                             logger::severity::information);
        }
    }
    catch (const std::exception& e) {
        if (get_logger() != nullptr) {
            get_logger()->log("Exception during deallocation: " + std::string(e.what()), logger::severity::error);
        }
    }
}

void allocator_boundary_tags::set_fit_mode(
    fit_mode mode)
{
    _current_fit_mode = mode;
}

std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info() const
{
    try {
        std::lock_guard<std::mutex> lock(_mutex);
        return get_blocks_info_inner();
    }
    catch (const std::exception& e) {
        if (get_logger() != nullptr) {
            get_logger()->log("Exception in get_blocks_info: " + std::string(e.what()), logger::severity::error);
        }
        return std::vector<block_info>();
    }
}

std::string allocator_boundary_tags::get_typename() const noexcept
{
    return "allocator_boundary_tags";
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::begin() const noexcept
{
    if (_trusted_memory == nullptr) {
        return boundary_iterator();
    }
    return boundary_iterator(_trusted_memory);
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::end() const noexcept
{
    return boundary_iterator();
}

std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info_inner() const
{
    try {
        std::vector<block_info> result;

        if (_trusted_memory == nullptr) {
            return result;
        }

        block_header* current = static_cast<block_header*>(_trusted_memory);
        while (current != nullptr) {
            block_info info;
            info.block_size = get_block_size(current);
            info.is_block_occupied = is_block_occupied(current);

            result.push_back(info);
            current = current->next_block;
        }

        return result;
    }
    catch (const std::exception& e) {
        if (get_logger() != nullptr) {
            get_logger()->log("Exception in get_blocks_info_inner: " + std::string(e.what()), logger::severity::error);
        }
        return std::vector<block_info>();
    }
}

allocator_boundary_tags::allocator_boundary_tags(const allocator_boundary_tags &other)
{
    try {
        _trusted_memory = nullptr;
        _logger = nullptr;

        if (other._trusted_memory == nullptr) {
            return;
        }

        if (other.get_logger() != nullptr) {
            other.get_logger()->log("allocator_boundary_tags copy constructor called", logger::severity::debug);
        }

        block_header* first_block = static_cast<block_header*>(other._trusted_memory);
        size_t blocks_memory_size = 0;

        block_header* current = first_block;
        while (current != nullptr) {
            blocks_memory_size += get_block_size(current);
            current = current->next_block;
        }

        size_t total_size = blocks_memory_size + allocator_metadata_size;

        if (other.get_logger() != nullptr) {
            other.get_logger()->log("[DEBUG copy constructor] Total memory size: " + std::to_string(total_size) +
                                   " (including " + std::to_string(allocator_metadata_size) + " bytes for metadata)",
                                   logger::severity::debug);
        }

        std::pmr::memory_resource* parent_allocator = std::pmr::get_default_resource();
        void* allocated_memory = parent_allocator->allocate(total_size, alignof(std::max_align_t));

        char* metadata_area = static_cast<char*>(allocated_memory);
        _trusted_memory = metadata_area + allocator_metadata_size;

        std::memcpy(_trusted_memory, other._trusted_memory, blocks_memory_size);

        ptrdiff_t offset = static_cast<char*>(_trusted_memory) - static_cast<char*>(other._trusted_memory);

        current = static_cast<block_header*>(_trusted_memory);
        while (current != nullptr) {
            if (current->prev_block != nullptr) {
                current->prev_block = reinterpret_cast<block_header*>(
                    reinterpret_cast<char*>(current->prev_block) + offset);
            }

            if (current->next_block != nullptr) {
                current->next_block = reinterpret_cast<block_header*>(
                    reinterpret_cast<char*>(current->next_block) + offset);
            }

            current->parent_allocator = parent_allocator;

            current = current->next_block;
        }

        set_logger(other.get_logger());
        _current_fit_mode = other._current_fit_mode;

        if (get_logger() != nullptr) {
            get_logger()->log("[DEBUG copy constructor] Copy completed successfully", logger::severity::debug);
        }
    }
    catch (const std::exception& e) {
        if (get_logger() != nullptr) {
            get_logger()->log("Exception in copy constructor: " + std::string(e.what()), logger::severity::error);
        }
        _trusted_memory = nullptr;
        _logger = nullptr;
    }
}

allocator_boundary_tags &allocator_boundary_tags::operator=(const allocator_boundary_tags &other)
{
    if (this != &other) {
        try {
            if (get_logger() != nullptr) {
                get_logger()->log("allocator_boundary_tags copy assignment called", logger::severity::debug);
            }

            if (_trusted_memory != nullptr) {
                void* original_memory = static_cast<char*>(_trusted_memory) - allocator_metadata_size;

                auto* block = static_cast<block_header*>(_trusted_memory);
                auto* parent_allocator = static_cast<std::pmr::memory_resource*>(block->parent_allocator);

                if (parent_allocator != nullptr) {
                    parent_allocator->deallocate(original_memory, 0, 0);
                } else {
                    auto* parent = dynamic_cast<std::pmr::memory_resource*>(this);
                    parent->deallocate(original_memory, 0, 0);
                }

                _trusted_memory = nullptr;
            }

            allocator_boundary_tags temp(other);

            std::swap(_trusted_memory, temp._trusted_memory);
            std::swap(_logger, temp._logger);

            _current_fit_mode = temp._current_fit_mode;

            if (get_logger() != nullptr) {
                get_logger()->log("[DEBUG copy assignment] Assignment completed successfully", logger::severity::debug);
            }
        }
        catch (const std::exception& e) {
            if (get_logger() != nullptr) {
                get_logger()->log("Exception in assignment operator: " + std::string(e.what()), logger::severity::error);
            }
            _trusted_memory = nullptr;
            _logger = nullptr;
        }
    }
    return *this;
}

bool allocator_boundary_tags::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    return this == &other;
}

bool allocator_boundary_tags::boundary_iterator::operator==(
        const allocator_boundary_tags::boundary_iterator &other) const noexcept
{
    return _occupied_ptr == other._occupied_ptr && _trusted_memory == other._trusted_memory;
}

bool allocator_boundary_tags::boundary_iterator::operator!=(
        const allocator_boundary_tags::boundary_iterator & other) const noexcept
{
    return !(*this == other);
}

allocator_boundary_tags::boundary_iterator &allocator_boundary_tags::boundary_iterator::operator++() & noexcept
{
    if (_occupied_ptr != nullptr) {
        block_header* block = static_cast<block_header*>(_occupied_ptr);
        _occupied_ptr = block->next_block;
        if (_occupied_ptr != nullptr) {
            _occupied = is_block_occupied(static_cast<block_header*>(_occupied_ptr));
        }
    }
    return *this;
}

allocator_boundary_tags::boundary_iterator &allocator_boundary_tags::boundary_iterator::operator--() & noexcept
{
    if (_occupied_ptr != nullptr) {
        block_header* block = static_cast<block_header*>(_occupied_ptr);
        _occupied_ptr = block->prev_block;
        if (_occupied_ptr != nullptr) {
            _occupied = is_block_occupied(static_cast<block_header*>(_occupied_ptr));
        }
    }
    return *this;
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::boundary_iterator::operator++(int n)
{
    boundary_iterator temp = *this;
    ++(*this);
    return temp;
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::boundary_iterator::operator--(int n)
{
    boundary_iterator temp = *this;
    --(*this);
    return temp;
}

size_t allocator_boundary_tags::boundary_iterator::size() const noexcept
{
    if (_occupied_ptr == nullptr) {
        return 0;
    }

    block_header* block = static_cast<block_header*>(_occupied_ptr);
    return get_block_size(block) - sizeof(block_header);
}

bool allocator_boundary_tags::boundary_iterator::occupied() const noexcept
{
    return _occupied;
}

void* allocator_boundary_tags::boundary_iterator::operator*() const noexcept
{
    if (_occupied_ptr == nullptr) {
        return nullptr;
    }

    return reinterpret_cast<char*>(_occupied_ptr) + sizeof(block_header);
}

allocator_boundary_tags::boundary_iterator::boundary_iterator()
    : _occupied_ptr(nullptr), _occupied(false), _trusted_memory(nullptr)
{
}

allocator_boundary_tags::boundary_iterator::boundary_iterator(void *trusted)
    : _occupied_ptr(trusted), _trusted_memory(trusted)
{
    if (_occupied_ptr != nullptr) {
        _occupied = is_block_occupied(static_cast<block_header*>(_occupied_ptr));
    } else {
        _occupied = false;
    }
}

void* allocator_boundary_tags::boundary_iterator::get_ptr() const noexcept
{
    return _occupied_ptr;
}
