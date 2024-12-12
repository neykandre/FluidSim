#include "gtest/gtest.h"
#include "../include/Types.hpp"
#include <sstream>

using namespace Fluid;

TEST(Fixed, Constructor) {
    auto a = Fixed<8, 1>(18);
    auto b = Fixed<16, 7> (42);
    auto c = Fixed<32, 11>(52);
    auto d = Fixed<64, 0>(13);

    EXPECT_EQ(typeid(a.v) , typeid(int8_t));
    EXPECT_EQ(typeid(b.v) , typeid(int16_t));
    EXPECT_EQ(typeid(c.v) , typeid(int32_t));
    EXPECT_EQ(typeid(d.v) , typeid(int64_t));
}

TEST (Fixed, Print) {
    auto a = Fixed<8, 1>(18);
    auto b = Fixed<16, 7> (42);
    auto c = Fixed<32, 11>(52);
    auto d = Fixed<64, 0>(13);
    
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "18");
    ss.str("");
    ss << b;
    EXPECT_EQ(ss.str(), "42");
    ss.str("");
    ss << c;
    EXPECT_EQ(ss.str(), "52");
    ss.str("");
    ss << d;
    EXPECT_EQ(ss.str(), "13");
    ss.str("");

    auto e = Fixed<32, 20>::from_raw(0x20000L);
    ss << e;
    EXPECT_EQ(ss.str(), "0.125");
}

TEST (Fixed, OperatorAdd) {
    auto a = Fixed<32, 15>::from_raw(0x1000L); // 0.125
    auto b = Fixed<32, 20>::from_raw(0x20000L); // 0.125
    auto c = a + b;
    auto d = b + a;
    EXPECT_EQ(c, 0.25);
    EXPECT_EQ(d, 0.25);
}

TEST(Fixed, OperatorSub) {
    auto a = Fixed<32, 15>::from_raw(0x1000L); // 0.125
    auto b = Fixed<32, 20>::from_raw(0x40000L); // 0.25
    auto c = a - b; // -0.125
    auto d = b - a; // 0.125
    EXPECT_EQ(c, -0.125);
    EXPECT_EQ(d, 0.125);
}