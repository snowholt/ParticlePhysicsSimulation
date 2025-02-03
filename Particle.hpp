#pragma once
#include <immintrin.h>
#include <memory>
#include <cstddef>

struct alignas(32) Particle {
    __m256 position;  // (x,y,z,_)
    __m256 velocity;  // (vx,vy,vz,_)
    float mass;
    float charge;
    
    void updatePosition(float dt) {
        position = _mm256_add_ps(position,
            _mm256_mul_ps(velocity, _mm256_set1_ps(dt)));
    }
};

template<typename T>
struct AlignedAllocator {
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    template<typename U>
    struct rebind {
        using other = AlignedAllocator<U>;
    };

    AlignedAllocator() noexcept = default;
    template<typename U>
    AlignedAllocator(const AlignedAllocator<U>&) noexcept {}

    pointer allocate(size_type n) {
        if (n > std::size_t(-1) / sizeof(T)) throw std::bad_alloc();
        if (auto ptr = static_cast<pointer>(_mm_malloc(n * sizeof(T), 32))) 
            return ptr;
        throw std::bad_alloc();
    }

    void deallocate(pointer p, size_type) noexcept {
        _mm_free(p);
    }
};

template<typename T, typename U>
bool operator==(const AlignedAllocator<T>&, const AlignedAllocator<U>&) { return true; }

template<typename T, typename U>
bool operator!=(const AlignedAllocator<T>&, const AlignedAllocator<U>&) { return false; }
