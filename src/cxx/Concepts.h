#pragma once
static_assert(__cplusplus >= 202300L, "cxx-libs requires C++23");
// (c) 2024 Steve O'Brien -- MIT License

#include <type_traits>

namespace cxx {

template <typename A, typename B>
concept SameRCV = std::is_same_v<std::remove_cvref_t<A>, std::remove_cvref_t<B>>;

template <typename A, typename B>
concept DifferentRCV = (!SameRCV<A, B>);

/** Something that's literally a `bool` and not just convertible to a `bool` */
template <typename B>
concept Bool = std::is_same_v<std::remove_cv_t<B>, bool>;

template <typename N>
concept NumericNotBool = (!Bool<N>) && (std::is_integral_v<N> || std::is_floating_point_v<N>);

// https://en.cppreference.com/w/cpp/named_req/SequenceContainer
template <typename S>
concept SequenceContainer = requires {
    typename S::value_type;
    typename S::iterator;
    typename S::const_iterator;
    typename S::reference;
    typename S::const_reference;
    std::is_convertible_v<typename S::iterator, typename S::const_iterator>;
    // s.begin(), .end() -> iterator
    // s.cbegin(), cend() -> const_iterator
};

template <typename S, typename C>
concept SequenceContainerOf = SequenceContainer<S> && requires {
    std::is_same_v<C, typename S::value_type>;
};

}  // namespace cxx
