#include <not_implemented.h>
#include "../include/allocator_boundary_tags.h"

allocator_boundary_tags::~allocator_boundary_tags()
{
    
    logger* logger_instance = get_logger();
    logger_instance->debug(get_typename() + "::~allocator_boundary_tags() : called.");
    
    if (_trusted_memory == nullptr)
    {
        if (logger_instance)
        {
            logger_instance->error(get_typename() +
                                   "::~allocator_boundary_tags() : no memory to deallocate.");
        }
        return;
    }
    logger_instance->trace(get_typename() +
                           "::~allocator_boundary_tags() : destroy mutex.");
    get_mutex().~mutex();

    try
    {
    
        auto* parent_allocator = get_parent_resource();

    
        size_t total_size = allocator_metadata_size + reinterpret_cast<size_t>((char*)_trusted_memory + sizeof(logger*) + sizeof(memory_resource*) + sizeof(allocator_with_fit_mode::fit_mode));

    
        if (parent_allocator != nullptr)
        {
            parent_allocator->deallocate(_trusted_memory, total_size);
            if (logger_instance)
            {
                logger_instance->trace(get_typename() +
                                       "::~allocator_boundary_tags() : memory deallocated via parent allocator.");
            }
        }
        else
        {
            ::operator delete(_trusted_memory);
            if (logger_instance)
            {
                logger_instance->trace(get_typename() +
                                         "::~allocator_boundary_tags() : memory deallocated via global operator delete.");
            }
        }

        _trusted_memory = nullptr;
    }
    catch (const std::exception& ex)
    {
        if (logger_instance)
        {
            logger_instance->error(get_typename() +
                                   "::~allocator_boundary_tags() : deallocation failed - " +
                                   std::string(ex.what()));
        }
        _trusted_memory = nullptr;
    }

    if (logger_instance)
    {
        logger_instance->trace(get_typename() +
                               "::~allocator_boundary_tags() : successfully finished.");
    }
}

allocator_boundary_tags::allocator_boundary_tags(allocator_boundary_tags&& other) noexcept
        : _trusted_memory(other._trusted_memory)
{
    trace_with_guard(get_typename() + "::allocator_boundary_tags(allocator_boundary_tags&&) : called.");

    
    std::cout<<"lock mutex in &&other"<<std::endl;
    std::lock_guard<std::mutex> lock(other.get_mutex());

    
    other._trusted_memory = nullptr;

    if (auto* logger = get_logger())
    {
        logger->debug(get_typename() +
                      "::allocator_boundary_tags(allocator_boundary_tags&&) : resources moved successfully.");
    }

    trace_with_guard(get_typename() + "::allocator_boundary_tags(allocator_boundary_tags&&) : successfully finished.");
}

std::pmr::memory_resource* allocator_boundary_tags::get_parent_resource() const noexcept
{
    if (_trusted_memory == nullptr)
    {
        return nullptr;
    }
    return *reinterpret_cast<std::pmr::memory_resource**>(
            reinterpret_cast<std::byte*>(_trusted_memory) + sizeof(logger*));
}

