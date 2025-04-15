#include <stdexcept>
#include <algorithm>
#include <vector>
#include <string>
#include <cmath>
#include <cstring>
#include <new>
#include <cstdint>
#include <random>
#include "../include/allocator_buddies_system.h"

namespace {

struct alignas(alignof(std::max_align_t)) allocator_metadata {
    logger* logger_ptr;
    std::pmr::memory_resource* parent_allocator;
    allocator_with_fit_mode::fit_mode fit;
    unsigned char k;
    std::mutex mutex;
    size_t total_allocated_size;
    uint32_t allocator_id;

    allocator_metadata() = default;
    ~allocator_metadata() = default;
};

allocator_metadata* get_metadata(void* trusted_memory) {
    return reinterpret_cast<allocator_metadata*>(trusted_memory);
}

size_t calculate_allocator_metadata_size() {
    return sizeof(allocator_metadata);
}

void* get_pool_start(void* trusted_memory) {
    return static_cast<char*>(trusted_memory) + calculate_allocator_metadata_size();
}

void* get_block_metadata(void* block) {
    if (block == nullptr) {
        return nullptr;
    }

    if (reinterpret_cast<uintptr_t>(block) < sizeof(uint32_t)) {
        return nullptr;
    }

    void* meta = static_cast<char*>(block) - sizeof(uint32_t);

    try {
        volatile uint32_t touch = *static_cast<uint32_t*>(meta);
        (void)touch;
    } catch (...) {
        return nullptr;
    }

    return meta;
}

size_t get_block_size(void* block_meta) {
    if (block_meta == nullptr) {
        return 0;
    }

    try {
        uint32_t meta = *static_cast<uint32_t*>(block_meta);
        return (meta >> 1) & 0x7F; // Биты 1-7 для size
    } catch (...) {
        return 0;
    }
}

bool is_block_occupied(void* block_meta) {
    if (block_meta == nullptr) {
        return false;
    }

    try {
        uint32_t meta = *static_cast<uint32_t*>(block_meta);
        return meta & 0x1; // Бит 0 для occupied
    } catch (...) {
        return false;
    }
}

void set_block_occupied(void* block_meta, bool occupied) {
    if (block_meta == nullptr) return;
    uint32_t meta = *static_cast<uint32_t*>(block_meta);
    meta = (meta & ~0x1) | (occupied ? 0x1 : 0);
    *static_cast<uint32_t*>(block_meta) = meta;
}

void set_block_size(void* block_meta, size_t size) {
    if (block_meta == nullptr) return;
    uint32_t meta = *static_cast<uint32_t*>(block_meta);
    meta = (meta & ~0xFE) | ((static_cast<uint32_t>(size) & 0x7F) << 1);
    *static_cast<uint32_t*>(block_meta) = meta;
}

void set_block_metadata(void* block_meta, bool occupied, size_t size, uint32_t allocator_id) {
    if (block_meta == nullptr) return;
    uint32_t meta = 0;
    meta |= (occupied ? 0x1 : 0);
    meta |= (static_cast<uint32_t>(size) & 0x7F) << 1;
    meta |= (allocator_id & 0xFFFFFF) << 8;
    *static_cast<uint32_t*>(block_meta) = meta;
}

uint32_t get_block_allocator_id(void* block_meta) {
    if (block_meta == nullptr) {
        return 0;
    }

    try {
        uint32_t meta = *static_cast<uint32_t*>(block_meta);
        return (meta >> 8) & 0xFFFFFF; // Биты 8-31 для allocator_id
    } catch (...) {
        return 0;
    }
}

void* get_buddy(void* block, size_t block_size, void* pool_start, size_t total_size) {
    size_t offset = static_cast<char*>(block) - static_cast<char*>(pool_start);
    size_t buddy_offset = offset ^ block_size;
    if (buddy_offset >= total_size) return nullptr;
    return static_cast<char*>(pool_start) + buddy_offset;
}

size_t next_power_of_two(size_t size) {
    if (size == 0) return 1;
    size--;
    size |= size >> 1;
    size |= size >> 2;
    size |= size >> 4;
    size |= size >> 8;
    size |= size >> 16;
    size |= size >> 32;
    return size + 1;
}

size_t nearest_greater_k_of_2(size_t size) {
    size_t k = 0;
    while ((1ULL << k) < size) {
        ++k;
    }
    return k;
}

uint32_t generate_unique_id() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint32_t> dis(1, 0xFFFFFF); // Ограничено 24 битами
    return dis(gen);
}

} // namespace

