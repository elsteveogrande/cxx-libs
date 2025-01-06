#include "cxx/Generator.h"
#include "cxx/Ref.h"

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

unsigned ctorCount {0};
unsigned dtorCount {0};

namespace {

struct Foo final {
    int a {42};
    double b {59.25};
    std::string c {"hello"};

    ~Foo() { ++dtorCount; }
    Foo() { ++ctorCount; }
    Foo(int a, double b, std::string c) : a(a), b(b), c(std::move(c)) { ++ctorCount; }
};

struct Repr {
    virtual ~Repr() noexcept = default;
    virtual bool operator==(Repr const&) const { return false; }
};

struct NumRepr : Repr {
    double val_;
    NumRepr(double val) : val_(val) {}
    bool operator==(NumRepr const& rhs) const { return val_ == rhs.val_; }
};

template <typename T>
struct LinkedList final {
    template <typename U = T>
    struct Node {
        cxx::Ref<T> data_ {};
        cxx::Ref<Node<U>> next_ {nullptr};
        size_t const size_ {0};
    };

    template <typename U = T>
    struct Iterator {
        cxx::Ref<Node<U>> node_;

        ~Iterator() noexcept = default;
        Iterator() noexcept = default;
        Iterator(cxx::Ref<Node<U>> const& node) noexcept : node_(node) {}
        Iterator(Iterator const&) noexcept = default;
        Iterator& operator=(Iterator const&) noexcept = default;

        U& operator*() { return *node_->data_; }
        U const& operator*() const { return node_->data_; }
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

    template <typename U = T>
    struct End : Iterator<U> {};

    cxx::Ref<Node<T>> front_ {nullptr};

    using value_type = T;
    using iterator = Iterator<T>;
    using const_iterator = Iterator<T> const;

    ~LinkedList() noexcept = default;
    LinkedList() noexcept = default;
    LinkedList(LinkedList const&) noexcept = default;
    LinkedList& operator=(LinkedList const&) noexcept = default;

    iterator begin() const { return {front_}; }
    iterator end(this auto) { return End {}; }
    const_iterator cbegin() const { return {front_}; }
    const_iterator cend(this auto) { return End {}; }
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

    void push(cxx::Ref<T> item) {
        assert(item);
        auto newFront = cxx::Ref<Node<T>>::make(item, front_);
        front_ = newFront;
    }
};

struct JSON;
struct ArrayRepr;

struct JSON final {
    cxx::Ref<NumRepr> num_;
    cxx::Ref<ArrayRepr> arr_;

    Repr const& val() const noexcept;

    ~JSON() noexcept = default;

    JSON() : JSON(nullptr, 0.0f) {}
    JSON(JSON const&) = default;
    JSON& operator=(JSON const&) = default;

    JSON(cxx::Ref<NumRepr> val, float) : num_(std::move(val)) {}

    JSON(int val);
    JSON(std::array<int, 1> val);
    bool operator==(JSON const& rhs) const;
};

struct ArrayRepr : Repr {
    LinkedList<JSON> vec_;

    ~ArrayRepr() noexcept = default;
    ArrayRepr() noexcept = default;