allocator_boundary_tags& allocator_boundary_tags::operator=(allocator_boundary_tags&& other) noexcept
{
    trace_with_guard(get_typename() + "::operator=(allocator_boundary_tags&&) : called.");

    if (this != &other)
    {
        
        std::unique_lock<std::mutex> lock_this(get_mutex(), std::defer_lock);
        std::unique_lock<std::mutex> lock_other(other.get_mutex(), std::defer_lock);
        std::cout<<"lock mutex in operator="<<std::endl;
        std::lock(lock_this, lock_other);

        
        if (_trusted_memory != nullptr)
        {
            try
            {
                size_t total_size = *reinterpret_cast<size_t*>(
                        reinterpret_cast<std::byte*>(_trusted_memory) +
                        sizeof(logger*) +
                        sizeof(memory_resource*) +
                        sizeof(allocator_with_fit_mode::fit_mode));

                if (auto* parent_alloc = get_parent_resource())
                {
                    parent_alloc->deallocate(_trusted_memory, total_size);
                }
                else
                {
                    ::operator delete(_trusted_memory);
                }
            }
            catch (const std::exception& ex)
            {
                if (auto* logger = get_logger())
                {
                    logger->error(get_typename() +
                                  "::operator=(allocator_boundary_tags&&) : deallocation failed - " +
                                  std::string(ex.what()));
                }
            }
        }

        
        _trusted_memory = other._trusted_memory;
        other._trusted_memory = nullptr;

        if (auto* logger = get_logger())
        {
            logger->debug(get_typename() +
                          "::operator=(allocator_boundary_tags&&) : resources moved successfully.");
        }
    }

    trace_with_guard(get_typename() + "::operator=(allocator_boundary_tags&&) : successfully finished.");

    return *this;
}

/** If parent_allocator* == nullptr you should use std::pmr::get_default_resource()
 */
allocator_boundary_tags::allocator_boundary_tags(
        size_t space_size,
        std::pmr::memory_resource* parent_allocator,
        logger* logger,
        allocator_with_fit_mode::fit_mode allocate_fit_mode)
{

    logger->debug(get_typename() + "::allocator_boundary_tags() : called");

        
        if (space_size == 0) {
            logger->error(get_typename() + "::allocator_boundary_tags() : space size 0");
            throw std::invalid_argument("Space size cannot be zero");
        }
        try {
        
            parent_allocator = parent_allocator ? parent_allocator : std::pmr::get_default_resource();

        
            size_t total_size = allocator_metadata_size + space_size;

        
            if (logger) {
                logger->debug(get_typename() + "::allocator_boundary_tags() : " +
                              "Requesting " + std::to_string(total_size) + " bytes");
                logger->information(get_typename() + "::allocator_boundary_tags() : available " + std::to_string(space_size));
            }

            
            _trusted_memory = parent_allocator->allocate(total_size);

            auto *memory = reinterpret_cast<std::byte *>(_trusted_memory);

            
            
            *reinterpret_cast<class logger **>(memory) = logger;
            memory += sizeof(class logger *);

            
            *reinterpret_cast<std::pmr::memory_resource **>(memory) = parent_allocator;
            memory += sizeof(memory_resource * );

            
            *reinterpret_cast<allocator_with_fit_mode::fit_mode *>(memory) = allocate_fit_mode;
            memory += sizeof(allocator_with_fit_mode::fit_mode);

            
            *reinterpret_cast<size_t *>(memory) = space_size;
            memory += sizeof(size_t);

            
            new(reinterpret_cast<std::mutex *>(memory)) std::mutex();
            memory += sizeof(std::mutex);

            
            *reinterpret_cast<void **>(memory) = nullptr;
        }catch (const std::exception& e) {
            if (logger) logger->error("Failed init\n");
            throw;
        }
        logger->debug(get_typename() + "::allocator_boundary_tags() : finished");

}


allocator_with_fit_mode::fit_mode allocator_boundary_tags::get_fit_mode() const {
    unsigned char* memory = reinterpret_cast<unsigned char*>(_trusted_memory);
    memory += sizeof(logger*) + sizeof(memory_resource*);
    return *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(memory);
}

