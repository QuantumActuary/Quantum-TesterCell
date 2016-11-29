/*
 * test_static.hpp
 *
 * Copyright (c) Thomas - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Thomas <tkchen@gmail.com>, Jan 19, 2016
 */

#ifndef TESTS_TEST_STATIC_HPP_
#define TESTS_TEST_STATIC_HPP_

#include "Engine/all.hpp"
#include "cells.hpp"
#include "gtest/gtest.h"

namespace Quantum
{
TEST(Static, DoesItWork)
{
    cell_ptr a = Cell::ptr(new Cell_<Add>());
    a->declare_params();
    a->declare_io();
    a->inputs["left"] << 2.0;
    a->inputs["right"] << 5.0;
    a->process();
    EXPECT_EQ(a->outputs.get<double>("out"),7.0);
}

TEST(Static, Works_with_python)
{
    cell_ptr b(new Cell_<PyTest>);
    b->declare_params();
    b->declare_io();
    EXPECT_NO_THROW(b->process());
    bp::dict temp = b->outputs["out"]->get<bp::dict>();
    EXPECT_EQ(bp::extract<int>(temp["ans"]), 3);
}
} //Quantum namespace

#endif /* TESTS_TEST_STATIC_HPP_ */
