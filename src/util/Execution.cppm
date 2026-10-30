module;

#include <version>

#if !defined(__EMSCRIPTEN__) && !defined(SHARK_NO_PARALLEL_EXECUTION) && defined(__cpp_lib_execution)
#define SHARK_STD_PARALLEL_EXECUTION
#elif defined(__EMSCRIPTEN_PTHREADS__) && !defined(SHARK_NO_PARALLEL_EXECUTION)
#define SHARK_POOL_PARALLEL_EXECUTION
#endif

export module shark:util.Execution;

import std;

export namespace shark::exec {

#ifdef SHARK_STD_PARALLEL_EXECUTION

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

} // namespace shark::exec

namespace shark::exec {

template <typename Policy>
inline constexpr bool is_parallel_policy_v =
    std::is_same_v<std::remove_cvref_t<Policy>, parallel_policy>
    || std::is_same_v<std::remove_cvref_t<Policy>, parallel_unsequenced_policy>;

#ifdef SHARK_STD_PARALLEL_EXECUTION

inline constexpr bool has_parallel_execution = true;

template <typename Policy, typename Iter, typename Fn>
inline void dispatch_for_each(Policy&& policy, Iter first, Iter last, Fn&& fn) {
    std::for_each(std::forward<Policy>(policy), first, last, std::forward<Fn>(fn));
}

template <typename Policy, typename Iter, typename T>
inline void dispatch_fill(Policy&& policy, Iter first, Iter last, const T& value) {
    std::fill(std::forward<Policy>(policy), first, last, value);
}

template <typename Policy, typename Iter, typename Size, typename T>
inline Iter dispatch_fill_n(Policy&& policy, Iter first, Size count, const T& value) {
    return std::fill_n(std::forward<Policy>(policy), first, count, value);
}

template <typename Policy, typename Iter, typename T, typename ReduceOp, typename TransformOp>
[[nodiscard]] inline T dispatch_transform_reduce(
    Policy&& policy, Iter first, Iter last, T init, ReduceOp&& reduceOp, TransformOp&& transformOp
) {
    return std::transform_reduce(
        std::forward<Policy>(policy), first, last, std::move(init), std::forward<ReduceOp>(reduceOp),
        std::forward<TransformOp>(transformOp)
    );
}

#elif defined(SHARK_POOL_PARALLEL_EXECUTION)

class WorkerPool {
public:
    using ChunkFn = void (*)(void* context, std::size_t begin, std::size_t end);

    static constexpr unsigned MAX_WORKERS     = 15;
    static constexpr int      SPIN_ITERATIONS = 1 << 16;

    static WorkerPool& instance() {
        static WorkerPool pool;
        return pool;
    }

    WorkerPool(const WorkerPool&)            = delete;
    WorkerPool& operator=(const WorkerPool&) = delete;

    ~WorkerPool() {
        {
            std::lock_guard lock(mutex);
            stopping = true;
        }
        wake.notify_all();
        for (std::thread& worker : workers) {
            worker.join();
        }
    }

    [[nodiscard]] std::size_t threadCount() const { return workers.size() + 1; }

    void run(std::size_t count, std::size_t chunkSize, void* context, ChunkFn fn) {
        if (insideJob || workers.empty()) {
            fn(context, 0, count);
            return;
        }
        std::unique_lock runLock(runMutex, std::try_to_lock);
        if (!runLock.owns_lock()) {
            fn(context, 0, count);
            return;
        }

        const Job current{.context = context, .fn = fn, .count = count, .chunkSize = chunkSize};
        {
            std::lock_guard lock(mutex);
            job = current;
            nextChunk.store(0, std::memory_order_relaxed);
            busyWorkers.store(readyWorkers, std::memory_order_relaxed);
            generation.fetch_add(1, std::memory_order_release);
        }
        wake.notify_all();

        insideJob = true;
        runChunks(current);
        insideJob = false;

        while (busyWorkers.load(std::memory_order_acquire) != 0) {
        }
    }

private:
    struct Job {
        void*       context   = nullptr;
        ChunkFn     fn        = nullptr;
        std::size_t count     = 0;
        std::size_t chunkSize = 1;
    };