[[nodiscard]] void* allocator_boundary_tags::do_allocate_sm(size_t size)
{
    logger* logger = get_logger();
    logger->debug(get_typename() + "::do_allocate_sm(): called");

    const size_t total_size = size + occupied_block_metadata_size;
    size_t allocator_size = *reinterpret_cast<size_t*>(reinterpret_cast<char*>(_trusted_memory) +
                                                       sizeof(class logger*) + sizeof(memory_resource*) +
                                                       sizeof(allocator_with_fit_mode::fit_mode));

    if (total_size > allocator_size) {
        if (auto* logger = get_logger()) {
            logger->error(get_typename() + "::do_allocate_sm(): requested size " +
                          std::to_string(size) + " is too large (max available: " +
                          std::to_string(allocator_size - occupied_block_metadata_size) + ")");
        }
        throw std::bad_alloc();
    }


    void* allocated_memory = nullptr;
    switch (get_fit_mode()) {
        case fit_mode::first_fit:
            allocated_memory = allocate_first_fit(size);
            break;
        case fit_mode::the_best_fit:
            allocated_memory = allocate_best_fit(size);
            break;
        case fit_mode::the_worst_fit:
            allocated_memory = allocate_worst_fit(size);
            break;
        default:
            logger->error(get_typename() + "::do_allocate_sm(): Unknown fit mode ");
            throw std::invalid_argument("Unknown fit mode");
    }

    
    if (allocated_memory == nullptr) {
        if (auto* logger = get_logger()) {
            logger->error(get_typename() + "::do_allocate_sm(): allocation failed for size " +
                          std::to_string(size));
        }
        throw std::bad_alloc();
    }
    logger->debug(get_typename() + "::do_allocate_sm(): finished");

    return allocated_memory;
}


void* allocator_boundary_tags::allocate_new_block(char* where, size_t size, void** first_block_ptr, size_t size_free) {
    //std::lock_guard<std::mutex> guard(get_mutex());
    if(size_free - size - occupied_block_metadata_size < occupied_block_metadata_size){
        size = size_free - size - occupied_block_metadata_size;
    }
    
    *reinterpret_cast<size_t*>(where) = size;
    *reinterpret_cast<void**>(where + sizeof(size_t)) = nullptr;
    *reinterpret_cast<void**>(where + sizeof(size_t) + sizeof(void*)) = nullptr;
    *reinterpret_cast<void**>(where + sizeof(size_t) + 2*sizeof(void*)) = _trusted_memory; 

    
    *first_block_ptr = where;
    
    return where;
}

void* allocator_boundary_tags::allocate_in_hole(char* where, size_t size,
                                                void** first_block_ptr,
                                                void* prev_block, void* next_block, size_t size_free) {
    
    if(size_free - size - occupied_block_metadata_size < occupied_block_metadata_size){
        size += size_free - size - occupied_block_metadata_size;
    }
    
    *reinterpret_cast<size_t*>(where) = size;
    *reinterpret_cast<void**>(where + sizeof(size_t)) = next_block;
    *reinterpret_cast<void**>(where + sizeof(size_t) + sizeof(void*)) = prev_block;
    *reinterpret_cast<void**>(where + sizeof(size_t) + 2*sizeof(void*)) = _trusted_memory;

    
    if (prev_block != nullptr) {
        *reinterpret_cast<void**>((char*)prev_block + sizeof(size_t)) = where;
    } else {
        *first_block_ptr = where;
    }

    if (next_block != nullptr) {
        *reinterpret_cast<void**>((char*)next_block + sizeof(size_t) + sizeof(void*)) = where;
    }
    
    return where;
}