allocator_buddies_system::~allocator_buddies_system() {
    if (!_trusted_memory) return;
    auto* meta = get_metadata(_trusted_memory);
    if (_trusted_memory) {
        meta->~allocator_metadata();
    }
    if (meta && meta->logger_ptr) {
        meta->logger_ptr->log("~allocator_buddies_system() called", logger::severity::debug);
    }
    if (_trusted_memory && meta && meta->parent_allocator) {
        meta->parent_allocator->deallocate(_trusted_memory, meta->total_allocated_size);
    }
    _trusted_memory = nullptr;
}

allocator_buddies_system::allocator_buddies_system(allocator_buddies_system&& other) noexcept
    : _trusted_memory(other._trusted_memory) {
    other._trusted_memory = nullptr;
    if (_trusted_memory) {
        auto* meta = get_metadata(_trusted_memory);
        if (meta->logger_ptr) {
            meta->logger_ptr->log("allocator_buddies_system move constructor called", logger::severity::debug);
        }
    }
}

allocator_buddies_system& allocator_buddies_system::operator=(allocator_buddies_system&& other) noexcept {
    if (this != &other) {
        auto* meta = _trusted_memory ? get_metadata(_trusted_memory) : nullptr;
        if (meta && meta->logger_ptr) {
            meta->logger_ptr->log("allocator_buddies_system move assignment called", logger::severity::debug);
        }
        this->~allocator_buddies_system();
        _trusted_memory = other._trusted_memory;
        other._trusted_memory = nullptr;
    }
    return *this;
}

allocator_buddies_system::allocator_buddies_system(
    size_t space_size,
    std::pmr::memory_resource* parent_allocator,
    logger* logger,
    allocator_with_fit_mode::fit_mode allocate_fit_mode)
    : _trusted_memory(nullptr) {
    size_t pool_size = next_power_of_two(space_size);
    size_t pool_k = nearest_greater_k_of_2(pool_size);
    if (pool_k < min_k) {
        throw std::logic_error("Pool size too small for allocator");
    }

    size_t allocator_meta_size = calculate_allocator_metadata_size();
    size_t total_alloc_size = pool_size + allocator_meta_size;

    std::pmr::memory_resource* parent = parent_allocator ? parent_allocator : std::pmr::get_default_resource();
    _trusted_memory = parent->allocate(total_alloc_size);
    if (!_trusted_memory) {
        throw std::bad_alloc();
    }

    new (_trusted_memory) allocator_metadata();

    auto* meta = get_metadata(_trusted_memory);
    meta->logger_ptr = logger;
    meta->parent_allocator = parent;
    meta->fit = allocate_fit_mode;
    meta->k = static_cast<unsigned char>(pool_k);
    meta->total_allocated_size = total_alloc_size;
    meta->allocator_id = generate_unique_id();

    if (meta->logger_ptr) {
        meta->logger_ptr->log("allocator_buddies_system constructor called", logger::severity::debug);
        meta->logger_ptr->log("pool_k (space_size_power_of_two): " + std::to_string(pool_k), logger::severity::debug);
        meta->logger_ptr->log("pool_size: " + std::to_string(pool_size), logger::severity::debug);
        meta->logger_ptr->log("total_allocated_size: " + std::to_string(total_alloc_size), logger::severity::debug);
        meta->logger_ptr->log("min_k (from header): " + std::to_string(min_k), logger::severity::debug);
        meta->logger_ptr->log("allocator_id: " + std::to_string(meta->allocator_id), logger::severity::debug);
    }

    void* pool_start = get_pool_start(_trusted_memory);

    if (meta->logger_ptr) {
        meta->logger_ptr->log("[DEBUG constructor] Initializing first block metadata.", logger::severity::debug);
    }

    void* block_meta = pool_start;
    set_block_metadata(block_meta, false, pool_k, meta->allocator_id);

    if (meta->logger_ptr) {
        meta->logger_ptr->log("[DEBUG constructor] First block initialized with size: " + std::to_string(pool_k) +
                             ", allocator_id: " + std::to_string(meta->allocator_id), logger::severity::debug);
    }
}

