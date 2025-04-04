#ifndef LIB_MISC_HELPERS_HPP_
#define LIB_MISC_HELPERS_HPP_

#if __has_include("esp_err.h")
#include "esp_err.h"
#endif
#include <chrono>
#include "lib_formatter.hpp"

using duration_ms_t = std::chrono::duration<int, std::milli>;
inline static constexpr const duration_ms_t kForever = duration_ms_t(-1);

template<class CB>
struct ScopeExit
{
    ScopeExit(CB &&cb):m_CB(std::move(cb)){}
    ~ScopeExit(){ m_CB(); }
private:
    CB m_CB;
};

template<typename BaseType, class Tag>
struct StrongType
{
    StrongType(BaseType d): m_Data(d) {}
    BaseType data() const { return m_Data; }
private:
    BaseType m_Data;

    friend struct Comparable;
    friend struct Oderable;
};

struct NonCopyable
{
    NonCopyable() = default;
    NonCopyable(NonCopyable const& rhs) = delete;
    NonCopyable& operator=(NonCopyable const& rhs) const = delete;
};

struct NonMovable
{
    NonMovable() = default;
    NonMovable(NonMovable && rhs) = delete;
    NonMovable& operator=(NonMovable && rhs) const = delete;
};

template<typename BaseType, BaseType kInv>
struct WithInvalidState
{
    static constexpr BaseType kInvalidState = kInv;
};

#if defined(NDEBUG) && !defined(FORCE_FMT)
#define FMT_PRINT(fmt,...) {}
#define FMT_PRINTLN(fmt,...) {}
#else
#ifndef PRINTF_FUNC
#define PRINTF_FUNC(...) printf(__VA_ARGS__)
#endif
#define FMT_PRINT(fmt,...) { char buf[256]; tools::format_to(tools::BufferFormatter(buf), fmt __VA_OPT__(,) __VA_ARGS__); PRINTF_FUNC("%s", buf); }
#define FMT_PRINTLN(fmt,...) { char buf[256]; tools::format_to(tools::BufferFormatter(buf), fmt "\n" __VA_OPT__(,) __VA_ARGS__); PRINTF_FUNC("%s", buf); }
#endif

#endif