void* allocator_boundary_tags::allocate_first_fit(size_t size) {

    std::lock_guard<std::mutex> guard(get_mutex());

    const size_t total_size = size + occupied_block_metadata_size;

    
    size_t allocator_size = *reinterpret_cast<size_t*>(
            (char*)_trusted_memory + sizeof(logger*) + sizeof(memory_resource*) +
            sizeof(allocator_with_fit_mode::fit_mode));

    char* heap_start = (char*)_trusted_memory + allocator_metadata_size;
    char* heap_end = heap_start + allocator_size;
    void** first_block_ptr = reinterpret_cast<void**>(heap_start - sizeof(void*));
    
    if (*first_block_ptr == nullptr) {
        if (heap_end - heap_start >= total_size) {
            return allocate_new_block(heap_start, size, first_block_ptr, heap_end - heap_start);
        }
        return nullptr;
    }

    
    size_t front_hole = (char*)(*first_block_ptr) - heap_start;
    if (front_hole >= total_size) {
        return allocate_in_hole(heap_start, size, first_block_ptr, nullptr, *first_block_ptr, front_hole);
    }

    
    void* current = *first_block_ptr;
    while (current != nullptr) {
        size_t current_size = *reinterpret_cast<size_t*>(current);
        void* next = *reinterpret_cast<void**>((char*)current + sizeof(size_t));
        char* current_end = (char*)current + occupied_block_metadata_size + current_size;

        if (next == nullptr) {
            
            size_t end_hole = heap_end - current_end;
            if (end_hole >= total_size) {
                return allocate_in_hole(current_end, size, first_block_ptr, current, nullptr, end_hole);
            }
            break;
        }

        
        size_t middle_hole = (char*)next - current_end;
        if (middle_hole >= total_size) {
            return allocate_in_hole(current_end, size, first_block_ptr, current, next, middle_hole);
        }

        current = next;
    }

    return nullptr;
}

void* allocator_boundary_tags::allocate_best_fit(size_t size) {

    std::lock_guard<std::mutex> guard(get_mutex());

    const size_t total_size = size + occupied_block_metadata_size;

    
    size_t allocator_size = *reinterpret_cast<size_t*>(
            (char*)_trusted_memory + sizeof(logger*) + sizeof(memory_resource*) +
            sizeof(allocator_with_fit_mode::fit_mode));

    char* heap_start = (char*)_trusted_memory + allocator_metadata_size;
    char* heap_end = heap_start + allocator_size;
    void** first_block_ptr = reinterpret_cast<void**>(heap_start - sizeof(void*));
    
    void* best_pos = nullptr;
    void* best_prev = nullptr;
    void* best_next = nullptr;
    size_t best_diff = SIZE_MAX;

    
    if (*first_block_ptr != nullptr) {
        size_t front_hole = (char*)(*first_block_ptr) - heap_start;
        if (front_hole >= total_size) {
            best_pos = heap_start;
            best_prev = nullptr;
            best_next = *first_block_ptr;
            best_diff = front_hole - total_size;
        }
    } else {
    
        if (heap_end - heap_start >= total_size) {
            return allocate_new_block(heap_start, size, first_block_ptr, heap_end - heap_start);
        }
        return nullptr;
    }

    
    void* current = *first_block_ptr;
    while (current != nullptr) {
        size_t current_size = *reinterpret_cast<size_t*>(current);
        void* next = *reinterpret_cast<void**>((char*)current + sizeof(size_t));
        char* current_end = (char*)current + occupied_block_metadata_size + current_size;

        size_t hole_size = (next != nullptr)
                           ? (char*)next - current_end
                           : heap_end - current_end;

        if (hole_size >= total_size) {
            size_t diff = hole_size - total_size;
            if (diff < best_diff) {
                best_pos = current_end;
                best_prev = current;
                best_next = next;
                best_diff = diff;

                if (diff == 0) break;
            }
        }

        current = next;
    }

    if (best_pos != nullptr) {
        return allocate_in_hole(reinterpret_cast<char*>(best_pos), size, first_block_ptr, best_prev, best_next, best_diff);
    }

    return nullptr;
}