void* allocator_buddies_system::do_allocate_sm(size_t size) {
    if (_trusted_memory == nullptr) {
        throw std::bad_alloc();
    }

    auto* meta = get_metadata(_trusted_memory);
    if (meta == nullptr) {
        throw std::bad_alloc();
    }

    std::lock_guard<std::mutex> lock(meta->mutex);
    if (meta->logger_ptr) {
        meta->logger_ptr->log("do_allocate_sm called with size: " + std::to_string(size), logger::severity::debug);
    }

    size_t adjusted_size = size + sizeof(void*) + sizeof(uint32_t);
    size_t k = nearest_greater_k_of_2(adjusted_size);
    k = std::max(k, static_cast<size_t>(min_k));
    if (k > meta->k) {
        if (meta->logger_ptr) {
            meta->logger_ptr->log("Requested size too large", logger::severity::error);
        }
        throw std::bad_alloc();
    }

    if (meta->logger_ptr) {
        meta->logger_ptr->log("[DEBUG do_allocate_sm] Adjusted size: " + std::to_string(adjusted_size) +
                             ", Calculated k: " + std::to_string(k) +
                             ", min_k: " + std::to_string(min_k), logger::severity::debug);
    }

    buddy_iterator best_block = end();
    size_t best_size = meta->fit == fit_mode::the_worst_fit ? 0 : std::numeric_limits<size_t>::max();

    buddy_iterator it_begin = begin();
    buddy_iterator it_end = end();

    try {
        for (auto it = it_begin; it != it_end; ++it) {
            void* block_ptr = *it;
            if (block_ptr == nullptr) {
                if (meta->logger_ptr) {
                    meta->logger_ptr->log("[DEBUG do_allocate_sm] Skipping null block pointer", logger::severity::debug);
                }
                continue;
            }

            void* block_meta = get_block_metadata(block_ptr);
            if (block_meta == nullptr) {
                if (meta->logger_ptr) {
                    meta->logger_ptr->log("[DEBUG do_allocate_sm] Invalid block metadata", logger::severity::debug);
                }
                continue;
            }

            size_t block_size = 1ULL << it.size();
            void* block_end = static_cast<char*>(block_meta) + block_size;
            void* pool_end = static_cast<char*>(get_pool_start(_trusted_memory)) + (1ULL << meta->k);
            if (block_end > pool_end) {
                if (meta->logger_ptr) {
                    meta->logger_ptr->log("[DEBUG do_allocate_sm] Block exceeds pool boundary", logger::severity::debug);
                }
                continue;
            }

            if (it.occupied() || it.size() < k) {
                if (meta->logger_ptr) {
                    meta->logger_ptr->log("[DEBUG do_allocate_sm] Block occupied or too small: size=" +
                                         std::to_string(it.size()) + ", required=" + std::to_string(k),
                                         logger::severity::debug);
                }
                continue;
            }

            if (meta->fit == fit_mode::first_fit) {
                best_block = it;
                best_size = block_size;
                if (meta->logger_ptr) {
                    meta->logger_ptr->log("[DEBUG do_allocate_sm] Found first fit block, size: " +
                                         std::to_string(block_size), logger::severity::debug);
                }
                break;
            }
            if (meta->fit == fit_mode::the_best_fit && block_size < best_size) {
                best_block = it;
                best_size = block_size;
                if (meta->logger_ptr) {
                    meta->logger_ptr->log("[DEBUG do_allocate_sm] Found better fit block, size: " +
                                         std::to_string(block_size), logger::severity::debug);
                }
            }
            if (meta->fit == fit_mode::the_worst_fit && block_size > best_size) {
                best_block = it;
                best_size = block_size;
                if (meta->logger_ptr) {
                    meta->logger_ptr->log("[DEBUG do_allocate_sm] Found worse fit block, size: " +
                                         std::to_string(block_size), logger::severity::debug);
                }
            }
        }
    } catch (const std::exception& e) {
        if (meta->logger_ptr) {
            meta->logger_ptr->log(std::string("Exception during block search: ") + e.what(), logger::severity::error);
        }
        throw std::bad_alloc();
    }

    void* best_block_ptr = *best_block;
    if (best_block == end() || best_block_ptr == nullptr) {
        if (meta->logger_ptr) {
            meta->logger_ptr->log("No suitable block found", logger::severity::error);
        }
        throw std::bad_alloc();
    }

    void* block = best_block_ptr;
    void* block_meta = get_block_metadata(block);
    if (block_meta == nullptr) {
        if (meta->logger_ptr) {
            meta->logger_ptr->log("[DEBUG do_allocate_sm] Best block has invalid metadata", logger::severity::error);
        }
        throw std::bad_alloc();
    }

    size_t current_k = get_block_size(block_meta);
    if (meta->logger_ptr) {
        meta->logger_ptr->log("[DEBUG do_allocate_sm] Starting block split. Initial block k: " +
                             std::to_string(current_k) + ", Target k: " + std::to_string(k),
                             logger::severity::debug);
    }

    while (current_k > k) {
        current_k--;
        size_t split_size = 1ULL << current_k;
        set_block_size(block_meta, current_k);

        void* current_block_start = static_cast<char*>(block) - sizeof(uint32_t);
        void* buddy_block_start = static_cast<char*>(current_block_start) + split_size;
        set_block_metadata(buddy_block_start, false, current_k, meta->allocator_id);

        if (meta->logger_ptr) {
            meta->logger_ptr->log("[DEBUG do_allocate_sm] Split block. New k: " + std::to_string(current_k) +
                                 ", Split size: " + std::to_string(split_size) +
                                 ", Allocator ID: " + std::to_string(meta->allocator_id), logger::severity::debug);
        }
    }

    set_block_occupied(block_meta, true);

    auto* block_ptr_storage = static_cast<char*>(block);
    *reinterpret_cast<void**>(block_ptr_storage) = block;

    void* user_ptr = block_ptr_storage + sizeof(void*);

    if (meta->logger_ptr) {
        size_t available = 0;
        for (auto it = begin(); it != end(); ++it) {
            void* block_ptr = *it;
            if (block_ptr != nullptr && !it.occupied()) {
                available += 1ULL << it.size();
            }
        }
        meta->logger_ptr->log("Available memory after allocation: " + std::to_string(available),
                             logger::severity::information);

        std::string blocks_state;
        auto blocks = get_blocks_info_inner();
        for (const auto& b : blocks) {
            blocks_state += (b.is_block_occupied ? "occup " : "avail ") + std::to_string(b.block_size) + "|";
        }
        if (!blocks_state.empty()) blocks_state.pop_back();
        meta->logger_ptr->log("Blocks state: " + blocks_state, logger::severity::debug);
    }

    return user_ptr;
}

