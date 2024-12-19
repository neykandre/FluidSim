#include <gtest/gtest.h>
#include "../include/Types.hpp"
#include <sstream>
#include <limits>
#include <stdexcept>

using namespace Fluid;

TEST(FixedTest, ConstructorTests) {
    // Test int constructor
    Fixed<32, 16> f1(5);
    std::stringstream ss;
    ss << f1;
    EXPECT_EQ(ss.str(), "5");
    ss.str("");

    // Test float constructor
    Fixed<32, 16> f2(3.14f);
    ss << f2;
    EXPECT_EQ(ss.str(), "3.14");
    ss.str("");

    // Test double constructor
    Fixed<32, 16> f3(2.7);
    ss << f3;
    EXPECT_EQ(ss.str(), "2.7");
    ss.str("");

    // Test default constructor
    Fixed<32, 16> f4;
    ss << f4;
    EXPECT_EQ(ss.str(), "0");
    ss.str("");

    // Test constructor for copying from another Fixed type
    Fixed<32, 20> f5(5.5);
    Fixed<32, 16> f6(f5);
    ss << f6;
    EXPECT_EQ(ss.str(), "5.5");
}

TEST(FixedTest, ArithmeticOperators) {
    Fixed<64, 16> a(0.6f), b(0.3f);

    // Test operator+=
    a += b;
    
    EXPECT_EQ(a.v, );

    // Test operator-=
    a -= b;
    EXPECT_EQ(a, 0.6);

    // Test operator*=
    a *= b;
    EXPECT_EQ(a, 0.18);

    // Test operator/=
    a /= b;
    EXPECT_EQ(a, 2);

    // Test operator+
    Fixed<32, 16> c = a + b;
    EXPECT_EQ(c, 0.9);

    // Test operator-
    c = a - b;
    EXPECT_EQ(c, 0.3);

    // Test operator*
    c = a * b;
    EXPECT_EQ(c, 0.18);

    // Test operator/
    c = a / b;
    EXPECT_EQ(c, 2);

    // Test division by zero
    EXPECT_THROW((a /= Fixed<32, 16>(0)), std::runtime_error);
    EXPECT_THROW((a / Fixed<32, 16>(0)), std::runtime_error);
}

TEST(FixedTest, ComparisonOperators) {
    Fixed<32, 16> a(5), b(3), c(5);

    // Test operator==
    EXPECT_EQ(a, c);
    EXPECT_NE(a, b);

    // Test operator<=>
    EXPECT_GT(a, b);
    EXPECT_LT(b, a);
    EXPECT_LE(a, c);
    EXPECT_GE(c, a);
}

TEST(FixedTest, UnaryOperators) {
    Fixed<32, 16> a(5);

    // Test operator-()
    Fixed<32, 16> b = -a;
    EXPECT_EQ(b.v, -5 << 16);
}

TEST(FixedTest, Functions) {
    Fixed<32, 16> a(-5);

    // Test abs
    Fixed<32, 16> b = abs(a);
    EXPECT_EQ(b.v, 5 << 16);

    // Test from_raw
    Fixed<32, 16> c = Fixed<32, 16>::from_raw(5 << 16);
    EXPECT_EQ(c.v, 5 << 16);
}

TEST(FixedTest, DoubleOperations) {
    Fixed<32, 16> a(5);
    double b = 3.0;

    // Test addition
    EXPECT_EQ((a + Fixed<32, 16>(b)).v, 8 << 16);
    EXPECT_EQ((Fixed<32, 16>(b) + a).v, 8 << 16);

    // Test subtraction
    EXPECT_EQ((a - Fixed<32, 16>(b)).v, 2 << 16);
    EXPECT_EQ((Fixed<32, 16>(b) - a).v, -2 << 16);

    // Test multiplication
    EXPECT_EQ((a * Fixed<32, 16>(b)).v, 15 << 16);
    EXPECT_EQ((Fixed<32, 16>(b) * a).v, 15 << 16);

    // Test division
    EXPECT_EQ((a / Fixed<32, 16>(b)).v, (5 << 16) / 3);
    EXPECT_EQ((Fixed<32, 16>(b) / a).v, (3 << 16) / 5);
}

TEST(FixedTest, EdgeCases) {
    Fixed<32, 16> a(0), b(1), c(std::numeric_limits<int32_t>::max() >> 16);

    // Test operations with zero
    EXPECT_EQ((a + b).v, 1 << 16);
    EXPECT_EQ((b - a).v, 1 << 16);
    EXPECT_EQ((a * c).v, 0);
    EXPECT_THROW(a / a, std::runtime_error);

    // Test overflow
    EXPECT_NE((c + c).v, (c.v << 1));  // Overflow expected

    // Test underflow
    Fixed<32, 16> d(std::numeric_limits<int32_t>::min() >> 16);
    EXPECT_NE((d - b).v, (d.v - (1 << 16)));  // Underflow expected
}

TEST(FixedTest, OutputOperator) {
    Fixed<32, 16> a(3);
    std::ostringstream oss;
    oss << a;
    EXPECT_EQ(oss.str(), "3");

    oss.str("");
    Fixed<32, 16> b(3.5);
    oss << b;
    EXPECT_EQ(oss.str(), "3.5");

    oss.str("");
    Fixed<32, 16> c(-2.25);
    oss << c;
    EXPECT_EQ(oss.str(), "-2.25");
}