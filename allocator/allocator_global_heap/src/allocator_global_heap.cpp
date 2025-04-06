#include "../include/allocator_global_heap.h"
#include <sstream>

allocator_global_heap::allocator_global_heap(logger *logger)
    : _logger(logger)
{
    trace_with_guard("allocator_global_heap constructor started");
    trace_with_guard("allocator_global_heap constructor finished");
}

allocator_global_heap::~allocator_global_heap()
{
    trace_with_guard("allocator_global_heap destructor started");
    trace_with_guard("allocator_global_heap destructor finished");
}

allocator_global_heap::allocator_global_heap(const allocator_global_heap &other)
    : _logger(other._logger)
{
    trace_with_guard("allocator_global_heap copy constructor started");
    trace_with_guard("allocator_global_heap copy constructor finished");
}

allocator_global_heap &allocator_global_heap::operator=(const allocator_global_heap &other)
{
    if (this != &other)
    {
        trace_with_guard("allocator_global_heap copy assignment started");
        _logger = other._logger;
        trace_with_guard("allocator_global_heap copy assignment finished");
    }
    return *this;
}

allocator_global_heap::allocator_global_heap(allocator_global_heap &&other) noexcept
    : _logger(other._logger)
{
    trace_with_guard("allocator_global_heap move constructor started");
    trace_with_guard("allocator_global_heap move constructor finished");
}

allocator_global_heap &allocator_global_heap::operator=(allocator_global_heap &&other) noexcept
{
    if (this != &other)
    {
        trace_with_guard("allocator_global_heap move assignment started");
        _logger = other._logger;
        trace_with_guard("allocator_global_heap move assignment finished");
    }
    return *this;
}

void* allocator_global_heap::do_allocate_sm(const size_t size)
{
    debug_with_guard("Starting allocation of size " + std::to_string(size));
    try
    {
        void* ptr = ::operator new(size);
        std::ostringstream oss;
        oss << "0x" << std::hex << reinterpret_cast<std::uintptr_t>(ptr);
        debug_with_guard("Successfully allocated memory at " + oss.str() + " of size " + std::to_string(size));
        return ptr;
    }
    catch (const std::bad_alloc& e)
    {
        error_with_guard("Failed to allocate memory of size " + std::to_string(size) + ": " + std::string(e.what()));
        throw;
    }
    catch (const std::exception& e)
    {
        error_with_guard("Unexpected exception during memory allocation: " + std::string(e.what()));
        throw;
    }
}

void allocator_global_heap::do_deallocate_sm(void* at)
{
    if (at == nullptr)
    {
        debug_with_guard("Attempted to deallocate NULL pointer - ignoring");
        return;
    }
    
    std::ostringstream oss;
    oss << "0x" << std::hex << reinterpret_cast<std::uintptr_t>(at);
    debug_with_guard("Starting deallocation of memory at " + oss.str());
    ::operator delete(at);
    debug_with_guard("Successfully deallocated memory at " + oss.str());
}

bool allocator_global_heap::do_is_equal(const std::pmr::memory_resource& other) const noexcept
{
    return dynamic_cast<const allocator_global_heap*>(&other) != nullptr;
}

logger* allocator_global_heap::get_logger() const
{
    return _logger;
}

std::string allocator_global_heap::get_typename() const
{
    return "allocator_global_heap";
}