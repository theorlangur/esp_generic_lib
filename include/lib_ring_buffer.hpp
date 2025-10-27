#ifndef LIB_RING_BUFFER_HPP_
#define LIB_RING_BUFFER_HPP_

#include <cstddef>
#include <optional>
#include "lib_misc_helpers.hpp"
#include "lib_type_traits.hpp"

template<class T> requires std::is_integral_v<T>
constexpr int HighestBitSet(T N)
{
    int r = -1;
    while(N) 
    {
        N >>= 1;
        ++r;
    }
    return r;
}

template<class T, size_t N>
struct RingBuffer
{
    using size_type = MinSizeType<N>::type;
    constexpr static int kHighestBit = HighestBitSet(size_type(N));
    constexpr static size_type kBufSize = kHighestBit >= 0 ? size_type(1) << (kHighestBit + 1) : 0;
    constexpr static size_type kMask = kBufSize - 1;

    template<class... Args> requires (N > 0)
    std::optional<size_type> push(Args&&...args)
    {
        auto newTail = (m_Tail + 1) & kMask;
        if (newTail == m_Head) return std::nullopt;
        m_Tail = newTail;
        new (&(m_Buf[m_Tail].item)) T{std::forward<Args>(args)...};
        return size_type(((kBufSize + m_Tail) - m_Head) & kMask);
    }

    std::optional<T> pop() requires (N > 0)
    {
        if (m_Tail == m_Head) return std::nullopt;
        ScopeExit cleanup = [&]{
            if constexpr (!std::is_trivially_destructible_v<T>)
                m_Buf[m_Head].item.~T();
        };
        m_Head = (m_Head + 1) & kMask;
        return m_Buf[m_Head].item;
    }

    bool drop() requires (N > 0)
    {
        if (m_Tail == m_Head) return false;

        m_Head = (m_Head + 1) & kMask;
        if constexpr (!std::is_trivially_destructible_v<T>)
            m_Buf[m_Head].item.~T();
        return true;
    }

    T* peek() requires (N > 0)
    {
        if (m_Tail == m_Head) return nullptr;
        return &m_Buf[(m_Head + 1) & kMask].item;
    }

    size_t size() const requires (N > 0)
    {
        if (m_Tail == m_Head) return 0;
        return (m_Tail + kBufSize - m_Head) & kMask;
    }

    struct iterator_t
    {
        RingBuffer &r;
        size_type i;
        void operator++()
        {
            i = (i + 1) & kMask;
        }
        bool operator!=(const iterator_t &it) const { return i != it.i; }
        T* operator*()
        {
            return &(r.m_Buf[(i + 1) & kMask].item);
        }
    };

    auto begin() { return iterator_t{*this, m_Head}; }
    auto end() { return iterator_t{*this, m_Tail}; }
private:
    union Raw
    {
        T item;
    };
    [[no_unique_address]]Raw m_Buf[kBufSize];
    size_type m_Head = 0;
    size_type m_Tail = 0;
};

#endif
