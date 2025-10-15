#include <gtest/gtest.h>
#include <mutex>
#include <condition_variable>
#include <vector>

#include "logger.h"

static std::mutex g_m;
static std::condition_variable g_cv;
static std::vector<LogMessage> g_seen;

static void TestObserver(LogMessage const& m) {
    std::lock_guard<std::mutex> lk(g_m);
    g_seen.push_back(m);
    g_cv.notify_all();
}

struct LoggerTapFixture : ::testing::Test {
    void SetUp() override {
#ifdef LOGGER_TEST_HOOKS
        g_seen.clear();
        setLogTestObserver(&TestObserver);
#endif
    }
    void TearDown() override {
#ifdef LOGGER_TEST_HOOKS
        setLogTestObserver(nullptr);
#endif
    }
};

TEST_F(LoggerTapFixture, InfoMacroEmitsAndCaptures) {
    LOG_INFO() << "hello";
    std::unique_lock<std::mutex> lk(g_m);
    g_cv.wait_for(lk, std::chrono::milliseconds(100), []{ return !g_seen.empty(); });
    ASSERT_FALSE(g_seen.empty());
    EXPECT_EQ(g_seen.back().level, LogLevel::INFO);
    EXPECT_EQ(g_seen.back().message, "hello");
}

TEST_F(LoggerTapFixture, DebugBehaviorDependsOnNDEBUG) {
    LOG_DEBUG() << "dbg";
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
#ifdef NDEBUG
    EXPECT_TRUE(g_seen.empty());
#else
    EXPECT_FALSE(g_seen.empty());
    EXPECT_EQ(g_seen.back().level, LogLevel::DEBUG);
#endif
}
