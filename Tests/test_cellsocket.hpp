/*
 * test_cellsocket.hpp
 *
 * Copyright (c) Thomas - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Thomas <tkchen@gmail.com>, Jan 13, 2016
 */

#ifndef TESTS_TEST_CELLSOCKET_HPP_
#define TESTS_TEST_CELLSOCKET_HPP_

#include <Python.h>
#include <Engine/all.hpp>
#include "gtest/gtest.h"
#include "boost/python.hpp"

using namespace Quantum;

TEST(CellSocket, make_cellsocket)
{
    namespace bp = boost::python;
    Py_Initialize();
    cellsocket_ptr cp = make_cellsocket<bp::object>();
    EXPECT_TRUE(cp->get<bp::object>() == bp::object());
    cellsocket_ptr cp1(make_cellsocket<bp::object>());
    EXPECT_TRUE(cp1->get<bp::object>() == bp::object());
}

TEST(CellSocket, Dirtiness)
{
    CellSocket meh;
    EXPECT_FALSE(meh.dirty());

    CellSocket cs(0.5f, "docstring");

    EXPECT_EQ(cs.type_name(), "float");
    EXPECT_EQ(cs.doc(), "docstring");

    EXPECT_FALSE(cs.dirty());
    EXPECT_EQ(cs.get<float>(), 0.5f);
    EXPECT_FALSE(cs.dirty());
    cs << 0.75f;
    EXPECT_FALSE(cs.dirty());
    EXPECT_EQ(cs.get<float>(), 0.75f);

    cs.dirty(true);
    EXPECT_TRUE(cs.dirty());
    cs.notify();
    EXPECT_FALSE(cs.dirty());
}

void callonme(cellsocket_ptr cs)
{
   cs << 0.75f;
}

TEST(CellSocket, Callbacks)
{
    cellsocket_ptr cs = make_cellsocket<float>();
    cs << 0.5f;
    cs->set_callback<float>(
            [&](float f){callonme(cs);}
    );
    cs->dirty(true);
    cs->notify();
    float result = cs->get<float>();
    EXPECT_EQ(0.75f, result);
}

TEST(CellSocket, Value_updates)
{
    cellsocket_ptr cs = make_cellsocket<float>();
    bool updated = false;
    cs->updater(
            [&](){updated=true;});
    EXPECT_FALSE(updated);
    cs << 0.5f; //this changes the value so an update notification will fire
    EXPECT_TRUE(updated);
    updated = false;
    cs << 0.5f; //no update since the value stays the same!
    EXPECT_FALSE(updated);
}

TEST(CellSocket, Constructors)
{
    {//default constructor
        CellSocket meh;
        EXPECT_FALSE(meh.dirty());
        EXPECT_FALSE(meh.graph_supplied());
        EXPECT_FALSE(meh.has_default());
        EXPECT_FALSE(meh.supplied());
        EXPECT_TRUE(meh.is_type<CellSocket::none>());
    }
    {//parameterized construction
        CellSocket meh(0.5f, "docstring");
        EXPECT_FALSE(meh.dirty());
        EXPECT_FALSE(meh.graph_supplied());
        EXPECT_TRUE(meh.has_default());
        EXPECT_TRUE(meh.supplied());
        EXPECT_TRUE(meh.is_type<float>());

        meh << 2.0f;

        EXPECT_TRUE(meh.has_default());
        EXPECT_TRUE(meh.is_type<float>());
        EXPECT_FALSE(meh.graph_supplied());
        EXPECT_TRUE(meh.supplied());
        EXPECT_TRUE(meh.get<float>()==2.0f);

        meh.graph_supplied(true);
        meh.dirty(true);
        //user_supplied and dirty are explicit states.
        EXPECT_TRUE(meh.graph_supplied());
        EXPECT_TRUE(meh.dirty());

        meh.notify();
        EXPECT_TRUE(meh.graph_supplied());
        EXPECT_FALSE(meh.dirty());
    }
    {//construction through pointer
        cellsocket_ptr meh = make_cellsocket<float>();
        EXPECT_FALSE(meh->dirty());
        EXPECT_FALSE(meh->graph_supplied());
        EXPECT_FALSE(meh->has_default());
        EXPECT_FALSE(meh->supplied());
        meh << 2.0f;
        EXPECT_TRUE(meh->get<float>()==2.0f);
        EXPECT_FALSE(meh->has_default());
        EXPECT_FALSE(meh->graph_supplied()); //constants aren't user-supplied
        EXPECT_TRUE(meh->supplied()); //they're simply supplied
        EXPECT_FALSE(meh->dirty());
    }
}