void* allocator_boundary_tags::allocate_worst_fit(size_t size) {

    std::lock_guard<std::mutex> guard(get_mutex());

    const size_t total_size = size + occupied_block_metadata_size;

    
    size_t allocator_size = *reinterpret_cast<size_t*>(
            (char*)_trusted_memory + sizeof(logger*) + sizeof(memory_resource*) +
            sizeof(allocator_with_fit_mode::fit_mode));
    char* heap_start = (char*)_trusted_memory + allocator_metadata_size;
    char* heap_end = heap_start + allocator_size;
    void** first_block_ptr = reinterpret_cast<void**>(heap_start - sizeof(void*));
    
    
    void* worst_pos = nullptr;
    void* worst_prev = nullptr;
    void* worst_next = nullptr;
    size_t worst_size = 0;
    
    if (*first_block_ptr == nullptr) {
        if (heap_end - heap_start >= total_size) {
            return allocate_new_block(heap_start, size, first_block_ptr, heap_end - heap_start);
        }
        return nullptr;
    }

    
    size_t front_hole = (char*)(*first_block_ptr) - heap_start;
    if (front_hole >= total_size) {
        worst_size = front_hole;
        worst_pos = (void*)heap_start;
        worst_next = *first_block_ptr;
        worst_prev = nullptr;
    }

    
    void* current = *first_block_ptr;
    while (current != nullptr) {
        size_t current_size = *reinterpret_cast<size_t*>(current);
        void* next = *reinterpret_cast<void**>((char*)current + sizeof(size_t));
        char* current_end = (char*)current + occupied_block_metadata_size + current_size;

        size_t hole_size = (next != nullptr)
                           ? (char*)next - current_end
                           : heap_end - current_end;

        if (hole_size >= total_size && hole_size > worst_size) {
            worst_pos = current_end;
            worst_prev = current;
            worst_next = next;
            worst_size = hole_size;
        }

        current = next;
    }

    if (worst_pos != nullptr) {
        return allocate_in_hole(reinterpret_cast<char*>(worst_pos), size, first_block_ptr, worst_prev, worst_next, worst_size);
    }

    return nullptr;
}

void allocator_boundary_tags::do_deallocate_sm(void* at) {
    logger* logger = get_logger();
    logger->debug(get_typename() + "::do_deallocate_sm(void* at): called");
    std::lock_guard<std::mutex> guard(get_mutex());

    if (at == nullptr) return;
    char* heap_start = reinterpret_cast<char*>(_trusted_memory) + allocator_metadata_size;
    size_t heap_size = *reinterpret_cast<size_t*>(
            reinterpret_cast<char*>(_trusted_memory) +
            sizeof(class logger*) + sizeof(memory_resource*) +
            sizeof(allocator_with_fit_mode::fit_mode));
    char* heap_end = heap_start + heap_size;

    
    if (at < (void*)heap_start || at > (void*)heap_end) {
        logger->error("::do_allocate_sm(void* at): pointer does not belong this allocator");
        throw std::invalid_argument("Pointer does not belong to this allocator");
    }

    
    char* block_start = reinterpret_cast<char*>(at);
    size_t block_size = *reinterpret_cast<size_t*>(block_start);
    logger->information(get_typename() + "::do_deallocate_sm(void* at): free " + std::to_string(block_size));
    
    void* next_block = *reinterpret_cast<void**>(reinterpret_cast<char*>(at) + sizeof(size_t));
    void* prev_block = *reinterpret_cast<void**>(reinterpret_cast<char*>(at) + sizeof(size_t) + sizeof(void*));
    if(prev_block){
        *reinterpret_cast<void**>(reinterpret_cast<char*>(prev_block) + sizeof(size_t)) = next_block;
    }
    else{
        void** first_block_ptr = reinterpret_cast<void**>(heap_start - sizeof(void*));
        *first_block_ptr = next_block;
    }
    if(next_block){
        *reinterpret_cast<void**>(reinterpret_cast<char*>(next_block) + sizeof(size_t) + sizeof(void*)) = prev_block;
    }
    logger->debug(get_typename() + "::do_deallocate_sm(void* at): finished");
}

inline void allocator_boundary_tags::set_fit_mode(
        allocator_with_fit_mode::fit_mode mode)
{
    unsigned char* memory = reinterpret_cast<unsigned char*>(_trusted_memory);
    allocator_with_fit_mode::fit_mode* mode_ptr = reinterpret_cast<allocator_with_fit_mode::fit_mode*>(memory+ sizeof(logger*)+ sizeof(memory_resource*));
    *mode_ptr = mode;

}

