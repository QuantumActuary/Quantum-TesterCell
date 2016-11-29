/*
 * Copyright (c) Thomas Chen - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Thomas Chen <tkchen@gmail.com>, Jan 19, 2016
 *
 * test_sockethandle.hpp
 */

#ifndef TESTS_TEST_SOCKETHANDLE_HPP_
#define TESTS_TEST_SOCKETHANDLE_HPP_

#include "Engine/all.hpp"
#include "gtest/gtest.h"
#include "cells.hpp"
using namespace Quantum;

TEST(SocketHandle, LifeTime)
{
    {
        SocketHandle<double> d = make_cellsocket<double>();
        EXPECT_TRUE(d);
    }
    {
        SocketHandle<double> d;
        EXPECT_FALSE(d);
        EXPECT_ANY_THROW(*d);

        d = make_cellsocket<double>();
        EXPECT_TRUE(d);
        *d = 3.555;
        EXPECT_EQ(*d, 3.555);
        // reassign
        d = make_cellsocket<double>();
        ASSERT_NE(*d, 3.555);

        EXPECT_ANY_THROW(d = make_cellsocket<std::string>());
    }
}

TEST(SocketHandle, NoDefault)
{
    cellsocket_ptr p = make_cellsocket<double>();
    SocketHandle<double> d = p; //p has to stay in scope...
    EXPECT_FALSE(d.user_supplied());
    EXPECT_FALSE(d.dirty());
    EXPECT_FALSE(d.has_default());

    *d = 3.14;
    EXPECT_FALSE(d.dirty());
    EXPECT_FALSE(d.user_supplied());
    EXPECT_FALSE(d.has_default());

    d.set_default_val(10);
    EXPECT_TRUE(d.has_default());
}

TEST(SocketHandle, Default)
{
    cellsocket_ptr p = make_cellsocket<double>();
    EXPECT_FALSE(p->dirty());

    SocketHandle<double> d = p; //p has to stay in scope...
    EXPECT_FALSE(d.dirty());

    d.set_default_val(1.41421356);

    EXPECT_FALSE(d.user_supplied());
    EXPECT_FALSE(d.dirty());
    EXPECT_TRUE(d.has_default());

    EXPECT_EQ(*d, 1.41421356);
    EXPECT_FALSE(d.dirty());
    d.notify();
    EXPECT_FALSE(d.dirty());

    *d = 3.14;
    EXPECT_FALSE(d.dirty());
    EXPECT_FALSE(d.user_supplied());
    EXPECT_TRUE(d.has_default());
}

template<typename T>
struct cbs
{
    cbs(): count(0), val(0) {}

    void operator()(const T& new_val)
    {
        val = new_val;
        count++;
    }

    int count;
    T val;
};

TEST(SocketHandle, BoostSignals)
{
    cellsocket_ptr p = make_cellsocket<double>();
    SocketHandle<double> d = p; //p has to stay in scope...
    d.set_default_val(1.41421356);

    cbs<double> c;
    d.set_callback(std::ref(c));
    d.notify();
    EXPECT_EQ(c.count, 0);
    EXPECT_EQ(c.val, 0);

    *d = 3.14;
    d.dirty(true); //dirtiness is explicit
    d.notify();
    EXPECT_EQ(c.count, 1);
    EXPECT_EQ(c.val, 3.14);

    *d = 5.55;
    EXPECT_FALSE(d.dirty());
    // callback didn't fire
    EXPECT_EQ(c.count, 1);
    EXPECT_EQ(c.val, 3.14);
    d.notify();

    // it didn't fire cause it aint dirty
    EXPECT_EQ(c.count, 1);
    EXPECT_EQ(c.val, 3.14);

    d.dirty(true);
    // still didn't fire... not notified
    EXPECT_EQ(c.count, 1);
    EXPECT_EQ(c.val, 3.14);

    // notifed, and was dirty... now it fires
    d.notify();
    EXPECT_EQ(c.count, 2);
    EXPECT_EQ(c.val, 5.55);
    EXPECT_FALSE(d.dirty());
}

TEST(SocketHandle, ObserverPattern)
{
    //observe a raw cellsocket
    CellSocket d;
    TestObserver e(&d);
    d.Observable::notify(Observable::DONE);
    EXPECT_TRUE(e.updated);

    //observe a shared pointer to a cellsocket
    cellsocket_ptr f = make_cellsocket<double>();
    TestObserver g(&(*f));
    f->Observable::notify(Observable::DONE);
    EXPECT_TRUE(g.updated);

    //observe a cellsocket handle
     SocketHandle<double> h(make_cellsocket<double>());
     TestObserver i(&h);
     h.Observable::notify(Observable::DONE);
     EXPECT_TRUE(i.updated);

}

TEST(SocketHandle, Expressions)
{
    cellsocket_ptr ta = make_cellsocket<double>();
    cellsocket_ptr tb = make_cellsocket<double>();
    cellsocket_ptr tc = make_cellsocket<double>();

    SocketHandle<double> a(ta), b(tb), c(tc);
    *a = 13.;
    EXPECT_EQ(*a, 13.);

    *b = 14.;
    EXPECT_EQ(*b, 14.);

    *c = 15.;
    EXPECT_EQ(*c, 15.);

    *a = (*b + *c);

    EXPECT_EQ(*a, 29.);
    EXPECT_EQ(*b, 14.);
    EXPECT_EQ(*c, 15.);
}



TEST(SocketHandle, Semantics)
{
  Cell::ptr c1(new Cell_<SocketHandleCellConst>);
  c1->declare_params();
  c1->declare_io();
  c1->configure();
  c1->process();
}

TEST(SocketHandle, DefaultConstruction)
{
    SocketHandle<double> d;
    //post conditions
    EXPECT_FALSE(d);
    EXPECT_TRUE(!d);
}

TEST(SocketHandle, CopyConstructionDefault)
{
    SocketHandle<double> d1;

    SocketHandle<double> d2(d1);
    //post condition
    EXPECT_FALSE(d2);

    SocketHandle<double> d3 = d1;
    //post condition
    EXPECT_FALSE(d3);

    d1 = make_cellsocket<double>();
    EXPECT_TRUE(d1);
    EXPECT_FALSE(d3);
    EXPECT_FALSE(d2);
}

TEST(SocketHandle, CopyConstructionValue)
{
    SocketHandle<double> d1( make_cellsocket<double>());

    SocketHandle<double> d2(d1);
    //post condition
    EXPECT_TRUE(d2);

    SocketHandle<double> d3 = d1;
    //post condition
    EXPECT_TRUE(d3);

    //all point to the same thing here.
    *d1 = 3.14;
    EXPECT_EQ(*d1,*d2);
    EXPECT_EQ(*d2,*d3);

    //assign a null spore to d2, no one else should be affected.
    d2 = SocketHandle<double>();
    EXPECT_TRUE(d1);
    EXPECT_TRUE(d3);
    EXPECT_FALSE(d2);
}

TEST(SocketHandle, ImplicitConstructor)
{
    cellsocket_ptr d = make_cellsocket<double>();
    cellsocket_ptr s = make_cellsocket<std::string>();
    SocketHandle<double> dd;
    SocketHandle<std::string> ss;
    dd = d;
    ss = s;
    EXPECT_THROW(dd = s, std::runtime_error);
    EXPECT_THROW(ss = d, std::runtime_error);
}

TEST(SocketHandle, NullAssign)
{
    SocketHandle<std::string> ss;
    EXPECT_THROW(ss = cellsocket_ptr(), std::runtime_error);
}

#endif /* TESTS_TEST_SOCKETHANDLE_HPP_ */