TEST(CellSocket, We_are_not_pointers)
{
    CellSocket a(0.5f, "A float"), b, c;
    b = a;
    c = a;
    c << 3.14f;
    EXPECT_NE(a.get<float>(), c.get<float>());
    EXPECT_EQ(a.get<float>(), b.get<float>());
    EXPECT_NE(&a.get<float>(), &c.get<float>());
    EXPECT_NE(&a.get<float>(), &b.get<float>());
}

TEST(CellSocket, Copyness)
{
    CellSocket a(0.5f, "A float"), b, c;
    b << a;
    c << b;
    c << 3.14f;
    EXPECT_NE(a.get<float>(), c.get<float>());
    EXPECT_EQ(a.get<float>(), b.get<float>());
    //memory addresses of these values are different
    EXPECT_NE(&a.get<float>(), &c.get<float>());
    EXPECT_NE(&a.get<float>(), &b.get<float>());

    c << a;
    EXPECT_EQ(a.get<float>(), c.get<float>());
    //a and c are not the same object in memory
    EXPECT_NE(&a.get<float>(), &c.get<float>());
}

TEST(CellSocket, Token_transfer)
{
    CellSocket a(0.5f, "A float"), b, c;
    a.token_id(1);
    b << a;
    c << b;
    EXPECT_EQ(a.token_id(), c.token_id());
    c << 3.14f;
    EXPECT_NE(a.token_id(), c.token_id());
    EXPECT_EQ(a.token_id(), b.token_id());
    b = c;
    EXPECT_EQ(b.token_id(), c.token_id());
    c.token_id(2);
    EXPECT_NE(b.token_id(), c.token_id());
    c << a;
    EXPECT_EQ(a.token_id(), c.token_id());
}

TEST(CellSocket, Token_resetting)
{
    CellSocket cs(1, "I'm just a cellsocket.");
    EXPECT_EQ(1, cs.get<int>());
    //insertion of primitive causes reset
    cs << 2;
    EXPECT_EQ(2, cs.get<int>());
    EXPECT_EQ(-1, cs.token_id()); //has been reset to -1
    cs.token_id(10);
    //modifying value through operator= does not reset token_id
    cs.get<int>() = 3;
    EXPECT_EQ(3, cs.get<int>());
    EXPECT_EQ(10, cs.token_id()); //remains at 10
}

TEST(CellSocket, Internal_use)
{
    CellSocket cs(1, "I'm just a cellsocket.");
    cs.internal_use(false);
    EXPECT_FALSE(cs.internal_use());
    cs.internal_use(true);
    EXPECT_TRUE(cs.internal_use());
}

TEST(CellSocket, Typeness)
{
    CellSocket a(0.5f, "A float"), b(0.5, "A double."), c;
    EXPECT_THROW(b << a, std::runtime_error);
    EXPECT_NO_THROW(c = a);
    EXPECT_THROW(c << b, std::runtime_error);
    EXPECT_NO_THROW(c = b);
    EXPECT_THROW(c << a, std::runtime_error);
    CellSocket n1(CellSocket::none(),"A none"), n2(CellSocket::none(),"Another none"), n3;
    n2 << n1;
    n1 << n2;
    n1 = n2;
    n2 = n1;
    n1 << b;
    n2 << n1;
    EXPECT_EQ(n2.get<double>(),0.5);
    EXPECT_THROW(n2 << a, std::runtime_error);
    //EXPECT_THROW(b.copy_value(n3), except::ValueNone);
}

TEST(CellSocket, AssignmentOfPODS)
{
    CellSocket a(0.005f, "A float");
    EXPECT_TRUE(a.is_type<float>());
    EXPECT_EQ(a.get<float>(), 0.005f);

    CellSocket b(500.0, "some double");
    EXPECT_TRUE(b.is_type<double>());
    EXPECT_EQ(b.get<double>(), 500.0);

    CellSocket c;
    EXPECT_TRUE(c.is_type<CellSocket::none>());

    // nothing is mutated on throwing conversion
    EXPECT_THROW(b << a, std::runtime_error);
    EXPECT_TRUE(a.is_type<float>());
    EXPECT_EQ(a.get<float>(), 0.005f);
    EXPECT_TRUE(b.is_type<double>());
    EXPECT_EQ(b.get<double>(), 500.0);

    // assignee takes on the type and value of the assigned
    EXPECT_NO_THROW(c = a);
    EXPECT_TRUE(a.is_type<float>());
    EXPECT_EQ(a.get<float>(), 0.005f);
    EXPECT_TRUE(c.is_type<float>());
    EXPECT_EQ(c.get<float>(), 0.005f);

    // again nothing is mutated on throwing conversion
    EXPECT_THROW(c << b, std::runtime_error);
    EXPECT_TRUE(b.is_type<double>());
    EXPECT_EQ(b.get<double>(), 500.0);
    EXPECT_TRUE(c.is_type<float>());
    EXPECT_EQ(c.get<float>(), 0.005f);

    // again assignment succeeds
    EXPECT_NO_THROW(c = b);
    EXPECT_TRUE(b.is_type<double>());
    EXPECT_EQ(b.get<double>(), 500.0);
    EXPECT_TRUE(c.is_type<double>());
    EXPECT_EQ(c.get<double>(), 500.0);

    // same as above
    EXPECT_THROW(c << a, std::runtime_error);
}