void allocator_buddies_system::do_deallocate_sm(void* at) {
    if (_trusted_memory == nullptr || at == nullptr) {
        if (_trusted_memory) {
            auto* meta = get_metadata(_trusted_memory);
            if (meta->logger_ptr) {
                meta->logger_ptr->log("Invalid pointer for deallocation: null pointer", logger::severity::error);
            }
        }
        throw std::invalid_argument("Invalid pointer for deallocation");
    }

    auto* meta = get_metadata(_trusted_memory);
    if (meta == nullptr) {
        throw std::invalid_argument("Invalid allocator state");
    }

    std::lock_guard<std::mutex> lock(meta->mutex);
    if (meta->logger_ptr) {
        meta->logger_ptr->log("do_deallocate_sm called", logger::severity::debug);
    }

    void* block_ptr_storage = static_cast<char*>(at) - sizeof(void*);
    void* block = *reinterpret_cast<void**>(block_ptr_storage);

    if (block == nullptr) {
        if (meta->logger_ptr) {
            meta->logger_ptr->log("Invalid block pointer", logger::severity::error);
        }
        throw std::invalid_argument("Invalid block pointer");
    }

    void* pool_start = get_pool_start(_trusted_memory);
    if (pool_start == nullptr) {
        if (meta->logger_ptr) {
            meta->logger_ptr->log("Invalid pool start", logger::severity::error);
        }
        throw std::invalid_argument("Invalid pool start");
    }

    size_t total_size = 1ULL << meta->k;
    if (block < pool_start || block >= static_cast<char*>(pool_start) + total_size) {
        if (meta->logger_ptr) {
            meta->logger_ptr->log("Invalid pointer for deallocation", logger::severity::error);
        }
        throw std::invalid_argument("Pointer does not belong to this allocator");
    }

    void* block_meta = get_block_metadata(block);
    if (block_meta == nullptr) {
        if (meta->logger_ptr) {
            meta->logger_ptr->log("Invalid block metadata", logger::severity::error);
        }
        throw std::invalid_argument("Invalid block metadata");
    }

    if (get_block_allocator_id(block_meta) != meta->allocator_id) {
        if (meta->logger_ptr) {
            meta->logger_ptr->log("Block does not belong to this allocator (ID mismatch)", logger::severity::error);
        }
        throw std::invalid_argument("Block does not belong to this allocator");
    }

    if (!is_block_occupied(block_meta)) {
        if (meta->logger_ptr) {
            meta->logger_ptr->log("Block already free", logger::severity::error);
        }
        throw std::invalid_argument("Block is not occupied");
    }

    set_block_occupied(block_meta, false);
    size_t current_k = get_block_size(block_meta);
    if (meta->logger_ptr) {
        meta->logger_ptr->log("[DEBUG do_deallocate_sm] Block freed, current k: " + std::to_string(current_k),
                             logger::severity::debug);
    }

    while (current_k < meta->k) {
        size_t block_size = 1ULL << current_k;
        void* buddy = get_buddy(block, block_size, pool_start, total_size);
        if (!buddy) {
            if (meta->logger_ptr) {
                meta->logger_ptr->log("[DEBUG do_deallocate_sm] No buddy found", logger::severity::debug);
            }
            break;
        }

        void* buddy_meta = get_block_metadata(buddy);
        if (buddy_meta == nullptr) {
            if (meta->logger_ptr) {
                meta->logger_ptr->log("[DEBUG do_deallocate_sm] Invalid buddy metadata", logger::severity::debug);
            }
            break;
        }

        if (is_block_occupied(buddy_meta) || get_block_size(buddy_meta) != current_k ||
            get_block_allocator_id(buddy_meta) != meta->allocator_id) {
            if (meta->logger_ptr) {
                meta->logger_ptr->log("[DEBUG do_deallocate_sm] Buddy occupied, different size, or different allocator",
                                     logger::severity::debug);
            }
            break;
        }

        block = std::min(block, buddy);
        block_meta = get_block_metadata(block);
        if (block_meta == nullptr) {
            if (meta->logger_ptr) {
                meta->logger_ptr->log("[DEBUG do_deallocate_sm] Invalid block metadata after merge", logger::severity::debug);
            }
            break;
        }

        set_block_size(block_meta, ++current_k);
        if (meta->logger_ptr) {
            meta->logger_ptr->log("[DEBUG do_deallocate_sm] Merged with buddy, new k: " + std::to_string(current_k),
                                 logger::severity::debug);
        }
    }

    if (meta->logger_ptr) {
        size_t available = 0;
        for (auto it = begin(); it != end(); ++it) {
            void* block_ptr = *it;
            if (block_ptr != nullptr && !it.occupied()) {
                available += 1ULL << it.size();
            }
        }
        meta->logger_ptr->log("Available memory after deallocation: " + std::to_string(available),
                             logger::severity::information);

        std::string blocks_state;
        auto blocks = get_blocks_info_inner();
        for (const auto& b : blocks) {
            blocks_state += (b.is_block_occupied ? "occup " : "avail ") + std::to_string(b.block_size) + "|";
        }
        if (!blocks_state.empty()) blocks_state.pop_back();
        meta->logger_ptr->log("Blocks state: " + blocks_state, logger::severity::debug);
    }
}

