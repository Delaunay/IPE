#ifndef PROJECT_TEST_TESTS_ADD_HEADER
#define PROJECT_TEST_TESTS_ADD_HEADER

#include "ipe.h"
#include <gtest/gtest.h>

using namespace ipe;

// TEST(add, eval)
// {
//     auto f = add(make_val(21), make_val(32));
//     Context ctx;

//     EXPECT_DOUBLE_EQ(21 + 32, f->full_eval(ctx));
// }

// TEST(add, derivate)
// {
//     auto x = make_var("x");

//     auto f = add(x, make_val(32));
//     Context ctx;

//     auto deriv = f->derivate("x");

//     EXPECT_DOUBLE_EQ(1, deriv->full_eval(ctx));
// }

#endif
