#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include "../ref/Ref.h"

#include <cassert>
#include <cstddef>

namespace cxx {

/**
 * A simple forward-linked-list.
 * Contains an object of type `T` via a reference `Ref<T>`.
 *
 * TODO: make this a doubly-linked list?  Might cause problems unless we get a `Weak` ref type
 * TODO: make thread-safe, lock-free, (wait-free?)
 */
template <typename T>
struct LinkedList final {
    template <typename U = T>
    struct Node {
        Ref<T> data_ {};
        Ref<Node<U>> next_ {nullptr};
        size_t const size_ {0};
    };

    template <typename U = T>
    struct Iterator {
        Ref<Node<U>> node_;
        Iterator(Ref<Node<U>> const& node) noexcept : node_(node) {}

        U const& operator*() const { return *node_->data_; }
        Iterator<U>& operator++() {
            node_ = node_->next_;
            return *this;
        }
        Iterator<U>& operator++(int) {
            node_ = node_->next_;
            return *this;
        }
        bool operator==(Iterator<U> const& rhs) const {
            if (!node_) { return !rhs.node_; }
            if (!rhs.node_) { return false; }
            return false;
        }
    };

    Ref<Node<T>> front_ {nullptr};
    Ref<Node<T>> back_ {nullptr};

    using value_type = T;
    using iterator = Iterator<T>;
    using const_iterator = Iterator<T> const;

    ~LinkedList() noexcept = default;
    LinkedList() noexcept = default;
    LinkedList(LinkedList const&) noexcept = default;
    LinkedList& operator=(LinkedList const&) noexcept = default;

    iterator begin() const { return {front_}; }
    iterator end(this auto) { return {nullptr}; }
    const_iterator cbegin() const { return {front_}; }
    const_iterator cend(this auto) { return {}; }
    size_t size() const { return front_ ? front_->size_ : 0; }

    bool operator==(LinkedList<T> const& rhs) const {
        if (size() != rhs.size()) { return false; }  // quick check: verify same size
        auto a = begin();                            // starting points for
        auto b = rhs.begin();                        // both lists
        for (; a.node_; ++a, ++b) {                  // until end of both lists
            if (*a != *b) { return false; }          // if anything not equal, return
        }
        return true;
    }

    // TODO: consider a threadsafe variant of LinkedList (or make it threadsafe)
    // TODO: after allowing CAS Ref reassignments

    void pushFront(Ref<T> item) {
        auto newFront = Ref<Node<T>>::make(item, front_);
        front_ = newFront;
        if (!back_) { back_ = front_; }
    }

    void pushBack(Ref<T> item) {
        auto newBack = Ref<Node<T>>::make(item, nullptr);
        if (back_) { back_->next_ = newBack; }
        back_ = newBack;
        if (!front_) { front_ = back_; }
    }
};
}  // namespace cxx