allocator_buddies_system::allocator_buddies_system(const allocator_buddies_system& other) {
    if (!other._trusted_memory) {
        _trusted_memory = nullptr;
        return;
    }
    auto* other_meta = get_metadata(other._trusted_memory);
    std::lock_guard<std::mutex> lock(other_meta->mutex);
    if (other_meta->logger_ptr) {
        other_meta->logger_ptr->log("allocator_buddies_system copy constructor called", logger::severity::debug);
    }

    size_t total_alloc_size_to_copy = other_meta->total_allocated_size;
    _trusted_memory = other_meta->parent_allocator->allocate(total_alloc_size_to_copy);
    if (!_trusted_memory) {
        throw std::bad_alloc();
    }

    std::memcpy(_trusted_memory, other._trusted_memory, total_alloc_size_to_copy);

    auto* meta = get_metadata(_trusted_memory);
    meta->allocator_id = generate_unique_id();
    new (&meta->mutex) std::mutex();

    void* pool_start = get_pool_start(_trusted_memory);
    void* current_block_meta = pool_start;
    void* pool_end = static_cast<char*>(pool_start) + (1ULL << meta->k);
    while (current_block_meta < pool_end) {
        uint32_t block_meta_value = *static_cast<uint32_t*>(current_block_meta);
        block_meta_value = (block_meta_value & 0xFF) | (meta->allocator_id << 8);
        *static_cast<uint32_t*>(current_block_meta) = block_meta_value;
        size_t block_size = 1ULL << ((block_meta_value >> 1) & 0x7F);
        current_block_meta = static_cast<char*>(current_block_meta) + block_size;
    }
}

