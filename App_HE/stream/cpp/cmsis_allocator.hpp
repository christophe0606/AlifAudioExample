#pragma once 

#include <cstdio>
#include <type_traits>

extern osMemoryPoolId_t cg_eventPool;
extern osMemoryPoolId_t cg_bufPool;
extern osMemoryPoolId_t cg_mutexPool;

template <typename T>
class CMSISEventPoolAllocator
{
public:
    using value_type = T;
    using is_always_equal = std::true_type;

    CMSISEventPoolAllocator() noexcept = default;

    template <typename U>
    CMSISEventPoolAllocator(const CMSISEventPoolAllocator<U> &) noexcept {}

    template <typename U>
    struct rebind
    {
        using other = CMSISEventPoolAllocator<U>;
    };

    T *allocate(std::size_t n)
    {
        if ((n == 1) && (sizeof(T) <= osMemoryPoolGetBlockSize(cg_eventPool)))
        {
            //printf("Alloc event %d\n",sizeof(T));
            return static_cast<T *>(osMemoryPoolAlloc(cg_eventPool,0));
        }
        else
        {
           //throw std::bad_alloc(); // required by standard
           return nullptr;
        }
    };

    void deallocate(T *p, std::size_t n) noexcept
    {
        osMemoryPoolFree(cg_eventPool,static_cast<void*>(p));
    };


    // Optional but recommended: construct and destroy can be omitted
    // if you're using std::allocator_traits (it will use placement new)
};

template <typename T, typename U>
bool operator==(const CMSISEventPoolAllocator<T> &, const CMSISEventPoolAllocator<U> &) { return true; }

template <typename T, typename U>
bool operator!=(const CMSISEventPoolAllocator<T> &, const CMSISEventPoolAllocator<U> &) { return false; }

template <typename T>
class CMSISBufPoolAllocator
{
public:
    using value_type = T;
    using is_always_equal = std::true_type;

    CMSISBufPoolAllocator() noexcept = default;

    template <typename U>
    CMSISBufPoolAllocator(const CMSISBufPoolAllocator<U> &) noexcept {}

    template <typename U>
    struct rebind
    {
        using other = CMSISBufPoolAllocator<U>;
    };

    T *allocate(std::size_t n)
    {
        if ((n == 1) && (sizeof(T) <= osMemoryPoolGetBlockSize(cg_bufPool)))
        {
            //printf("Alloc buf %d\n",sizeof(T));
            return static_cast<T *>(osMemoryPoolAlloc(cg_bufPool,0));
        }
        else
        {
            //throw std::bad_alloc();
            return nullptr;
        }
    };

    void deallocate(T *p, std::size_t n) noexcept
    {
        osMemoryPoolFree(cg_bufPool,static_cast<void*>(p));
    };


    // Optional but recommended: construct and destroy can be omitted
    // if you're using std::allocator_traits (it will use placement new)
};

template <typename T, typename U>
bool operator==(const CMSISBufPoolAllocator<T> &, const CMSISBufPoolAllocator<U> &) { return true; }

template <typename T, typename U>
bool operator!=(const CMSISBufPoolAllocator<T> &, const CMSISBufPoolAllocator<U> &) { return false; }

template <typename T>
class CMSISMutexPoolAllocator
{
public:
    using value_type = T;
    using is_always_equal = std::true_type;

    CMSISMutexPoolAllocator() noexcept = default;

    template <typename U>
    CMSISMutexPoolAllocator(const CMSISMutexPoolAllocator<U> &) noexcept {}

    template <typename U>
    struct rebind
    {
        using other = CMSISMutexPoolAllocator<U>;
    };

    T *allocate(std::size_t n)
    {
        if ((n == 1) && (sizeof(T) <= osMemoryPoolGetBlockSize(cg_mutexPool)))
        {
            //printf("Alloc mutex %d\n",sizeof(T));
            return static_cast<T *>(osMemoryPoolAlloc(cg_mutexPool,0));
        }
        else
        {
            //throw std::bad_alloc();
            return nullptr;
        }
    };

    void deallocate(T *p, std::size_t n) noexcept
    {
        osMemoryPoolFree(cg_bufPool,static_cast<void*>(p));
    };


    // Optional but recommended: construct and destroy can be omitted
    // if you're using std::allocator_traits (it will use placement new)
};

template <typename T, typename U>
bool operator==(const CMSISMutexPoolAllocator<T> &, const CMSISMutexPoolAllocator<U> &) { return true; }

template <typename T, typename U>
bool operator!=(const CMSISMutexPoolAllocator<T> &, const CMSISMutexPoolAllocator<U> &) { return false; }
