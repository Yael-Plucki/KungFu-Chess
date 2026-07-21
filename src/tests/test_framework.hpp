#pragma once

#include <iostream>
#include <string>

inline int test_failures = 0;

#define EXPECT_TRUE(expr) \
    do { \
        if (!(expr)) { \
            std::cerr << "FAIL: " << __FILE__ << ":" << __LINE__ << " EXPECT_TRUE(" #expr ")\n"; \
            ++test_failures; \
        } \
    } while (0)

#define EXPECT_FALSE(expr) EXPECT_TRUE(!(expr))

#define EXPECT_EQ(actual, expected) \
    do { \
        const auto& _actual = (actual); \
        const auto& _expected = (expected); \
        if (!(_actual == _expected)) { \
            std::cerr << "FAIL: " << __FILE__ << ":" << __LINE__ \
                      << " EXPECT_EQ(" #actual ", " #expected ")\n"; \
            ++test_failures; \
        } \
    } while (0)

#define RUN_TEST(fn) \
    do { \
        test_failures = 0; \
        fn(); \
        if (test_failures == 0) { \
            std::cout << "PASS: " #fn "\n"; \
        } else { \
            std::cout << "FAIL: " #fn " (" << test_failures << " failures)\n"; \
        } \
    } while (0)