allocator_buddies_system& allocator_buddies_system::operator=(const allocator_buddies_system& other) {
    if (this == &other) {
        return *this;
    }

    auto* meta = _trusted_memory ? get_metadata(_trusted_memory) : nullptr;
    auto* other_meta = other._trusted_memory ? get_metadata(other._trusted_memory) : nullptr;

    if (meta && other_meta) {
        if (meta < other_meta) {
            meta->mutex.lock();
            other_meta->mutex.lock();
        } else {
            other_meta->mutex.lock();
            meta->mutex.lock();
        }
    } else if (meta) {
        meta->mutex.lock();
    } else if (other_meta) {
        other_meta->mutex.lock();
    }

    struct LockGuard {
        std::mutex* m1;
        std::mutex* m2;
        LockGuard(std::mutex* m1, std::mutex* m2) : m1(m1), m2(m2) {}
        ~LockGuard() {
            if (m1) m1->unlock();
            if (m2) m2->unlock();
        }
    } guard(meta ? &meta->mutex : nullptr, other_meta ? &other_meta->mutex : nullptr);

    if (meta && meta->logger_ptr) {
        meta->logger_ptr->log("allocator_buddies_system copy assignment called", logger::severity::debug);
    }

    this->~allocator_buddies_system();

    if (other._trusted_memory && other_meta) {
        size_t total_alloc_size_to_copy = other_meta->total_allocated_size;
        _trusted_memory = other_meta->parent_allocator->allocate(total_alloc_size_to_copy);
        if (!_trusted_memory) {
            throw std::bad_alloc();
        }
        std::memcpy(_trusted_memory, other._trusted_memory, total_alloc_size_to_copy);

        auto* new_meta = get_metadata(_trusted_memory);
        new_meta->allocator_id = generate_unique_id();
        new (&new_meta->mutex) std::mutex();

        void* pool_start = get_pool_start(_trusted_memory);
        void* current_block_meta = pool_start;
        void* pool_end = static_cast<char*>(pool_start) + (1ULL << new_meta->k);
        while (current_block_meta < pool_end) {
            uint32_t block_meta_value = *static_cast<uint32_t*>(current_block_meta);
            block_meta_value = (block_meta_value & 0xFF) | (new_meta->allocator_id << 8);
            *static_cast<uint32_t*>(current_block_meta) = block_meta_value;
            size_t block_size = 1ULL << ((block_meta_value >> 1) & 0x7F);
            current_block_meta = static_cast<char*>(current_block_meta) + block_size;
        }
    } else {
        _trusted_memory = nullptr;
    }

    return *this;
}

bool allocator_buddies_system::do_is_equal(const std::pmr::memory_resource& other) const noexcept {
    const auto* other_ptr = dynamic_cast<const allocator_buddies_system*>(&other);
    return other_ptr && _trusted_memory == other_ptr->_trusted_memory;
}

void allocator_buddies_system::set_fit_mode(allocator_with_fit_mode::fit_mode mode) {
    auto* meta = get_metadata(_trusted_memory);
    std::lock_guard<std::mutex> lock(meta->mutex);
    if (meta->logger_ptr) {
        meta->logger_ptr->log("set_fit_mode called", logger::severity::debug);
    }
    meta->fit = mode;
}

std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info() const noexcept {
    auto* meta = get_metadata(_trusted_memory);
    std::lock_guard<std::mutex> lock(meta->mutex);
    return get_blocks_info_inner();
}

logger* allocator_buddies_system::get_logger() const {
    return get_metadata(_trusted_memory)->logger_ptr;
}

std::string allocator_buddies_system::get_typename() const {
    return "allocator_buddies_system";
}

