#ifndef LIB_OBJECT_POOL_HPP_
#define LIB_OBJECT_POOL_HPP_

#include <assert.h>
#include "lib_type_traits.hpp"


template<size_t N>
class MinBitSet
{
public:
    using size_type = MinBitSizeType<N>::type;
    static constexpr size_t bits_per_element = sizeof(size_type) * 8;
    static constexpr size_t data_count = (N + bits_per_element - 1) / bits_per_element;

    constexpr MinBitSet() = default;

    bool test(size_t bit) const
    {
        return (m_Data[bit / bits_per_element] & (1 << (bit % bits_per_element))) != 0;
    }

    void set(size_t bit)
    {
        m_Data[bit / bits_per_element] |= 1 << (bit % bits_per_element);
    }

    void reset(size_t bit)
    {
        m_Data[bit / bits_per_element] &= ~(1 << (bit % bits_per_element));
    }

private:
    size_type m_Data[data_count] = {};
};

template<class T, size_t N>
class ObjectPool
{
public:
    using size_type = MinSizeType<N>::type;
    static constexpr size_type kInvalid = N;

    template<ObjectPool<T,N> &staticPool>
    class Ptr
    {
    public:
        using can_relocate = void;

        Ptr() = default;
        Ptr(T *pPtr): m_pPtr(pPtr) {}
        template<class... Args>// requires (!(std::is_same_v<std::remove_cvref_t<Args>, T*>||...))
        Ptr(Args&&... args):m_pPtr(staticPool.Acquire(std::forward<Args>(args)...)) { }
        ~Ptr() { staticPool.Release(m_pPtr); }

        Ptr(const Ptr&) = delete;//no copy
        Ptr& operator=(const Ptr &rhs) = delete;

        //move ok
        Ptr(Ptr &&rhs): m_pPtr(rhs.m_pPtr) { rhs.m_pPtr = nullptr; }
        Ptr& operator=(Ptr &&rhs)
        { 
            m_pPtr = rhs.m_pPtr;
            rhs.m_pPtr = nullptr; 
            return *this;
        }

        void reset(T *pNew = nullptr) { staticPool.Release(m_pPtr); m_pPtr = pNew; }
        T* release() { auto *pRes = m_pPtr; m_pPtr = nullptr; return pRes; }

        auto operator->() const { return m_pPtr; }
        operator T*() const { return m_pPtr; }
        T& operator *() const { return *m_pPtr; }
    private:
        T *m_pPtr = nullptr;
    };

    constexpr ObjectPool()
    {
        for(size_t i = 0; i < N; ++i) m_Data[i].m_NextFree = i + 1;
    }

    size_type PtrToIdx(T *pPtr) const
    {
        if (!pPtr) return kInvalid;
        assert(((Elem*)pPtr >= m_Data) && ((Elem*)pPtr < (m_Data + N)));
        size_t res = (Elem*)pPtr - m_Data;
        assert(m_Allocated.test(res));
        return (size_type)res;
    }

    T *IdxToPtr(size_t idx)
    {
        assert((idx < N) && m_Allocated.test(idx));
        return &m_Data[idx].m_Object;
    }

    bool IsValid(T *pPtr) const
    {
        if (pPtr)
        {
            assert(((Elem*)pPtr >= m_Data) && ((Elem*)pPtr < (m_Data + N)));
            size_t i = (Elem*)pPtr - m_Data;
            return m_Allocated.test(i);
        }
        return false;
    }

    template<class... Args>
    T* Acquire(Args&&... args)
    {
        if (m_FirstFree >= N) return nullptr;
        auto i = m_FirstFree;
        m_FirstFree = m_Data[m_FirstFree].m_NextFree;
        m_Allocated.set(i);
        return new (&m_Data[i].m_Object) T{std::forward<Args>(args)...};
    }

    void Release(T* pPtr)
    {
        if (pPtr)
        {
            assert(((Elem*)pPtr >= m_Data) && ((Elem*)pPtr < (m_Data + N)));
            if constexpr (!simple_destructible_t<T>)
                pPtr->~T();
            size_t i = (Elem*)pPtr - m_Data;
            m_Data[i].m_NextFree = m_FirstFree;
            m_FirstFree = i;
            m_Allocated.reset(m_FirstFree);
        }
    }
private:
    MinBitSet<N> m_Allocated;
    size_type m_FirstFree = 0;
    union Elem
    {
        constexpr Elem():m_NextFree(0){}
        ~Elem(){}
        size_type m_NextFree;
        T m_Object;
    };
    Elem m_Data[N];
};

#endif
