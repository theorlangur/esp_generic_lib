#ifndef LIB_RING_BUFFER_HPP_
#define LIB_RING_BUFFER_HPP_

#include <cstddef>
#include <optional>
#include "lib_misc_helpers.hpp"
#include "lib_type_traits.hpp"

template<class T, size_t N>
struct RingBuffer
{
    using size_type = MinSizeType<N>::type;

    template<class... Args>
    std::optional<size_type> push(Args&&...args)
    {
        if (m_Full) return std::nullopt;
        new (&(m_Buf[m_Tail++].item)) T{std::forward<Args>(args)...};
        m_Tail %= N;
        m_Full = m_Tail == m_Head;
        return size_type(((N + m_Tail) - m_Head) % N);
    }

    std::optional<T> pop()
    {
        if (m_Tail == m_Head && !m_Full) return std::nullopt;
        ScopeExit cleanup = [&]{
            if constexpr (!std::is_trivially_destructible_v<T>)
                m_Buf[m_Head].item.~T();
            ++m_Head;
            m_Full = false;
        };
        return m_Buf[m_Head].item;
    }

    bool drop()
    {
        if (m_Tail == m_Head && !m_Full) return false;

        if constexpr (!std::is_trivially_destructible_v<T>)
            m_Buf[m_Head].item.~T();
        ++m_Head;
        m_Full = false;
        return true;
    }

    T* peek()
    {
        if (m_Tail == m_Head && !m_Full) return nullptr;
        return &m_Buf[m_Head].item;
    }
private:
    union Raw
    {
        T item;
    };
    Raw m_Buf[N];
    size_type m_Head = N;
    size_type m_Tail = N;
    bool m_Full = false;
};

#endif