std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info_inner() const {
    std::vector<allocator_test_utils::block_info> blocks;

    if (_trusted_memory == nullptr) {
        std::cerr << "[DEBUG get_blocks_info_inner] _trusted_memory is null. Returning empty." << std::endl;
        return blocks;
    }
    auto* meta = get_metadata(_trusted_memory);
    logger* log = meta ? meta->logger_ptr : nullptr;

    if (log) log->log("[DEBUG get_blocks_info_inner] Entered function.", logger::severity::debug);
    else std::cerr << "[DEBUG get_blocks_info_inner] Entered function (logger unavailable)." << std::endl;

    if (meta == nullptr) {
        if (log) log->log("[DEBUG get_blocks_info_inner] Allocator metadata is null. Returning empty.", logger::severity::error);
        else std::cerr << "[DEBUG get_blocks_info_inner] Allocator metadata is null. Returning empty." << std::endl;
        return blocks;
    }
    void* pool_start = get_pool_start(_trusted_memory);
    if (pool_start == nullptr) {
        if (log) log->log("[DEBUG get_blocks_info_inner] Pool start is null. Returning empty.", logger::severity::error);
        else std::cerr << "[DEBUG get_blocks_info_inner] Pool start is null. Returning empty." << std::endl;
        return blocks;
    }

    void* current_block_meta = pool_start;
    void* pool_end = static_cast<char*>(pool_start) + (1ULL << meta->k);

    if (log) {
        log->log("[DEBUG get_blocks_info_inner] Pool start: " + std::to_string(reinterpret_cast<uintptr_t>(pool_start)) +
                 ", Pool end: " + std::to_string(reinterpret_cast<uintptr_t>(pool_end)) +
                 ", Pool k: " + std::to_string(meta->k) +
                 ", Min k: " + std::to_string(min_k), logger::severity::debug);
    } else {
        std::cerr << "[DEBUG get_blocks_info_inner] Pool start: " << pool_start
                  << ", Pool end: " << pool_end
                  << ", Pool k: " << meta->k
                  << ", Min k: " << min_k << std::endl;
    }

    int block_count = 0;
    while (current_block_meta < pool_end) {
        block_count++;
        if (log) log->log("[DEBUG get_blocks_info_inner] Iteration " + std::to_string(block_count) +
                         ": current_meta = " + std::to_string(reinterpret_cast<uintptr_t>(current_block_meta)),
                         logger::severity::debug);
        else std::cerr << "[DEBUG get_blocks_info_inner] Iteration " << block_count
                      << ": current_meta = " << current_block_meta << std::endl;

        try {
            volatile unsigned char touch = static_cast<block_metadata*>(current_block_meta)->size;
            (void)touch;
        } catch (...) {
            if (log) log->log("[DEBUG get_blocks_info_inner] Cannot read block metadata byte. Exiting loop.", logger::severity::warning);
            else std::cerr << "[DEBUG get_blocks_info_inner] Cannot read block metadata byte. Exiting loop." << std::endl;
            break;
        }

        size_t block_k = get_block_size(current_block_meta);
        if (log) log->log("[DEBUG get_blocks_info_inner]   Read block_k = " + std::to_string(block_k), logger::severity::debug);
        else std::cerr << "[DEBUG get_blocks_info_inner]   Read block_k = " << block_k << std::endl;

        if (block_k > meta->k || block_k < min_k) {
            if (log) log->log("[DEBUG get_blocks_info_inner] Invalid block k detected (block_k=" + std::to_string(block_k) +
                             ", meta->k=" + std::to_string(meta->k) + ", min_k=" + std::to_string(min_k) +
                             "). Exiting loop.", logger::severity::warning);
            else std::cerr << "[DEBUG get_blocks_info_inner] Invalid block k detected (block_k=" << block_k
                          << ", meta->k=" << static_cast<int>(meta->k)
                          << ", min_k=" << min_k
                          << "). Exiting loop." << std::endl;
            break;
        }

        size_t block_size = 1ULL << block_k;
        if (log) log->log("[DEBUG get_blocks_info_inner]   Calculated block_size = " + std::to_string(block_size), logger::severity::debug);
        else std::cerr << "[DEBUG get_blocks_info_inner]   Calculated block_size = " << block_size << std::endl;

        if (static_cast<char*>(current_block_meta) + block_size > static_cast<char*>(pool_end)) {
            if (log) log->log("[DEBUG get_blocks_info_inner] Block exceeds pool boundary. Exiting loop.", logger::severity::warning);
            else std::cerr << "[DEBUG get_blocks_info_inner] Block exceeds pool boundary. Exiting loop." << std::endl;
            break;
        }

        bool is_occupied = is_block_occupied(current_block_meta);
        if (log) log->log("[DEBUG get_blocks_info_inner]   Is occupied = " + std::string(is_occupied ? "true" : "false"), logger::severity::debug);
        else std::cerr << "[DEBUG get_blocks_info_inner]   Is occupied = " << (is_occupied ? "true" : "false") << std::endl;

        blocks.push_back({block_size, is_occupied});
        if (log) log->log("[DEBUG get_blocks_info_inner]   Block added to vector.", logger::severity::debug);
        else std::cerr << "[DEBUG get_blocks_info_inner]   Block added to vector." << std::endl;

        void* next_block_meta = static_cast<char*>(current_block_meta) + block_size;
        if (log) log->log("[DEBUG get_blocks_info_inner]   Next meta = " +
                         std::to_string(reinterpret_cast<uintptr_t>(next_block_meta)),
                         logger::severity::debug);
        else std::cerr << "[DEBUG get_blocks_info_inner]   Next meta = " << next_block_meta << std::endl;
        current_block_meta = next_block_meta;
    }

    if (current_block_meta != pool_end) {
        if (log) log->log("[DEBUG get_blocks_info_inner] Final metadata pointer (" +
                         std::to_string(reinterpret_cast<uintptr_t>(current_block_meta)) +
                         ") does not match pool end (" +
                         std::to_string(reinterpret_cast<uintptr_t>(pool_end)) +
                         "). Potential error.", logger::severity::warning);
        else std::cerr << "[DEBUG get_blocks_info_inner] Final metadata pointer (" << current_block_meta
                      << ") does not match pool end (" << pool_end << "). Potential error." << std::endl;
    }

    if (log) log->log("[DEBUG get_blocks_info_inner] Exiting function. Found " + std::to_string(blocks.size()) + " blocks.", logger::severity::debug);
    else std::cerr << "[DEBUG get_blocks_info_inner] Exiting function. Found " << blocks.size() << " blocks." << std::endl;

    return blocks;
}

