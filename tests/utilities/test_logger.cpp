#include <chrono>
#include <gtest/gtest.h>
#include <latch>
#include <thread>
#include <type_traits>
#include <vector>

#include "logger.h"

using sclock = std::chrono::system_clock;
using namespace std::chrono_literals;

static_assert(!std::is_copy_constructible_v<Logger>);
static_assert(!std::is_copy_assignable_v<Logger>);
static_assert(!std::is_move_constructible_v<Logger>);
static_assert(!std::is_move_assignable_v<Logger>);

TEST(LoggerSingleton, SameAddressAcrossCalls) {
    auto& a = Logger::getInstance();
    auto& b = Logger::getInstance();
    EXPECT_EQ(&a, &b);           // same object
}

TEST(LoggerSingleton, SingleInstanceAcrossThreads) {
    constexpr int N = 16;
    std::latch start(1);
    Logger* ptrs[N] = {};
    std::vector<std::thread> ts;
    ts.reserve(N);

    for (int i = 0; i < N; ++i) {
        ts.emplace_back([&, i]{
            start.wait();                        // start together
            ptrs[i] = &Logger::getInstance();
        });
    }
    start.count_down();
    for (auto& t : ts) t.join();

    for (int i = 1; i < N; ++i) {
        EXPECT_EQ(ptrs[0], ptrs[i]);            // all got the same address
    }
}

TEST(LogMessageOrder, BasicLessGreater) {
    auto t0 = sclock::time_point{} + 10ms;
    auto t1 = t0 + 1ms;

    LogMessage a{LogLevel::INFO, t0, "a", std::thread::id{}};
    LogMessage b{LogLevel::INFO, t1, "b", std::thread::id{}};

    EXPECT_LT(a, b);
    EXPECT_GT(b, a);
}

TEST(LogMessageOrder, EqualTimestampsAreEquivalent) {
    auto t = sclock::time_point{} + 42ms;
    LogMessage x{LogLevel::INFO, t, "x", {}};
    LogMessage y{LogLevel::WARNING, t, "y", {}};

    EXPECT_FALSE(x < y);
    EXPECT_FALSE(x > y);
}

TEST(PriorityQueue, Order) {
    ThreadSafePriorityQueue<LogMessage> logQueue;

    auto t0 = sclock::time_point{} + 10ms;
    auto t1 = t0 + 1ms;
    auto t2 = t0 - 5ms;

    logQueue.push({LogLevel::INFO, t0, "0", {}});
    logQueue.push({LogLevel::INFO, t1, "1", {}});
    logQueue.push({LogLevel::INFO, t2, "2", {}});

    EXPECT_EQ(logQueue.top().timestamp, t2);
}
