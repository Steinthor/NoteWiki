#include <gtest/gtest.h>
#include <latch>
#include <thread>
#include <type_traits>
#include <vector>

#include "logger.h"

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