TEST(CellSocket, AssignmentOfNone)
{
    CellSocket n1(CellSocket::none(),"A none");
    CellSocket n2(CellSocket::none(),"Another none");

    CellSocket n3;
    n2 << n1;
    EXPECT_EQ(n1.get<CellSocket::none>(), n2.get<CellSocket::none>());
    n1 << n2;
    EXPECT_EQ(n1.get<CellSocket::none>(), n2.get<CellSocket::none>());

    n1 = n2;
    n2 = n1;

    CellSocket _500(500.0, "five hundred");

    n1 << _500;
    EXPECT_TRUE(n1.is_type<double>());
    EXPECT_TRUE(_500.is_type<double>());
    EXPECT_EQ(n1.get<double>(), 500.0);
    EXPECT_EQ(_500.get<double>(), 500.0);
}

TEST(CellSocket, Boost_python_auto_convert)
{
    namespace bp = boost::python;
    CellSocket bpt(bp::object(2.05), "A bp object");
    CellSocket dt(7.05, "A double");

    // you can't do this... first you get out the bp::object,
    // then you get the double out of it.
    EXPECT_THROW(bpt.get<double>(), std::runtime_error);

    // CellSocket(bp::object) autoconverts to CellSocket(nonobject)
    dt << bpt;
    EXPECT_EQ(2.05, dt.get<double>());

    // can be overwritten thereafter
    dt << 7.05;
    EXPECT_EQ(dt.get<double>(), 7.05);

    // cannot auto-convert from non-compatible objects
    bp::list l;
    l.append(3.05);
    bp::object o = l;
    CellSocket bpl(o, "A bp list");
    EXPECT_THROW(dt << bpl, std::runtime_error);

    // copyee is not mutated
    double value = bp::extract<double>(bpt.get<bp::object>());
    EXPECT_EQ(value, 2.05);

    // dt has not lost its internal type
    EXPECT_THROW(dt << std::string("NOTADOUBLE"), std::runtime_error);

    // but we can't autoconvert from Python's None type
    bpt << bp::object();
    EXPECT_THROW(dt << bpt, std::runtime_error);
}

TEST(CellSocket, Raw_python_auto_converts_on_input)
{
    namespace bp = boost::python;
    {//double
        bp::object bpo(2.05);
        CellSocket bpc;
        //auto convert from a raw PyObject* to boost::python
        bpc << bpo.ptr();
        EXPECT_TRUE(bpc.is_type<bp::object>());
        double d = bp::extract<double>(bpc.get<bp::object>());
        EXPECT_EQ(d, 2.05);
    }
    {//int
        bp::object bpo(2);
        CellSocket bpc;
        //auto convert from a raw PyObject* to boost::python
        bpc << bpo.ptr();
        EXPECT_TRUE(bpc.is_type<bp::object>());
        int i = bp::extract<int>(bpc.get<bp::object>());
        EXPECT_EQ(i, 2);
    }
    {//bool
        bp::object bpo(true);
        CellSocket bpc;
        //auto convert from a raw PyObject* to boost::python
        bpc << bpo.ptr();
        EXPECT_TRUE(bpc.is_type<bp::object>());
        bool b = bp::extract<bool>(bpc.get<bp::object>());
        EXPECT_TRUE(b);
    }
    {//string
        bp::object bpo("test");
        CellSocket bpc;
        //auto convert from a raw PyObject* to boost::python
        bpc << bpo.ptr();
        EXPECT_TRUE(bpc.is_type<bp::object>());
        std::string s = bp::extract<std::string>(bpc.get<bp::object>());
        EXPECT_EQ(s, "test");
    }
}

