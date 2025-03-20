#ifndef LIB_NOTIFICATION_NODE_HPP_
#define LIB_NOTIFICATION_NODE_HPP_

#include "lib_linked_list.hpp"
#include <utility>

template<class Self>
struct GenericNotificationNode: Node
{
    template<class...T>
    auto Notify(T&&... args) { return static_cast<Self&>(*this)->DoNotify(std::forward<T>(args)...); }

    void RegisterSelf() { Register(static_cast<Self&>(*this)); }

    static void Register(Self &n)
    {
        g_List += n;
    }
    static LinkedListT<Self> g_List;
};
template<class Self>
LinkedListT<Self> GenericNotificationNode<Self>::g_List;

#endif