    WorkerPool() {
        const unsigned cores = std::max(1u, std::thread::hardware_concurrency());
        const unsigned count = std::min(cores - 1, MAX_WORKERS);
        workers.reserve(count);
        for (unsigned i = 0; i < count; ++i) {
            workers.emplace_back([this] { workerLoop(); });
        }
    }

    void runChunks(const Job& current) {
        const std::size_t chunks = (current.count + current.chunkSize - 1) / current.chunkSize;
        for (std::size_t c = nextChunk.fetch_add(1, std::memory_order_relaxed); c < chunks;
             c             = nextChunk.fetch_add(1, std::memory_order_relaxed)) {
            const std::size_t begin = c * current.chunkSize;
            current.fn(current.context, begin, std::min(begin + current.chunkSize, current.count));
        }
    }

    void workerLoop() {
        insideJob          = true;
        std::uint64_t seen = 0;
        {
            std::lock_guard lock(mutex);
            seen = generation.load(std::memory_order_relaxed);
            ++readyWorkers;
        }
        while (true) {
            std::uint64_t latest = generation.load(std::memory_order_acquire);
            for (int spin = 0; spin < SPIN_ITERATIONS && latest == seen; ++spin) {
                latest = generation.load(std::memory_order_acquire);
            }
            if (latest == seen) {
                std::unique_lock lock(mutex);
                wake.wait(lock, [&] { return stopping || generation.load(std::memory_order_relaxed) != seen; });
                if (stopping) {
                    return;
                }
                latest = generation.load(std::memory_order_relaxed);
            }
            seen = latest;

            runChunks(job);
            busyWorkers.fetch_sub(1, std::memory_order_release);
        }
    }

    static inline thread_local bool insideJob = false;