/*//FIXME: Works, but don't know how to write the test
TEST(CellSocket, Raw_python_auto_converts_on_output)
{
    //auto converts back to raw PyObject*
    CellSocket bpc(bp::object(2.05), "A boost python socket.");
    bp::list bpl;
    bpl.append(1.05);
    PyObject *po = bpl.ptr();

    //EXPECT_NO_THROW(bpc >> po);
    bpc >> po;
    EXPECT_EQ(PyFloat_AsDouble(PyList_GetItem(po, 0)), 2.05);
}*/

TEST(CellSocket, POD2PythonConversion)
{
    namespace bp = boost::python;
    CellSocket bpt(bp::object(2.05), "A bp object");
    CellSocket dt(7.05, "A double");

    {
        bp::extract<double> extractor(bpt.get<bp::object>());
        EXPECT_EQ(extractor(), 2.05);
    }
    EXPECT_TRUE(bpt.is_type<bp::object>());
    bpt << dt;

    // bpt is still a bp::object
    EXPECT_TRUE(bpt.is_type<bp::object>());

    // dt is still a double
    EXPECT_TRUE(dt.is_type<double>());

    {// double was copied correctly into dt
        bp::extract<double> extractor(bpt.get<bp::object>());
        EXPECT_EQ(extractor(), 7.05);
    }
    // dt was not mutated
    EXPECT_EQ(dt.get<double>(), 7.05);
}

TEST(CellSocket, BoostPyDefaultness)
{
    namespace bp = boost::python;
    CellSockets ts;
    ts.declare<bp::object>("x","A bp object");
    bp::object x;
    ts["x"] >> x;
    EXPECT_EQ(x, bp::object()); //the None type
}

TEST(CellSocket, SyntacticSugar)
{
    int x = 2;
    float y = 3.14;
    std::string z = "z";

    CellSocket tx(x,"doc"),ty(y,"doc"),tz(z,"doc");
    tz >> z;
    tz << z;
    ty >> y;
    ty << y;
    tx << x;
    tx >> x;
    EXPECT_THROW(tx >> y;, std::runtime_error);
    EXPECT_THROW(tx << z;, std::runtime_error);

    CellSockets ts;
    ts.declare<int>("x");
    ts.declare<float>("y");
    ts.declare<std::string>("z");
    ts["x"] >> x;
    ts["x"] << x;
    ts["y"] >> y;
    ts["y"] << y;
    ts["z"] >> z;
    ts["z"] << z;
    EXPECT_THROW(ts["z"] >> y;, std::runtime_error);
    EXPECT_THROW(ts["z"] << x;, std::runtime_error);

    EXPECT_THROW(ts["w"] >> x;, std::runtime_error);
    EXPECT_THROW(ts["t"] << x;, std::runtime_error);
}

TEST(CellSocket, Nones)
{
    namespace bp = boost::python;
    cellsocket_ptr a = make_cellsocket<CellSocket::none>();
    cellsocket_ptr b = make_cellsocket<CellSocket::none>();
    EXPECT_TRUE(a->is_type<CellSocket::none>());
    EXPECT_TRUE(a->same_type(*b));
    EXPECT_TRUE(b->same_type(*a));
    a << b;
    EXPECT_TRUE(a->is_type<CellSocket::none>());
    EXPECT_TRUE(a->same_type(*b));
    EXPECT_TRUE(b->same_type(*a));
    a >> b;
    EXPECT_TRUE(a->same_type(*b));
    EXPECT_TRUE(b->same_type(*a));

    // you can assign anything to a none CellSocket, it changes type
    a << 7.05;
    EXPECT_TRUE(a->is_type<double>());
    EXPECT_EQ(a->get<double>(), 7.05);

    // note: now a is a double, you can't assign a string to it
    std::string s("ess");
    EXPECT_THROW(a << s, std::runtime_error);

    // assignment makes it a vanilla none again
    a = b;
    EXPECT_TRUE(a->is_type<CellSocket::none>());
    EXPECT_TRUE(a->same_type(*b));
    EXPECT_TRUE(b->same_type(*a));

    // bp object with a string in it
    bp::object obj(s);

    a << obj;
    EXPECT_TRUE(a->is_type<bp::object>());
}