allocator_buddies_system::buddy_iterator::buddy_iterator() : _block(nullptr) {}

allocator_buddies_system::buddy_iterator::buddy_iterator(void* start) : _block(start) {}

bool allocator_buddies_system::buddy_iterator::operator==(const buddy_iterator& other) const noexcept {
    return _block == other._block;
}

bool allocator_buddies_system::buddy_iterator::operator!=(const buddy_iterator& other) const noexcept {
    return _block != other._block;
}

allocator_buddies_system::buddy_iterator& allocator_buddies_system::buddy_iterator::operator++() & noexcept {
    if (_block == nullptr) {
        return *this;
    }

    void* block_meta = get_block_metadata(_block);
    if (block_meta == nullptr) {
        _block = nullptr;
        return *this;
    }

    size_t block_size = 1ULL << get_block_size(block_meta);
    if (block_size == 0) {
        _block = nullptr;
        return *this;
    }

    void* current_block = static_cast<char*>(_block) - sizeof(block_metadata);
    void* next_block = static_cast<char*>(current_block) + block_size;
    _block = static_cast<char*>(next_block) + sizeof(block_metadata);

    return *this;
}

allocator_buddies_system::buddy_iterator allocator_buddies_system::buddy_iterator::operator++(int) {
    buddy_iterator temp = *this;
    ++(*this);
    return temp;
}

size_t allocator_buddies_system::buddy_iterator::size() const noexcept {
    if (_block == nullptr) {
        return 0;
    }

    void* block_meta = get_block_metadata(_block);
    if (block_meta == nullptr) {
        return 0;
    }

    try {
        return get_block_size(block_meta);
    } catch (...) {
        return 0;
    }
}

bool allocator_buddies_system::buddy_iterator::occupied() const noexcept {
    if (_block == nullptr) {
        return false;
    }

    void* block_meta = get_block_metadata(_block);
    if (block_meta == nullptr) {
        return false;
    }

    try {
        return is_block_occupied(block_meta);
    } catch (...) {
        return false;
    }
}

void* allocator_buddies_system::buddy_iterator::operator*() const noexcept {
    return _block;
}

allocator_buddies_system::buddy_iterator allocator_buddies_system::begin() const noexcept {
    if (_trusted_memory == nullptr) {
        return buddy_iterator(nullptr);
    }
    void* pool_start = get_pool_start(_trusted_memory);
    if (pool_start == nullptr) {
        return buddy_iterator(nullptr);
    }
    return buddy_iterator(static_cast<char*>(pool_start) + sizeof(block_metadata));
}

allocator_buddies_system::buddy_iterator allocator_buddies_system::end() const noexcept {
    if (_trusted_memory == nullptr) {
        return buddy_iterator(nullptr);
    }
    auto* meta = get_metadata(_trusted_memory);
    if (meta == nullptr) {
        return buddy_iterator(nullptr);
    }
    void* pool_start = get_pool_start(_trusted_memory);
    void* pool_end_address = static_cast<char*>(pool_start) + (1ULL << meta->k);
    return buddy_iterator(static_cast<char*>(pool_end_address) + sizeof(block_metadata));
}