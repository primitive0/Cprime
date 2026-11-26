#ifndef CPRIME_AST_LIST_H_
#define CPRIME_AST_LIST_H_

#include <iterator>
#include <type_traits>
#include <cprime/support/prelude.hpp>
#include <cprime/support/contract.hpp>

namespace cprime::ast {

class ListHook
{
    template<typename T>
    friend class List;

    template<typename T>
    friend class ListIterator;

public:
    ListHook(const ListHook&) = delete;
    auto operator=(const ListHook&) -> ListHook& = delete;

    ListHook(ListHook&&) = delete;
    auto operator=(ListHook&&) -> ListHook& = delete;

protected:
    explicit ListHook()
        : prev_{nullptr}
        , next_{nullptr}
    {
    }

private:
    ListHook* prev_;
    ListHook* next_;
};

template<typename T>
class List;

template<typename T>
class ListIterator final
{
    friend class List<T>;

public:
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = std::remove_const_t<T>;
    using pointer = T*;
    using reference = T&;

public:
    explicit(false) ListIterator()
        : current_{nullptr}
    {
    }

    auto operator*() const -> T&
    {
        CPRIME_DEBUG_ASSERT(current_ != nullptr);
        return static_cast<T&>(*current_);
    }

    auto operator->() const -> T*
    {
        CPRIME_DEBUG_ASSERT(current_ != nullptr);
        return static_cast<T*>(current_);
    }

    auto operator++() -> ListIterator&
    {
        CPRIME_DEBUG_ASSERT(current_ != nullptr);
        current_ = current_->next_;
        return *this;
    }

    auto operator++(int) -> ListIterator
    {
        auto old = *this;
        ++(*this);
        return old;
    }

    // TODO: add sentinel
    // auto operator--() -> ListIterator&
    // {
    //     CPRIME_DEBUG_ASSERT(current_ != nullptr);
    //     current_ = current_->prev_;
    //     return *this;
    // }
    //
    // auto operator--(int) -> ListIterator
    // {
    //     auto old = *this;
    //     --(*this);
    //     return old;
    // }

    auto operator==(const ListIterator& rhs) const -> bool
    {
        return current_ == rhs.current_;
    }

private:
    explicit ListIterator(ListHook* current)
        : current_{current}
    {
    }

private:
    ListHook* current_;
};

template<typename T>
class List final
{
public:
    List(const List&) = delete;
    auto operator=(const List&) -> List& = delete;

    List(List&& other) noexcept
        : front_{std::exchange(other.front_, nullptr)}
        , back_{std::exchange(other.back_, nullptr)}
        , size_{std::exchange(other.size_, 0)}
    {
    }

    auto operator=(List&&) -> List& = delete;

    explicit List()
        : front_{nullptr}
        , back_{nullptr}
        , size_{0}
    {
    }

    auto front() -> T&
    {
        CPRIME_DEBUG_ASSERT(size_ != 0);
        CPRIME_DEBUG_ASSERT(front_ != nullptr && back_ != nullptr);
        return *front_;
    }

    auto back() -> T&
    {
        CPRIME_DEBUG_ASSERT(size_ != 0);
        CPRIME_DEBUG_ASSERT(front_ != nullptr && back_ != nullptr);
        return *back_;
    }

    auto size() -> size_t
    {
        return size_;
    }

    auto push_front(T& value) -> void
    {
        CPRIME_DEBUG_ASSERT(!is_node_linked(value));

        if (size_ == 0) {
            front_ = &value;
            back_ = &value;
        } else {
            CPRIME_DEBUG_ASSERT(front_ != nullptr && back_ != nullptr);
            as_hook(value).next_ = front_;
            as_hook(front_).prev_ = &value;
            front_ = &value;
        }
        ++size_;
    }

    auto push_back(T& value) -> void
    {
        CPRIME_DEBUG_ASSERT(!is_node_linked(value));

        if (size_ == 0) {
            front_ = &value;
            back_ = &value;
        } else {
            CPRIME_DEBUG_ASSERT(front_ != nullptr && back_ != nullptr);
            as_hook(back_).next_ = &value;
            as_hook(value).prev_ = back_;
            back_ = &value;
        }
        ++size_;
    }

    auto begin() -> ListIterator<T>
    {
        return ListIterator<T>{front_};
    }

    auto end() -> ListIterator<T>
    {
        return ListIterator<T>{nullptr};
    }

private:
    static auto is_node_linked(T& value) -> bool
    {
        // TODO: fix implementation with sentinel node
        ListHook& hook = as_hook(value);
        return hook.prev_ != nullptr || hook.next_ != nullptr;
    }

    static auto as_hook(T* value) -> ListHook&
    {
        CPRIME_DEBUG_ASSERT(value != nullptr);
        return static_cast<ListHook&>(*value);
    }

    static auto as_hook(T& value) -> ListHook&
    {
        return static_cast<ListHook&>(value);
    }

private:
    T* front_;
    T* back_;
    size_t size_;
};

} // namespace cprime::ast

#endif // CPRIME_AST_LIST_H_
