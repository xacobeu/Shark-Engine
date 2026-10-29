#pragma once

#include <algorithm>
#include <compare>
#include <cstddef>
#include <functional>
#include <iterator>
#include <numeric>
#include <ranges>
#include <type_traits>
#include <utility>
#include <version>

#if !defined(__EMSCRIPTEN__) && !defined(SHARK_NO_PARALLEL_EXECUTION) && __has_include(<execution>)
#include <execution>
#if defined(__cpp_lib_execution)
#define SHARK_HAS_PARALLEL_EXECUTION 1
#endif
#endif

namespace shark::exec {

#if defined(SHARK_HAS_PARALLEL_EXECUTION)

using sequenced_policy            = std::execution::sequenced_policy;
using parallel_policy             = std::execution::parallel_policy;
using parallel_unsequenced_policy = std::execution::parallel_unsequenced_policy;
using unsequenced_policy          = std::execution::unsequenced_policy;

inline constexpr sequenced_policy            seq       = std::execution::seq;
inline constexpr parallel_policy             par       = std::execution::par;
inline constexpr parallel_unsequenced_policy par_unseq = std::execution::par_unseq;
inline constexpr unsequenced_policy          unseq     = std::execution::unseq;

#else

struct sequenced_policy {};
struct parallel_policy {};
struct parallel_unsequenced_policy {};
struct unsequenced_policy {};

inline constexpr sequenced_policy            seq{};
inline constexpr parallel_policy             par{};
inline constexpr parallel_unsequenced_policy par_unseq{};
inline constexpr unsequenced_policy          unseq{};

#endif

namespace detail {

// std::views::iota is input-iterator tagged, so PSTL runs it serially; this one is random-access.
template <typename Index> struct IndexIterator {
    static_assert(std::is_integral_v<Index>, "IndexIterator requires an integral index type");

    using iterator_category = std::random_access_iterator_tag;
    using iterator_concept  = std::random_access_iterator_tag;
    using value_type        = Index;
    using difference_type   = std::ptrdiff_t;
    using pointer           = const Index*;
    using reference         = Index;

    Index value{};

    constexpr IndexIterator() noexcept = default;
    constexpr explicit IndexIterator(Index v) noexcept : value(v) {}

    [[nodiscard]] constexpr Index operator*() const noexcept { return value; }
    [[nodiscard]] constexpr Index operator[](difference_type n) const noexcept {
        return static_cast<Index>(static_cast<difference_type>(value) + n);
    }

    constexpr IndexIterator& operator++() noexcept {
        ++value;
        return *this;
    }
    constexpr IndexIterator operator++(int) noexcept {
        IndexIterator tmp = *this;
        ++value;
        return tmp;
    }
    constexpr IndexIterator& operator--() noexcept {
        --value;
        return *this;
    }
    constexpr IndexIterator operator--(int) noexcept {
        IndexIterator tmp = *this;
        --value;
        return tmp;
    }

    constexpr IndexIterator& operator+=(difference_type n) noexcept {
        value = static_cast<Index>(static_cast<difference_type>(value) + n);
        return *this;
    }
    constexpr IndexIterator& operator-=(difference_type n) noexcept {
        value = static_cast<Index>(static_cast<difference_type>(value) - n);
        return *this;
    }

    [[nodiscard]] friend constexpr IndexIterator operator+(IndexIterator it, difference_type n) noexcept {
        it += n;
        return it;
    }
    [[nodiscard]] friend constexpr IndexIterator operator+(difference_type n, IndexIterator it) noexcept {
        it += n;
        return it;
    }
    [[nodiscard]] friend constexpr IndexIterator operator-(IndexIterator it, difference_type n) noexcept {
        it -= n;
        return it;
    }
    [[nodiscard]] friend constexpr difference_type operator-(IndexIterator a, IndexIterator b) noexcept {
        return static_cast<difference_type>(a.value) - static_cast<difference_type>(b.value);
    }

    [[nodiscard]] friend constexpr bool operator==(IndexIterator a, IndexIterator b) noexcept  = default;
    [[nodiscard]] friend constexpr auto operator<=>(IndexIterator a, IndexIterator b) noexcept = default;
};

template <typename Policy>
inline constexpr bool is_parallel_policy_v =
    std::is_same_v<std::remove_cvref_t<Policy>, parallel_policy>
    || std::is_same_v<std::remove_cvref_t<Policy>, parallel_unsequenced_policy>;

} // namespace detail

template <typename Policy, typename Iter, typename Fn>
inline void for_each([[maybe_unused]] Policy&& policy, Iter first, Iter last, Fn&& fn) {
#if defined(SHARK_HAS_PARALLEL_EXECUTION)
    std::for_each(std::forward<Policy>(policy), first, last, std::forward<Fn>(fn));
#else
    std::for_each(first, last, std::forward<Fn>(fn));
#endif
}

template <typename Policy, std::ranges::input_range Range, typename Fn>
inline void for_each(Policy&& policy, Range&& range, Fn&& fn) {
    shark::exec::for_each(
        std::forward<Policy>(policy), std::ranges::begin(range), std::ranges::end(range), std::forward<Fn>(fn)
    );
}

template <typename Policy, typename Index, typename Fn>
    requires std::is_integral_v<Index>
inline void for_each_index([[maybe_unused]] Policy&& policy, Index begin, Index end, Fn&& fn) {
    if (begin >= end) {
        return;
    }
#if defined(SHARK_HAS_PARALLEL_EXECUTION)
    if constexpr (detail::is_parallel_policy_v<Policy>) {
        std::for_each(
            std::forward<Policy>(policy), detail::IndexIterator<Index>(begin), detail::IndexIterator<Index>(end),
            std::forward<Fn>(fn)
        );
        return;
    }
#endif
    for (Index i = begin; i < end; ++i) {
        fn(i);
    }
}

template <typename Policy, typename Index, typename Fn>
    requires std::is_integral_v<Index>
inline void for_each_index(Policy&& policy, Index count, Fn&& fn) {
    shark::exec::for_each_index(std::forward<Policy>(policy), Index{0}, count, std::forward<Fn>(fn));
}

template <typename Policy, typename Iter, typename T>
inline void fill([[maybe_unused]] Policy&& policy, Iter first, Iter last, const T& value) {
#if defined(SHARK_HAS_PARALLEL_EXECUTION)
    std::fill(std::forward<Policy>(policy), first, last, value);
#else
    std::fill(first, last, value);
#endif
}

template <typename Policy, std::ranges::input_range Range, typename T>
inline void fill(Policy&& policy, Range&& range, const T& value) {
    shark::exec::fill(std::forward<Policy>(policy), std::ranges::begin(range), std::ranges::end(range), value);
}

template <typename Policy, typename Iter, typename Size, typename T>
inline Iter fill_n([[maybe_unused]] Policy&& policy, Iter first, Size count, const T& value) {
#if defined(SHARK_HAS_PARALLEL_EXECUTION)
    return std::fill_n(std::forward<Policy>(policy), first, count, value);
#else
    return std::fill_n(first, count, value);
#endif
}

template <typename Policy, typename Iter, typename T, typename ReduceOp, typename TransformOp>
[[nodiscard]] inline T transform_reduce(
    [[maybe_unused]] Policy&& policy, Iter first, Iter last, T init, ReduceOp&& reduceOp, TransformOp&& transformOp
) {
#if defined(SHARK_HAS_PARALLEL_EXECUTION)
    return std::transform_reduce(
        std::forward<Policy>(policy), first, last, std::move(init), std::forward<ReduceOp>(reduceOp),
        std::forward<TransformOp>(transformOp)
    );
#else
    return std::transform_reduce(
        first, last, std::move(init), std::forward<ReduceOp>(reduceOp), std::forward<TransformOp>(transformOp)
    );
#endif
}

template <typename Policy, std::ranges::input_range Range, typename T, typename ReduceOp, typename TransformOp>
[[nodiscard]] inline T
transform_reduce(Policy&& policy, Range&& range, T init, ReduceOp&& reduceOp, TransformOp&& transformOp) {
    return shark::exec::transform_reduce(
        std::forward<Policy>(policy), std::ranges::begin(range), std::ranges::end(range), std::move(init),
        std::forward<ReduceOp>(reduceOp), std::forward<TransformOp>(transformOp)
    );
}

template <typename Policy, typename Index, typename T, typename ReduceOp, typename TransformOp>
    requires std::is_integral_v<Index>
[[nodiscard]] inline T transform_reduce_index(
    [[maybe_unused]] Policy&& policy, Index begin, Index end, T init, ReduceOp&& reduceOp, TransformOp&& transformOp
) {
    if (begin >= end) {
        return init;
    }
#if defined(SHARK_HAS_PARALLEL_EXECUTION)
    if constexpr (detail::is_parallel_policy_v<Policy>) {
        return std::transform_reduce(
            std::forward<Policy>(policy), detail::IndexIterator<Index>(begin), detail::IndexIterator<Index>(end),
            std::move(init), std::forward<ReduceOp>(reduceOp), std::forward<TransformOp>(transformOp)
        );
    }
#endif
    for (Index i = begin; i < end; ++i) {
        init = reduceOp(std::move(init), transformOp(i));
    }
    return init;
}

template <typename Policy, typename Index, typename T, typename ReduceOp, typename TransformOp>
    requires std::is_integral_v<Index>
[[nodiscard]] inline T
transform_reduce_index(Policy&& policy, Index count, T init, ReduceOp&& reduceOp, TransformOp&& transformOp) {
    return shark::exec::transform_reduce_index(
        std::forward<Policy>(policy), Index{0}, count, std::move(init), std::forward<ReduceOp>(reduceOp),
        std::forward<TransformOp>(transformOp)
    );
}

} // namespace shark::exec