    static bool arraysEqual(auto& a1, auto& a2);
    bool operator==(ArrayRepr const& rhs) const { return arraysEqual(vec_, rhs.vec_); }
};

bool JSON::operator==(JSON const& rhs) const { return val() == rhs.val(); }

JSON::JSON(int val) : JSON(cxx::Ref<NumRepr>::make(val), 0) {}

bool ArrayRepr::arraysEqual(auto& a1, auto& a2) {
    if (a1.size() != a2.size()) { return false; }
    auto end = a1.end();
    auto it1 = a1.begin();
    auto it2 = a2.begin();
    while (it1 != end) {
        if (*it1++ != *it2++) { return false; }
    }
    return true;
}

JSON::JSON(std::array<int, 1> val) : arr_(cxx::Ref<ArrayRepr>::make()) {
    for (auto const& item : val) { arr_->vec_.push(cxx::Ref<JSON>::make(item)); }
}

Repr const& JSON::val() const noexcept {
    if (num_) { return *num_; }
    if (arr_) { return *arr_; }
    std::unreachable();
}

}  // namespace

int main() {
    // Number of times a `Foo` object was constructed or destructed
    assert(ctorCount == 0);
    assert(dtorCount == 0);
    auto noLeaks = [&]() { return dtorCount == ctorCount; };

    ctorCount = dtorCount = 0;
    std::cerr << "basic: make, no args" << std::endl;  // One day I'll get around to StackTrace...
    {
        auto ref = cxx::Ref<Foo>::make();
        assert(ref->a == 42);
        assert(ref->b == 59.25);
        assert(ref->c == "hello");
        assert(ctorCount == 1);
        assert(dtorCount == 0);
    }
    assert(noLeaks());

    ctorCount = dtorCount = 0;
    std::cerr << "basic/make with args" << std::endl;
    {
        auto ref = cxx::Ref<Foo>::make(1, 2, "test3");
        assert(ref->a == 1);
        assert(ref->b == 2);
        assert(ref->c == "test3");
        assert(ctorCount == 1);
        assert(dtorCount == 0);
    }
    assert(noLeaks());

    ctorCount = dtorCount = 0;
    std::cerr << "copy ctor" << std::endl;
    {
        auto ref = cxx::Ref<Foo>::make();
        std::cerr << "copy ctor: before: refs=" << ref.refs() << std::endl;
        assert(ref.refs() == 1);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
        auto ref2(ref);
        std::cerr << "copy ctor: after:  refs=" << ref.refs() << std::endl;
        assert(ref.refs() == 2);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
    }
    assert(noLeaks());

    ctorCount = dtorCount = 0;
    std::cerr << "copy assign" << std::endl;
    {
        auto ref = cxx::Ref<Foo>::make();
        assert(ref.refs() == 1);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
        auto ref2 = ref;
        assert(ref.refs() == 2);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
    }
    assert(noLeaks());

    ctorCount = dtorCount = 0;
    std::cerr << "move ctor" << std::endl;
    {
        auto ref = cxx::Ref<Foo>::make();
        assert(ref.refs() == 1);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
        auto ref2(std::move(ref));
        assert(ref2.refs() == 1);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
    }
    assert(noLeaks());

    ctorCount = dtorCount = 0;
    std::cerr << "move assign" << std::endl;
    {
        auto ref = cxx::Ref<Foo>::make();
        assert(ref.refs() == 1);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
        auto ref2 = std::move(ref);
        assert(ref2.refs() == 1);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
    }
    assert(noLeaks());

    struct Bar {
        cxx::Ref<Foo> foo;
    };

    ctorCount = dtorCount = 0;
    std::cerr << "ref inside a struct" << std::endl;
    {
        Bar bar {cxx::Ref<Foo>::make()};
        assert(bar.foo.refs() == 1);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
        auto bar2(bar);
        assert(bar.foo.refs() == 2);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
    }
    assert(noLeaks());

    ctorCount = dtorCount = 0;
    std::cerr << "ref inside a ref" << std::endl;
    {
        auto bar = cxx::Ref<Bar>::make(cxx::Ref<Foo>::make());
        assert(bar->foo.refs() == 1);
        assert(bar.refs() == 1);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
        auto bar2(bar);
        assert(bar->foo.refs() == 1);
        assert(bar.refs() == 2);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
    }
    assert(noLeaks());

    ctorCount = dtorCount = 0;
    std::cerr << "ref inside a vector" << std::endl;
    {
        auto foo = cxx::Ref<Foo>::make();
        assert(foo.refs() == 1);
        std::vector<cxx::Ref<Foo>> vec;
        for (int i = 0; i < 10; i++) { vec.push_back(foo); }
        assert(foo.refs() == 11);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
    }
    assert(noLeaks());

    ctorCount = dtorCount = 0;
    std::cerr << "ref inside a vector ref" << std::endl;
    {
        auto bar = cxx::Ref<Bar>::make(cxx::Ref<Foo>::make());
        assert(bar->foo.refs() == 1);
        assert(bar.refs() == 1);
        std::vector<cxx::Ref<Bar>> vec;
        for (int i = 0; i < 10; i++) { vec.push_back(bar); }
        assert(bar->foo.refs() == 1);
        assert(bar.refs() == 11);
        assert(ctorCount == 1);
        assert(dtorCount == 0);
    }
    assert(noLeaks());

    ctorCount = dtorCount = 0;
    std::cerr << "ref from generator" << std::endl;
    {
        auto foo = cxx::Ref<Foo>::make();
        auto gen = [&]() -> cxx::Generator<cxx::Ref<Foo>> { co_yield foo; };
        for (auto f : gen()) {}
    }
    assert(noLeaks());

    ctorCount = dtorCount = 0;
    std::cerr << "ref within a mini-JSON lib" << std::endl;
    { JSON j(std::array<int, 1> {0}); }
    assert(noLeaks());

    // TODO tests which verify thread-safety

    return 0;
}
