/*
 * Copyright (c) Thomas Chen - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Thomas Chen <tkchen@gmail.com>, Jan 10, 2016
 *
 * test_behaviors.hpp
 */

#ifndef TESTS_TEST_BEHAVIORS_HPP_
#define TESTS_TEST_BEHAVIORS_HPP_

#include "Engine/all.hpp"
#include "Engine/observable.hpp"
#include "gtest/gtest.h"
#include "cells.hpp"

namespace Quantum
{

TEST(Behaviors, Declare_more_than_once)
{
    cell_ptr c = std::make_shared<Cell_<Operation>>();
    c->declare_params();
    c->declare_io();
    c->inputs["a"] << 1;
    c->inputs["b"] << 1;
    c->parameters["minus"] << false;
    c->process();
    EXPECT_EQ(2, c->outputs.get<int>("ans"));
    EXPECT_THROW(c->declare_params(), std::runtime_error);
    EXPECT_THROW(c->declare_io(), std::runtime_error);
}

TEST(Behaviors, Connect_incorrect_type)
{
    cell_ptr c = std::make_shared<Cell_<Operation>>();
    c->declare_params();
    c->declare_io();
    EXPECT_THROW(c->inputs["a"] << 1.5, std::runtime_error);
    c->inputs["b"] << 1;
    c->parameters["minus"] << false;
    c->process();
    EXPECT_EQ(1, c->outputs.get<int>("ans"));
}

TEST(Behaviors, Required_sockets)
{
    cell_ptr c = std::make_shared<Cell_<Operation>>();
    c->declare_params();
    c->declare_io();
    c->inputs["a"]->required(true);
    c->parameters["minus"] << false;

    //if a required socket does not have a value:
    EXPECT_THROW(c->process(), std::runtime_error);
    EXPECT_THROW(c->process(-1), std::runtime_error);
    EXPECT_THROW(c->process(0), std::runtime_error);
    EXPECT_THROW(c->process(1), std::runtime_error);
    EXPECT_THROW(c->process(12), std::runtime_error);

    //if a required socket has a constant value:
    c->inputs["a"] << 1;
    EXPECT_NO_THROW(c->process()); //note b has default value so it is "supplied"

    //The process id does not matter when the socket is constant
    EXPECT_NO_THROW(c->process(23));

    //However, output sockets will receive the process id as their new token id
    EXPECT_EQ(23, c->outputs["ans"]->token_id());

    //Even though socket b is not required, if connected to another cell, the
    //token id must match the process id before the cell can process
    circuit_ptr C(new Circuit);
    C->insert(c);
    cell_ptr c1 = c->clone();
    C->insert(c1);
    C->connect(c, "ans", c1, "b");
    c1->inputs["a"] << 1;
    Scheduler(C).execute(12);
    EXPECT_EQ(11, c1->inputs["b"]->token_id());
    //should not be able to process(0) because token_id is at 11
    EXPECT_EQ(Quantum::DO_OVER, c1->process(0));

    //It will only succeed if we process at the correct pid
    EXPECT_EQ(11, c1->inputs["b"]->token_id());
    EXPECT_EQ(Quantum::OK, c1->process(11));

    //Note that since c1 is "downstream" from c, we cannot process c1 until
    //we've received the proper token_id from c:
    EXPECT_EQ(Quantum::DO_OVER, c1->process(3));

    //It will only succeed if we process c before c1:
    Scheduler(C).execute(4);
    EXPECT_EQ(3, c1->inputs["b"]->token_id());
    EXPECT_EQ(Quantum::OK, c1->process(3));
}

TEST(Behaviors, Processing_with_tokens)
{
    cell_ptr c1 = std::make_shared<Cell_<Operation>>();
    c1->declare_params();
    c1->declare_io();
    SocketHandle<int> ans = c1->outputs["ans"];

    //there are no required inputs so this should succeed
    c1->inputs["a"] << 1;
    c1->process(0);
    EXPECT_EQ(*ans, 1);

    //The process id does not matter if all inputs are constant
    c1->inputs["a"]->required(true);
    EXPECT_EQ(c1->process(0), Quantum::OK);

    //if process id matches token id, process will execute
    c1->inputs["a"]->token_id(0);
    c1->process(0);
    EXPECT_EQ(*ans, 1);
    //outputs should inherit the process id as their new token id
    EXPECT_EQ(c1->outputs["ans"]->token_id(), 0);

    auto c2 = c1->clone();
    SocketHandle<int> ans2 = c2->outputs["ans"];
    EXPECT_FALSE(c2->inputs["a"]->required()); //because this is not statically defined
    //tokens do not get cloned
    EXPECT_EQ(c2->inputs["a"]->token_id(), -1);

}

TEST(Behaviors, Outputting_with_tokens)
{
    //This cell has a process function that uses assignment operator= on output
    cell_ptr c1 = std::make_shared<Cell_<B>>();
    //This cell has a process function that uses insertion operator<< on output
    cell_ptr c2 = std::make_shared<Cell_<C>>();
    c1->declare_io();
    c2->declare_io();

    c1->inputs["a"] << 1.5;
    c1->inputs["a"]->token_id(10);
    c2->inputs["a"] << 1.5;
    c2->inputs["a"]->token_id(10);

    c1->process();
    c2->process();

    EXPECT_EQ(-1, c1->outputs["primitive"]->token_id());
    EXPECT_EQ(-1, c1->outputs["socket"]->token_id());
    EXPECT_EQ(-1, c2->outputs["primitive"]->token_id());
    EXPECT_EQ(10, c2->outputs["socket"]->token_id());
    EXPECT_EQ(-1, c2->outputs["combo"]->token_id());
}

TEST(Behaviors, Scheduling_with_tokens)
{
    cell_ptr c1 = std::make_shared<Cell_<Operation>>();
    c1->declare_params();
    c1->declare_io();
    auto c2 = c1->clone();

    c1->inputs["a"] << 1;
    //since c1's input is primitive, we can process at any pid
    EXPECT_NO_THROW(c1->process(100));
    EXPECT_EQ(100, c1->outputs["ans"]->token_id());

    circuit_ptr C(new Circuit);
    C->insert(c1);
    C->insert(c2);
    C->connect(c1, "ans", c2, "a");
    C->configure_all();

    //when connected via cellsocket, c2's input becomes user_supplied
    EXPECT_TRUE(c2->inputs["a"]->graph_supplied());

    //by connecting, c1's current token id gets transferred.
    EXPECT_EQ(c2->inputs["a"]->token_id(), 100);

    c2->inputs["a"]->required(true);
    //the process id is relevant now because c2 is not connected to a primitive
    EXPECT_EQ(Quantum::DO_OVER, c2->process(1));

    c1->name("c1");
    c2->name("c2");
    Scheduler sched(C);
    c1->inputs["a"] << 3;
    //execute 10 iterations. This will step through pids 0-9
    EXPECT_NO_THROW(sched.execute(10));
    SocketHandle<int> ans(c2->outputs["ans"]);
    EXPECT_EQ(3, *ans);
    //the token_ids should be 9 since we ran process ids 0-9
    EXPECT_EQ(9, c2->outputs["ans"]->token_id());

}

TEST(Behaviors, Cells_are_observable)
{
    class CellObserver: public Observer
    {
    public:
        std::string status = "I have not seen...";
        CellObserver(Observable *o): Observer(o){}
        void update(Observable::Event e)
        {
            status = "Cannot unsee what has been seen!";
        }
    };

    cell_ptr c1 = std::make_shared<Cell_<Operation>>();
    c1->declare_params();
    c1->declare_io();
    CellObserver c2(&(*c1));

    c1->inputs["a"] << 1;
    c1->process();
    EXPECT_EQ("Cannot unsee what has been seen!", c2.status);
}

TEST(Behaviors, Smart_reprocessing)
{
    //Test that recalculation only happens when an input has changed.
    cell_ptr c = std::make_shared<Cell_<Operation>>();
    c->declare_params();
    c->declare_io();
    c->inputs["a"] << 1;
    c->inputs["b"] << 1;
    c->parameters["minus"] << false;
    c->process();
    EXPECT_EQ(2, c->outputs.get<int>("ans"));
    EXPECT_FALSE(c->needs_process());
    c->inputs["a"] << 2;
    EXPECT_TRUE(c->needs_process());
}

}//Quantum namespace

#endif /* TESTS_TEST_BEHAVIORS_HPP_ */