    std::vector<std::thread>   workers;
    std::mutex                 runMutex;
    std::mutex                 mutex;
    std::condition_variable    wake;
    Job                        job;
    std::atomic<std::size_t>   nextChunk{0};
    std::atomic<std::uint64_t> generation{0};
    std::atomic<std::size_t>   busyWorkers{0};
    std::size_t                readyWorkers = 0;
    bool                       stopping     = false;
};

inline constexpr bool has_parallel_execution = true;

inline constexpr std::size_t MIN_PARALLEL_COUNT = 256;

[[nodiscard]] inline std::size_t pool_chunk_size(std::size_t count) {
    constexpr std::size_t MIN_CHUNK = 64;
    return std::max(MIN_CHUNK, count / (WorkerPool::instance().threadCount() * 8));
}

template <typename Body> inline void pool_run(std::size_t count, std::size_t chunkSize, Body& body) {
    WorkerPool::instance().run(count, chunkSize, &body, [](void* context, std::size_t begin, std::size_t end) {
        (*static_cast<Body*>(context))(begin, end);
    });
}

template <typename Policy, typename Iter>
inline constexpr bool runs_on_pool_v = is_parallel_policy_v<Policy> && std::random_access_iterator<Iter>;

template <typename Policy, typename Iter, typename Fn>
inline void dispatch_for_each(Policy&&, Iter first, Iter last, Fn&& fn) {
    if constexpr (runs_on_pool_v<Policy, Iter>) {
        const auto count = static_cast<std::size_t>(last - first);
        if (count >= MIN_PARALLEL_COUNT) {
            auto body = [first, &fn](std::size_t begin, std::size_t end) {
                for (std::size_t i = begin; i < end; ++i) {
                    fn(first[static_cast<std::iter_difference_t<Iter>>(i)]);
                }
            };
            pool_run(count, pool_chunk_size(count), body);
            return;
        }
    }
    std::for_each(first, last, std::forward<Fn>(fn));
}

template <typename Policy, typename Iter, typename T>
inline void dispatch_fill(Policy&&, Iter first, Iter last, const T& value) {
    std::fill(first, last, value);
}

template <typename Policy, typename Iter, typename Size, typename T>
inline Iter dispatch_fill_n(Policy&&, Iter first, Size count, const T& value) {
    return std::fill_n(first, count, value);
}

template <typename Policy, typename Iter, typename T, typename ReduceOp, typename TransformOp>
[[nodiscard]] inline T
dispatch_transform_reduce(Policy&&, Iter first, Iter last, T init, ReduceOp&& reduceOp, TransformOp&& transformOp) {
    if constexpr (runs_on_pool_v<Policy, Iter>) {
        const auto count = static_cast<std::size_t>(last - first);
        if (count >= MIN_PARALLEL_COUNT) {
            const std::size_t             chunkSize = pool_chunk_size(count);
            std::vector<std::optional<T>> partials((count + chunkSize - 1) / chunkSize);
            auto body = [first, chunkSize, &partials, &reduceOp, &transformOp](std::size_t begin, std::size_t end) {
                auto at  = [first](std::size_t i) { return first[static_cast<std::iter_difference_t<Iter>>(i)]; };
                T    acc = transformOp(at(begin));
                for (std::size_t i = begin + 1; i < end; ++i) {
                    acc = reduceOp(std::move(acc), transformOp(at(i)));
                }
                partials[begin / chunkSize].emplace(std::move(acc));
            };
            pool_run(count, chunkSize, body);
            for (std::optional<T>& partial : partials) {
                init = reduceOp(std::move(init), std::move(*partial));
            }
            return init;
        }
    }
    return std::transform_reduce(
        first, last, std::move(init), std::forward<ReduceOp>(reduceOp), std::forward<TransformOp>(transformOp)
    );
}

#else

inline constexpr bool has_parallel_execution = false;

template <typename Policy, typename Iter, typename Fn>
inline void dispatch_for_each(Policy&&, Iter first, Iter last, Fn&& fn) {
    std::for_each(first, last, std::forward<Fn>(fn));
}

template <typename Policy, typename Iter, typename T>
inline void dispatch_fill(Policy&&, Iter first, Iter last, const T& value) {
    std::fill(first, last, value);
}

template <typename Policy, typename Iter, typename Size, typename T>
inline Iter dispatch_fill_n(Policy&&, Iter first, Size count, const T& value) {
    return std::fill_n(first, count, value);
}

template <typename Policy, typename Iter, typename T, typename ReduceOp, typename TransformOp>
[[nodiscard]] inline T
dispatch_transform_reduce(Policy&&, Iter first, Iter last, T init, ReduceOp&& reduceOp, TransformOp&& transformOp) {
    return std::transform_reduce(
        first, last, std::move(init), std::forward<ReduceOp>(reduceOp), std::forward<TransformOp>(transformOp)
    );
}

#endif

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

    constexpr inline IndexIterator() noexcept = default;
    constexpr inline explicit IndexIterator(Index v) noexcept : value(v) {}

    [[nodiscard]] constexpr inline Index operator*() const noexcept { return value; }
    [[nodiscard]] constexpr inline Index operator[](difference_type n) const noexcept {
        return static_cast<Index>(static_cast<difference_type>(value) + n);
    }

    constexpr inline IndexIterator& operator++() noexcept {
        ++value;
        return *this;
    }
    constexpr inline IndexIterator operator++(int) noexcept {
        IndexIterator tmp = *this;
        ++value;
        return tmp;
    }
    constexpr inline IndexIterator& operator--() noexcept {
        --value;
        return *this;
    }
    constexpr inline IndexIterator operator--(int) noexcept {
        IndexIterator tmp = *this;
        --value;
        return tmp;
    }