inline std::mutex &allocator_boundary_tags::get_mutex() const
{
    if (_trusted_memory == nullptr)
    {
        throw std::logic_error("Access to mutex in invalid allocator state");
    }

    auto *ptr = reinterpret_cast<std::byte*>(_trusted_memory);
    ptr += sizeof(logger*);
    ptr += sizeof(memory_resource*);
    ptr += sizeof(allocator_with_fit_mode::fit_mode);
    ptr += sizeof(size_t);

    return *reinterpret_cast<std::mutex*>(ptr);
}


std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info() const
{
    logger* logger = get_logger();
    logger->trace(get_typename() + "::get_blocks_info(): called");

    std::lock_guard<std::mutex> guard(get_mutex());

    auto result = get_blocks_info_inner();

    logger->debug(get_typename() + "::get_blocks_info() : retrieved " + std::to_string(result.size()) + " blocks.");

    logger->trace(get_typename() + "::get_blocks_info(): finished");
    return result;
}

std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info_inner() const
{
    logger* logger = get_logger();
    logger->trace(get_typename() + "::get_blocks_info_inner(): called");
    std::vector<allocator_test_utils::block_info> blocks_info;

    if (_trusted_memory == nullptr)
    {
        logger->error(get_typename() + "::get_blocks_info_inner(): not init memory");
        return blocks_info;
    }

    try
    {
        
        char* heap_start = reinterpret_cast<char*>(_trusted_memory) + allocator_metadata_size;
        size_t heap_size = *reinterpret_cast<size_t*>(
                reinterpret_cast<char*>(_trusted_memory) +
                sizeof(class logger*) + sizeof(memory_resource*) +
                sizeof(allocator_with_fit_mode::fit_mode));
        char* heap_end = heap_start + heap_size;

        
        void** first_block_ptr = reinterpret_cast<void**>(heap_start - sizeof(void*));
        char* current_block = reinterpret_cast<char*>(*first_block_ptr);

        while (current_block != nullptr && current_block < heap_end)
        {
            size_t block_size = *reinterpret_cast<size_t*>(current_block);
            void* next_block = *reinterpret_cast<void**>(current_block + sizeof(size_t));

            size_t data_size = (block_size) ?  block_size - occupied_block_metadata_size: occupied_block_metadata_size;
            bool is_occupied = true;

            blocks_info.push_back({
                                          .block_size = block_size + occupied_block_metadata_size,
                                          .is_block_occupied = is_occupied
                                  });

            current_block = reinterpret_cast<char*>(next_block);
        }

        
        current_block = reinterpret_cast<char*>(*first_block_ptr);
        char* prev_block_end = heap_start;
        size_t hole_counter = 1;

        while (current_block != nullptr && current_block < heap_end)
        {
        
            if (current_block > prev_block_end)
            {
                size_t hole_size = current_block - prev_block_end;

                blocks_info.push_back({
                                              .block_size = hole_size,
                                              .is_block_occupied = false
                                      });
            }

        
            size_t block_size = *reinterpret_cast<size_t*>(current_block);
            prev_block_end = current_block + occupied_block_metadata_size + block_size;

        
            void* next_block = *reinterpret_cast<void**>(current_block + sizeof(size_t));
            current_block = reinterpret_cast<char*>(next_block);
        }

        
        if (prev_block_end < heap_end)
        {
            size_t hole_size = heap_end - prev_block_end;
            blocks_info.push_back({
                                          .block_size = hole_size,
                                          .is_block_occupied = false
                                  });
        }

    }
    catch (...)
    {

        logger->error(get_typename() + "::get_blocks_info_inner() : iteration failed.");
        throw;
    }

    return blocks_info;
}

