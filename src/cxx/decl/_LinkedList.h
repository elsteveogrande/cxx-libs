#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include <cassert>
#include <concepts>
#include <cstddef>
#include <memory>

namespace cxx {

template <std::regular T>
struct LinkedList final {
    template <typename U = T>
    struct Node {
        std::shared_ptr<T> data_ {};
        std::shared_ptr<Node<U>> next_ {nullptr};
        size_t const size_ {0};
    };

    template <typename U = T>
    struct Iterator {
        std::shared_ptr<Node<U>> node_;

        ~Iterator() noexcept = default;
        Iterator() noexcept = default;
        Iterator(std::shared_ptr<Node<U>> const& node) noexcept : node_(node) {}
        Iterator(Iterator const&) noexcept = default;
        Iterator& operator=(Iterator const&) noexcept = default;

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

    std::shared_ptr<Node<T>> front_ {nullptr};
    std::shared_ptr<Node<T>> back_ {nullptr};

    using value_type = T;
    using iterator = Iterator<T>;
    using const_iterator = Iterator<T> const;

    ~LinkedList() noexcept = default;
    LinkedList() noexcept = default;
    LinkedList(LinkedList const&) noexcept = default;
    LinkedList& operator=(LinkedList const&) noexcept = default;

    iterator begin() const { return {front_}; }
    iterator end(this auto) { return {}; }
    const_iterator cbegin() const { return {front_}; }
    const_iterator cend(this auto) { return {}; }
    size_t size() const { return front_->size_; }

    bool operator==(LinkedList<T> const& rhs) const {
        if (size() != rhs.size()) { return false; }
        auto a = begin();
        auto b = rhs.begin();
        for (; a.node_; ++a, ++b) {
            if (*a != *b) { return false; }
        }
        return true;
    }

    // TODO: consider a threadsafe variant of LinkedList (or make it threadsafe)
    // TODO: after allowing CAS Ref reassignments

    void pushFront(std::shared_ptr<T> item) {
        auto newFront = std::make_shared<Node<T>>(item, front_);
        front_ = newFront;
        if (!back_) { back_ = front_; }
    }

    void pushBack(std::shared_ptr<T> item) {
        auto newBack = std::make_shared<Node<T>>(item, nullptr);
        if (back_) { back_->next_ = newBack; }
        back_ = newBack;
        if (!front_) { front_ = back_; }
    }
};
}  // namespace cxx