    constexpr inline IndexIterator& operator+=(difference_type n) noexcept {
        value = static_cast<Index>(static_cast<difference_type>(value) + n);
        return *this;
    }
    constexpr inline IndexIterator& operator-=(difference_type n) noexcept {
        value = static_cast<Index>(static_cast<difference_type>(value) - n);
        return *this;
    }

    [[nodiscard]] friend constexpr inline IndexIterator operator+(IndexIterator it, difference_type n) noexcept {
        it += n;
        return it;
    }
    [[nodiscard]] friend constexpr inline IndexIterator operator+(difference_type n, IndexIterator it) noexcept {
        it += n;
        return it;
    }
    [[nodiscard]] friend constexpr inline IndexIterator operator-(IndexIterator it, difference_type n) noexcept {
        it -= n;
        return it;
    }
    [[nodiscard]] friend constexpr inline difference_type operator-(IndexIterator a, IndexIterator b) noexcept {
        return static_cast<difference_type>(a.value) - static_cast<difference_type>(b.value);
    }

    [[nodiscard]] friend constexpr inline bool operator==(IndexIterator a, IndexIterator b) noexcept  = default;
    [[nodiscard]] friend constexpr inline auto operator<=>(IndexIterator a, IndexIterator b) noexcept = default;
};

} // namespace shark::exec

export namespace shark::exec {

template <typename Policy, typename Iter, typename Fn>
inline void for_each(Policy&& policy, Iter first, Iter last, Fn&& fn) {
    dispatch_for_each(std::forward<Policy>(policy), first, last, std::forward<Fn>(fn));
}

template <typename Policy, std::ranges::input_range Range, typename Fn>
inline void for_each(Policy&& policy, Range&& range, Fn&& fn) {
    shark::exec::for_each(
        std::forward<Policy>(policy), std::ranges::begin(range), std::ranges::end(range), std::forward<Fn>(fn)
    );
}

template <typename Policy, typename Index, typename Fn>
    requires std::is_integral_v<Index>
inline void for_each_index(Policy&& policy, Index begin, Index end, Fn&& fn) {
    if (begin >= end) {
        return;
    }
    if constexpr (has_parallel_execution && is_parallel_policy_v<Policy>) {
        dispatch_for_each(
            std::forward<Policy>(policy), IndexIterator<Index>(begin), IndexIterator<Index>(end), std::forward<Fn>(fn)
        );
        return;
    }
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
inline void fill(Policy&& policy, Iter first, Iter last, const T& value) {
    dispatch_fill(std::forward<Policy>(policy), first, last, value);
}

template <typename Policy, std::ranges::input_range Range, typename T>
inline void fill(Policy&& policy, Range&& range, const T& value) {
    shark::exec::fill(std::forward<Policy>(policy), std::ranges::begin(range), std::ranges::end(range), value);
}

template <typename Policy, typename Iter, typename Size, typename T>
inline Iter fill_n(Policy&& policy, Iter first, Size count, const T& value) {
    return dispatch_fill_n(std::forward<Policy>(policy), first, count, value);
}

template <typename Policy, typename Iter, typename T, typename ReduceOp, typename TransformOp>
[[nodiscard]] inline T
transform_reduce(Policy&& policy, Iter first, Iter last, T init, ReduceOp&& reduceOp, TransformOp&& transformOp) {
    return dispatch_transform_reduce(
        std::forward<Policy>(policy), first, last, std::move(init), std::forward<ReduceOp>(reduceOp),
        std::forward<TransformOp>(transformOp)
    );
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
    Policy&& policy, Index begin, Index end, T init, ReduceOp&& reduceOp, TransformOp&& transformOp
) {
    if (begin >= end) {
        return init;
    }
    if constexpr (has_parallel_execution && is_parallel_policy_v<Policy>) {
        return dispatch_transform_reduce(
            std::forward<Policy>(policy), IndexIterator<Index>(begin), IndexIterator<Index>(end), std::move(init),
            std::forward<ReduceOp>(reduceOp), std::forward<TransformOp>(transformOp)
        );
    }
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