inline logger *allocator_boundary_tags::get_logger() const
{
    if (_trusted_memory == nullptr)
    {
        return nullptr;
    }
    return *reinterpret_cast<logger**>(_trusted_memory);
}

inline std::string allocator_boundary_tags::get_typename() const noexcept
{
    return "allocator_boundary_tags";
}


allocator_boundary_tags::boundary_iterator allocator_boundary_tags::begin() const noexcept {
    return boundary_iterator(reinterpret_cast<char*>(_trusted_memory) + allocator_metadata_size);
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::end() const noexcept {
    size_t total_size = *reinterpret_cast<size_t*>(
            reinterpret_cast<char*>(_trusted_memory) +
            sizeof(logger*) + sizeof(memory_resource*) +
            sizeof(allocator_with_fit_mode::fit_mode));
    return boundary_iterator(reinterpret_cast<char*>(_trusted_memory) + total_size);
}

bool allocator_boundary_tags::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    if (this == &other) return true;
    const auto* derived = dynamic_cast<const allocator_boundary_tags*>(&other);
    if (derived == nullptr) return false;

    return this->_trusted_memory == derived->_trusted_memory;

}

bool allocator_boundary_tags::boundary_iterator::operator==(const boundary_iterator &other) const noexcept {
    return _occupied_ptr == other._occupied_ptr;
}

bool allocator_boundary_tags::boundary_iterator::operator!=(const boundary_iterator &other) const noexcept {
    return !(*this == other);
}

allocator_boundary_tags::boundary_iterator &allocator_boundary_tags::boundary_iterator::operator++() & noexcept {
    size_t block_size = *reinterpret_cast<size_t*>(_occupied_ptr) & ~size_t(1);
    _occupied_ptr = reinterpret_cast<void*>(
            reinterpret_cast<std::byte*>(_occupied_ptr) + block_size
    );

    size_t tag = *reinterpret_cast<size_t*>(_occupied_ptr);
    _occupied = tag & 1;

    return *this;
}

allocator_boundary_tags::boundary_iterator &allocator_boundary_tags::boundary_iterator::operator--() & noexcept {
    void* footer_ptr = reinterpret_cast<std::byte*>(_occupied_ptr) - sizeof(size_t);
    size_t block_size = *reinterpret_cast<size_t*>(footer_ptr) & ~size_t(1);

    _occupied_ptr = reinterpret_cast<std::byte*>(_occupied_ptr) - block_size;
    size_t tag = *reinterpret_cast<size_t*>(_occupied_ptr);
    _occupied = tag & 1;

    return *this;
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::boundary_iterator::operator++(int) {
    boundary_iterator tmp = *this;
    ++(*this);
    return tmp;
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::boundary_iterator::operator--(int) {
    boundary_iterator tmp = *this;
    --(*this);
    return tmp;
}

size_t allocator_boundary_tags::boundary_iterator::size() const noexcept {
    return *reinterpret_cast<size_t*>(_occupied_ptr) & ~size_t(1);
}

bool allocator_boundary_tags::boundary_iterator::occupied() const noexcept {
    return _occupied;
}

void* allocator_boundary_tags::boundary_iterator::operator*() const noexcept {
    return reinterpret_cast<void*>(
            reinterpret_cast<std::byte*>(_occupied_ptr) + sizeof(size_t)
    );
}

void *allocator_boundary_tags::boundary_iterator::get_ptr() const noexcept {
    return _occupied_ptr;
}

allocator_boundary_tags::boundary_iterator::boundary_iterator()
        : _occupied_ptr(nullptr), _occupied(false), _trusted_memory(nullptr) {}

allocator_boundary_tags::boundary_iterator::boundary_iterator(void *trusted)
        : _trusted_memory(trusted)
{
    _occupied_ptr = trusted;
    size_t tag = *reinterpret_cast<size_t*>(_occupied_ptr);
    _occupied = tag & 1;
}