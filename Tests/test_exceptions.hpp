//
// Copyright (c) 2011, Willow Garage, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Willow Garage, Inc. nor the names of its
//       contributors may be used to endorse or promote products derived from
//       this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//

#include "Engine/all.hpp"
#include "Engine/circuit.hpp"
#include "cells.hpp"
#include "gtest/gtest.h"
#include "Engine/omp_scheduler.hpp"

using namespace Quantum;



TEST(Exceptions, ExceptionalModules)
{
    Cell* p = new Cell_<ExceptionalModule1>;
    EXPECT_THROW(p->declare_params(), std::exception);
}

TEST(Exceptions, ExceptionUnknownException)
{
    Cell* c = new Cell_<ExceptionUnknownException>;
    c->declare_params();
    EXPECT_THROW(c->declare_io(), std::string);
}

TEST(Exceptions, NotExist)
{
    Cell::ptr m(new Cell_<NotExist>);
    EXPECT_THROW(m->process(), std::exception);
}

TEST(Exceptions, WrongType)
{
    Cell::ptr m(new Cell_<WrongType>);
    m->declare_params();
    m->declare_io();
    m->configure();
    EXPECT_THROW(m->process(), std::exception);
}
/* FIXME
TEST(Exceptions, WrongType_sched)
{
    Cell::ptr m(new Cell_<WrongType>);
    m->declare_params();
    m->declare_io();
    m->configure();
    circuit_ptr p(new Circuit);
    p->insert(m);
    Scheduler sched(p);
    EXPECT_THROW(sched.execute(2), std::runtime_error); //terminates app!
}
*/
/*FIXME
TEST(Exceptions, ParameterCBExcept_sched)
{
    Cell::ptr m(new Cell_<ParameterCBExcept>);
    m->declare_params();
    m->declare_io();
    m->parameters["x"] << 5.1;
    m->parameters["x"]->dirty(true);
    circuit_ptr p(new Circuit);
    p->insert(m);
    Scheduler sched(p);
    bool threw = false;
    try
    {
        sched.execute(8);
        FAIL();
    }
    catch (const std::exception &e)
    {
        threw = true;
        //std::cout << "Good, threw an exception:\n" << e.what() << std::endl;
    }
    EXPECT_TRUE(threw);
}
*/
TEST(Exceptions, ConstructorExcept)
{
    try
    {
        cell_ptr m = cell_ptr(new Cell_<InConstructorExcept>()); //note: no construction actually happens yet
        m->declare_params();
        m->declare_io();
        circuit_ptr p = circuit_ptr(new Circuit);
        p->insert(m);
        Scheduler sched(p);
        sched.execute(8);
        FAIL();//no exception thrown, should not reach here...
    }
    catch (const std::exception &e)
    {
        EXPECT_EQ(std::string("no.... I do not want to live."), e.what()); //fails due to segfault.
    }
}
