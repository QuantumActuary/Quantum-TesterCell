/*
 * Copyright (c) Thomas Chen - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Thomas Chen <tkchen@gmail.com>, Jan 10, 2016
 *
 * test_profiling.hpp
 */

#ifndef TESTS_TEST_PROFILING_HPP_
#define TESTS_TEST_PROFILING_HPP_

#include "Engine/all.hpp"
#include "cells.hpp"
#include "gtest/gtest.h"

#include <chrono>

namespace Quantum
{

TEST(Profiling, timeit)
{
    cell_ptr c = std::make_shared<Cell_<Sleeper>>();
    c->declare_params();
    c->declare_io();
    c->configure();

    c->profile[Cell::T_PROCESS] = true;
    c->inputs["milliseconds"] << 500;
    c->process();
    EXPECT_NEAR(500000, c->us[Cell::T_PROCESS].count(), 10000);

    auto d = c->clone();
    d->configure();
    //cloning does not copy the profile settings.
    EXPECT_FALSE(d->profile[Cell::T_PROCESS]);
    d->inputs["milliseconds"] << 650;
    d->process();
    EXPECT_NEAR(0, d->us[Cell::T_PROCESS].count(), 5000);

    /* Doesn't work, parameters can't be reset once configured
    e->parameters["timeit"] << true;
    e->process();
    EXPECT_NEAR(650, e->us.count(), 10);
    */
}
}

#endif /* TESTS_TEST_PROFILING_HPP_ */
