/*
 * test_cellsockets.hpp
 *
 * Copyright (c) Thomas - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Thomas <tkchen@gmail.com>, Jan 18, 2016
 */

#ifndef TESTS_TEST_CELLSOCKETS_HPP_
#define TESTS_TEST_CELLSOCKETS_HPP_

#include <Python.h>
#include <Engine/all.hpp>
#include "gtest/gtest.h"
#include "boost/python.hpp"

namespace bp = boost::python;
using namespace Quantum;

TEST(CellSockets, TestMapping)
{
    CellSockets t;
    t.declare<bool>("bool", "booly an", true);

    cellsocket_ptr tp = t["bool"];
    EXPECT_TRUE(tp);
}

TEST(CellSockets, Cannot_redeclare)
{
    CellSockets t;
    t.declare<bool>("bool", "booly an", true);
    t.declare<bool>("b2");
    t.declare<std::string>("foo","A str", "bar");
    try
    {
        t.declare("bool",t["foo"]);
        ASSERT_FALSE(true);//should not reach here.
    }catch(std::exception& e)
    {
        EXPECT_TRUE(std::string(e.what()) ==
                "bool already exists. It's type is bool");
    }
    EXPECT_TRUE(t["bool"]->is_type<bool>());
    t.get<bool>("b2") = false;
    t.get<bool>("bool") = true;
    EXPECT_THROW(t.declare<bool>("b2") = t["bool"], std::runtime_error);
    t.get<bool>("b2") = false;
    EXPECT_FALSE(t.get<bool>("b2"));
    EXPECT_TRUE(t.get<bool>("bool"));
}

TEST(CellSockets, CopyValue)
{
    CellSockets t1,t2;
    t1.declare<bool>("a", "booly an", false);
    t1.declare<CellSocket::none>("any","a nony");
    t2.declare<bool>("b","a booly an", true);
    t1["a"] << t2["b"];
    t1["any"] << t2["b"];
    t1["any"] << t1["a"];
    EXPECT_TRUE(t1.get<bool>("any"));
}

TEST(CellSockets, DefaultCtor)
{
    CellSockets t;
    EXPECT_TRUE(t.size() == 0);

    cellsockets_ptr tp(new CellSockets());
    EXPECT_TRUE(tp->size() == 0);
}

//TEST(CellSockets, AssignementOperator)
//{
//  CellSockets t1, t2;
//  t1.declare<int>("x","x is an int",3);
//  t1 = t2; //cannot compile because noncopyable. good.
//  EXPECT_EQ(t2.get<int>("x"),3);
//  t2.get<int>("x") = 5;
//  EXPECT_EQ(t1.get<int>("x"),5);
//}

//test use of insert for copy
TEST(CellSockets, Copy)
{
    CellSockets t1, t2;
    t1.declare<int>("x", "x is an int", 3);
    t1.declare<double>("y", "y is a double", 2.05);

    //perform a shallow copy
    t2.insert(t1.begin(),t1.end());
    EXPECT_EQ(t2.get<int>("x"), 3);

    //The copies are the same object in memory
    t2.get<int>("x") = 5;
    EXPECT_EQ(t1.get<int>("x"), 5);

    //All elements are copied
    EXPECT_EQ(t2.get<double>("y"), 2.05);

    //The copies are the same object in memory
    t2.get<double>("y") = 3.05;
    EXPECT_EQ(t2.get<double>("y"), 3.05);
    EXPECT_EQ(t1.get<double>("y"), 3.05);
}

TEST(CellSockets, Declare)
{
    CellSockets t1;
    t1.declare<int>("x", "x is an int", 3);
    EXPECT_EQ(t1.get<int>("x"), 3);
    EXPECT_EQ(t1.size(), 1u);

    EXPECT_THROW(t1.declare<int>("x", "another declare", 11),
            std::runtime_error);

    //should still be valid after throw.
    EXPECT_EQ(t1.get<int>("x"), 3);

    //TODO: should you be able to do this?
    t1.clear();
    t1.declare<double>("x", "yet another declare", 17.5);
    EXPECT_EQ(t1.get<double>("x"), 17.5);
}

struct SpamFoo
{
    int blammo;
};

TEST(CellSockets, SyntacticSugarness)
{
    CellSockets t1, t2;
    t1.declare<int>("x", "x is an int", 3);
    t2.declare<std::string>("yy", "yy's doc");
    t2.declare<int>("y", "y is an int", 10);
    t1.declare<std::string>("s");
    //we can even declare our own type
    t2.declare<SpamFoo>("f", "SpamFoo is stuffs");
    std::string s("howdy");

    //insertion operator to put value in
    t1["s"] << s;
    EXPECT_EQ(t1.get<std::string>("s"), "howdy");

    //assignment operator can also put values in
    t1.get<std::string>("s") = "sally";

    //extraction operator to pull things out
    t1["s"] >> s;
    EXPECT_EQ(s, "sally");

    //insertion and extractions work on maps too
    s = "harry";
    t1["s"] << s;
    s = "";
    t2["yy"] << t1["s"];
    t2["yy"] >> s;
    EXPECT_EQ(s, "harry");

    //insertions work on CellSockets as well
    CellSocket x(std::string("foobar"), "docstr");
    t2["yy"] << x;
    //it only copies values, not doc text
    EXPECT_EQ(t2["yy"]->doc(), "yy's doc");
    EXPECT_EQ(t2.get<std::string>("yy"), "foobar");

    //we can create CellSockets inside smart pointers
    cellsocket_ptr xp = make_cellsocket<std::string>();
    //and the insertion still works on the pointer variable
    xp << std::string("hello there.");
    t2["yy"] << xp;
    EXPECT_EQ(t2.get<std::string>("yy"), "hello there.");
    //extraction works also
    t1["s"] >> xp;
    EXPECT_EQ(xp->get<std::string>(), "harry");

    //errors are thrown when trying to copy non-compatible types
    EXPECT_THROW(t1["s"] << t2["f"], std::runtime_error);
    EXPECT_THROW(t1["s"] >> t2["f"], std::runtime_error);
}
#endif /* TESTS_TEST_CELLSOCKETS_HPP_ */