TEST(CellSocket, ConversionTableFromNoneColumn)
{
    namespace bp = boost::python;
    CellSocket none_;

    { // none << none
        CellSocket othernone_;
        EXPECT_NO_THROW(othernone_ << none_);
    }

    { // object << none
        CellSocket object_(bp::object(3.14159), "pyobj");
        EXPECT_THROW(object_ << none_, std::runtime_error);
    }

    { // double << none
        CellSocket double_(3.14159, "double");
        EXPECT_THROW(double_ << none_, std::runtime_error);
    }
}

TEST(CellSocket, ConversionTableFromPyObjectColumn)
{
    namespace bp = boost::python;
    CellSocket pypi_(bp::object(3.1415), "py pi");

    { // none << object
        CellSocket none_;
        none_ << pypi_;
        bp::object rt = none_.get<bp::object>();
        bp::extract<double> extractor(rt);
        EXPECT_EQ(extractor(), 3.1415);
    }

    { // object << object
        CellSocket o2(bp::object(7.777), "sevens");
        o2 << pypi_;
        bp::object rt = o2.get<bp::object>();
        bp::extract<double> extractor(rt);
        EXPECT_EQ(extractor(), 3.1415);
    }

    { // double << object (compatible)
        CellSocket double_(5.555, "double");
        double_ << pypi_;
        EXPECT_EQ(double_.get<double>(), 3.1415);
    }

    { // string << object (incompatible)
        CellSocket string_(std::string("oops"), "double");
        EXPECT_THROW(string_ << pypi_, std::runtime_error);
    }
}

TEST(CellSocket, ConversionTableFromUDTColumn)
{
    namespace bp = boost::python;
    CellSocket udt_(std::string("STRINGY"), "py pi");

    { // none << udt
        CellSocket none_;
        none_ << udt_;
        std::string s = none_.get<std::string>();
        EXPECT_EQ(s, "STRINGY");
    }

    { // object << udt
        CellSocket o2(bp::object(7.777), "sevens");
        o2 << udt_;
        bp::object rt = o2.get<bp::object>();
        std::string xtracted = bp::extract<std::string>(rt);
        EXPECT_EQ(xtracted, std::string("STRINGY"));
    }

    { // string << udt (compatible)
        CellSocket string_(std::string("NOTSTRINGY"), "is other string");
        string_ << udt_;
        EXPECT_EQ(string_.get<std::string>(), std::string("STRINGY"));
        // not the same string
        EXPECT_NE(&(string_.get<std::string>()), &(udt_.get<std::string>()));
    }

    { // double << udt (incompatible)
        CellSocket double_(3.1415, "double");
        EXPECT_THROW(double_ << udt_, std::runtime_error);
    }
}

TEST(CellSocket, ConvertersCopied)
{
    namespace bp = boost::python;
    cellsocket_ptr a = make_cellsocket<CellSocket::none>();
    cellsocket_ptr b = make_cellsocket<double>();
    EXPECT_FALSE(a->same_type(*b));
    EXPECT_FALSE(b->same_type(*a));
    *a = *b;
    EXPECT_TRUE(a->same_type(*b));
    EXPECT_TRUE(b->same_type(*a));
    bp::object obj(3.1415);
    *a << obj;
    EXPECT_EQ(a->get<double>(), 3.1415);
    bp::object obj2;
    *a >> obj2;
    bp::extract<double> e(obj2);
    EXPECT_TRUE(e.check());
    EXPECT_EQ(e(), 3.1415);
}

TEST(CellSocket, Nullptr)
{
    cellsocket_ptr a, b;
    EXPECT_THROW(a << b, std::runtime_error);
}

TEST(CellSocket, Signal_emitter)
{
    cellsocket_ptr a = make_cellsocket<int>();
    std::function<void(void)> emit = [](){
        std::cout<<"Emitting signal!"<<std::endl;
    };
    a->updater(emit);
    a << 1; //should emit signal

}

namespace testy{
struct FooBar
{
    int x;
};
}//namespace testy

TEST(CellSocket, TypeReg)
{
    //This registers the CellSocket type
    Quantum::make_cellsocket<testy::FooBar>();
    //Make CellSockets from the registry
    CellSocket
        a = Quantum::Registry::CellSocket::get("int"),
        b = Quantum::Registry::CellSocket::get("testy::FooBar"),
        c = Quantum::Registry::CellSocket::get("std::string");
    //Make CellSockets from arbitrary objects
    CellSocket ta(int(4),""), tb(testy::FooBar(), ""), tc(std::string(""),"");
    EXPECT_TRUE(ta.same_type(a));
    EXPECT_TRUE(tb.same_type(b));
    EXPECT_TRUE(tc.same_type(c));
}


#endif /* TESTS_TEST_CELLSOCKET_HPP_ */
