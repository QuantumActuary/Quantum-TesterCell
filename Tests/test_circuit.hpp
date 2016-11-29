/*
 * test_circuit.hpp
 *
 * Copyright (c) Thomas - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Thomas <tkchen@gmail.com>, Feb 11, 2016
 */

#ifndef TESTS_TEST_CIRCUIT_HPP_
#define TESTS_TEST_CIRCUIT_HPP_

#include "Engine/all.hpp"
#include "Engine/kernel.h"
#include "cells.hpp"
#include "gtest/gtest.h"

namespace Quantum
{

class CircuitsTest: public ::testing::Test
{
public:
    Kernel* theKernel;
    std::string root_dir;
    CircuitsTest()
    {
        theKernel = Kernel::getKernel();
        root_dir = "../../";
        theKernel->setRootDirectory("/Users/Thomas/Quantum/QuantumGUI/src");
        theKernel->loadPlugin("libTesterCell.dylib");
    }
};

TEST_F(CircuitsTest, Disconnect)
{
    cell_ptr a = theKernel->getCellRegistry().getCell("Quantum::TesterCell::Hello::Hello");
    cell_ptr b = theKernel->getCellRegistry().getCell("Quantum::TesterCell::Print::Print");
    cell_ptr c = theKernel->getCellRegistry().getCell("Quantum::TesterCell::Start::Start");

    circuit_ptr d(new Circuit());
    d->insert(a);
    d->insert(b);
    d->insert(c);
    d->connect(a, "msg", b, "msg");
    d->connect(c, ">>", b, ">>");
    Scheduler(d).execute(1);
    EXPECT_EQ(Quantum::OK, b->outputs.get<ReturnCode>(">>"));
    EXPECT_NO_THROW(d->disconnect(a, "msg", b, "msg"));

    //disconnect with one-to-many
    cell_ptr e = theKernel->getCellRegistry().getCell("Quantum::TesterCell::Print::Print");
    d->connect(a, "msg", b, "msg");
    d->connect(a, "msg", e, "msg");
    Scheduler(d).execute(1);
    EXPECT_EQ(Quantum::OK, e->outputs.get<ReturnCode>(">>"));
    EXPECT_NO_THROW(d->disconnect(a, "msg", b, "msg"));
    EXPECT_NO_THROW(d->disconnect(a, "msg", e, "msg"));

}

TEST_F(CircuitsTest, Observe_a_socket)
{
    cell_ptr a = theKernel->getCellRegistry().getCell("Quantum::TesterCell::Hello::Hello");
    cell_ptr b = theKernel->getCellRegistry().getCell("Quantum::TesterCell::Print::Print");

    circuit_ptr d(new Circuit());
    d->insert(a);
    d->insert(b);


    //Observe using a sockethandle
    SocketHandle<std::string> h(b->inputs["msg"]);
    TestObserver e(&h);
    h.Observable::notify(Observable::DONE);
    EXPECT_TRUE(e.updated);

    //Observe using the cellsocket
    TestObserver f(&*(b->inputs["msg"]));
    b->inputs["msg"]->Observable::notify(Observable::DONE);
    EXPECT_TRUE(f.updated);

    //Test that connection event triggers observation
    TestObserver g(&*(b->inputs["msg"]));
    d->connect(a, "msg", b, "msg");
    EXPECT_TRUE(g.updated);

    //Test that disconnect also triggers the observation
    g.updated = false;
    EXPECT_FALSE(g.updated);
    d->disconnect(a, "msg", b, "msg");
    EXPECT_TRUE(g.updated);

}
} //Quantum namespace

#endif /* TESTS_TEST_CIRCUIT_HPP_ */
