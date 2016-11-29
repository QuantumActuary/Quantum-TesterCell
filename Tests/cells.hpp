/*
 * cells.hpp
 *
 * Copyright (c) Thomas - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Thomas <tkchen@gmail.com>, Feb 10, 2016
 */

#ifndef TESTS_CELLS_HPP_
#define TESTS_CELLS_HPP_

#include "Engine/all.hpp"

#include <chrono>
#include <thread>


namespace Quantum
{

class TestObserver: public Observer
{
public:
    bool updated;
    TestObserver(Observable *o): Observer(o)
    {
        updated = false;
    }
    void update(Observable::Event e)
    {
        updated = true;
    }
};

struct Operation
{
    bool minus = false;

    static void declare_params(CellSockets &p)
    {
        p.declare<bool>("minus", "Subtraction", false);
    }

    static void declare_io(const CellSockets &p, CellSockets &i, CellSockets &o)
    {
        i.declare<int>("a", "An integer.");
        i.declare<int>("b", "An integer.", 0);
        o.declare<int>("ans", "Result of operation.", 0);

    }

    void configure(const CellSockets &p, const CellSockets &i, const CellSockets &o)
    {
        minus = p.get<bool>("minus");
    }

    ReturnCode process(const CellSockets &i, const CellSockets &o)
    {
        if(minus)
        {
            o["ans"] << i.get<int>("a") - i.get<int>("b");
        }
        else
        {
            o["ans"] << i.get<int>("a") + i.get<int>("b");
        }
        return OK;
    }
};

struct Pause
{
    static void declare_io(const CellSockets &p, CellSockets &i, CellSockets &o)
    {
        i.declare<int>("milliseconds", "Number of milliseconds to pause", 0);
        i.declare<bool>("link", "Connection to prior node", false);
        i["link"]->required(true);
        o.declare<bool>("done", "Connector to next node", false);
    }

    ReturnCode process(const CellSockets &i, const CellSockets &o)
    {
        SocketHandle<int> ms = i["milliseconds"];
        std::this_thread::sleep_for(std::chrono::milliseconds(*ms));
        o["done"] << true;
        return Quantum::OK;
    }
};

struct A
{
    static void
    declare_params(CellSockets& p)
    {
      p.declare<double> ("d","Doc",5.0);
      p.declare<std::string> ("x","Doc a string", "Hello");
    }
    static void declare_io(const CellSockets& p, CellSockets& i, CellSockets& o)
    {
        i.declare<double>("a", "An input.");
        i["a"]->required(true);
    }
};

struct B
{
    static void declare_io(const CellSockets& p, CellSockets& i, CellSockets& o)
    {
        i.declare<double>("a", "An input.");
        o.declare<double>("primitive", "A primitive assignment.");
        o.declare<double>("socket", "A value assigned from a socket.");
    }
    ReturnCode process(const CellSockets& i, const CellSockets& o)
    {
        o["primitive"]->get<double>() = 2.5;
        o["socket"]->get<double>() = i["a"]->get<double>();
        return Quantum::OK;
    }
};

struct C
{
    static void declare_io(const CellSockets& p, CellSockets& i, CellSockets& o)
    {
        i.declare<double>("a", "An input.");
        o.declare<double>("primitive", "A primitive insertion.");
        o.declare<double>("socket", "A socket insertion.");
        o.declare<double>("combo", "A calculated insertion.");
    }
    ReturnCode process(const CellSockets& i, const CellSockets& o)
    {
        o["primitive"] << 2.5;
        o["socket"] << i["a"];
        o["combo"] << i["a"]->get<double>() + 2.5;
        return Quantum::OK;
    }
};

struct ExceptionalModule1
{
  static void
  declare_params(CellSockets& p)
  {
    p.declare<double> ("d");
    p.declare<float> ("f").set_default_val(p.get<float> ("d"));
  }
};

struct ExceptionUnknownException
{
  static void
  declare_params(CellSockets& p)
  {
    p.declare<double> ("d");
  }
  static void
  declare_io(const CellSockets& p, CellSockets& in, CellSockets& out)
  {
    in.declare<double> ("d");
    throw std::string("A string");
  }
};

struct NotExist
{
    static void
    declare_params(CellSockets& p)
    {
        p.declare<int>("a");
    }
    static void
    declare_io(const CellSockets& p, CellSockets& in, CellSockets& out)
    {
        in.declare<double>("d");
        in.declare<ExceptionalModule1>("c");
        in.declare<std::string>("e");
        out.declare<std::string>("a");
    }

    int
    process(const CellSockets& in, const CellSockets& out)
    {
        in.get<double>("a");
        return 0;
    }
};

struct WrongType
{
    static void
    declare_io(const CellSockets& p, CellSockets& in, CellSockets& out)
    {
        in.declare<double>("d");
    }
    int
    process(const CellSockets& in, const CellSockets& out)
    {
        in.get<int>("d"); //FIXME: sends SIGABRT signal causing termination
        return 0;
    }
};

struct ParameterCBExcept
{
    static void
    declare_params(CellSockets& p)
    {
        p.declare<double> ("x");
    }
    void xcb(double x)
    {
        //std::cout << "*** about to throw std::runtime_error ***" << std::endl;
        throw std::runtime_error("I'm a bad callback, and I like it that way.");
    }
    void
    configure(const CellSockets& p,const CellSockets& in, const CellSockets& out)
    {
        //std::cout << "configurated ***" << std::endl;
        //xcb is triggered if p["x"] is dirty
        SocketHandle<double> x = p["x"];
        x.set_callback(boost::bind(&ParameterCBExcept::xcb,this,_1));
    }
};

struct InConstructorExcept
{
    InConstructorExcept()
    {
        throw std::runtime_error("no.... I do not want to live.");
    }
};

struct Sleeper
{
    static void declare_io(const CellSockets &p, CellSockets &i, CellSockets &o)
    {
        i.declare<int>("milliseconds", "Number of milliseconds to sleep.", 0);
    }
    ReturnCode process(const CellSockets &i, const CellSockets &o)
    {
        std::chrono::milliseconds timespan(i.get<int>("milliseconds"));
        std::this_thread::sleep_for(timespan);
        return OK;
    }
};

class NeverOutput
{
public:
    static void declare_params(CellSockets &p)
    {
        //p.declare<int>("ret", "ReturnCode", 0);
    }
    static void declare_io(const CellSockets &p, CellSockets &i, CellSockets &o)
    {
        i.declare<bool>("a", "Dont connect me");
        i["a"]->required(true);
        i["a"]->graph_supplied(true);
        i.declare<int>("ret", "ReturnCode", 0); //0=Quantum::OK

        o.declare<bool>("done", "Dont put anything here", false);
        o.declare<bool>("link", "Put something here to continue execution", false);
    }
    ReturnCode process(const CellSockets &i, const CellSockets &o)
    {
        o["link"] << true;
        std::cout << "No soup for you!" << std::endl;
        return ReturnCode(i["ret"]->get<int>());
    }
};

struct SocketHandleCellConst
{
    static void declare_params(CellSockets&p)
    {
        p.declare<std::string>("foo");
    }
    static void declare_io(const CellSockets& p, CellSockets& i, CellSockets& o)
    {
        i.declare<double>("d");
        o.declare<double>("out");
    }
    void configure(const CellSockets& p, const CellSockets& i, const CellSockets& o)
    {
        foo = p["foo"];
        d = i["d"];
        out = o["out"];
        //shouldn't compile.
        //i.declare<double>("d","a new d.");
    }

    int process(const CellSockets& i, const CellSockets& o)
    {
        *out = *d;
        //    out << d; //fail to compile
        //    o["out"] << d; //fail to compile
        o["out"] << *d;
        o["out"] << 3.4;
        o["out"] << i["d"];

        //    o["out"] = 2.0;//fail to compile.
        //    tendril_ptr tp = out.get();//fail to compile

        //out >> d; //should fail to compile
        //i["d"] >> out; //should fail at compile time.
        i["d"] >> *out; //should not fail.
        return Quantum::OK;
    }
    SocketHandle<double> d, out;
    SocketHandle<std::string> foo;
};

struct Add
{
    static void declare_io(const Quantum::CellSockets& p, Quantum::CellSockets& i, Quantum::CellSockets& o)
    {
        i.declare(&Add::left_,"left");
        i.declare(&Add::right_,"right");
        o.declare(&Add::out_,"out");
    }
    int process(const CellSockets& i, const CellSockets& o)
    {
        o["out"] << *left_ + *right_;
        return Quantum::OK;
    }
    Quantum::SocketHandle<double> out_, left_, right_;
};

struct PyTest
{
    static void declare_io(const Quantum::CellSockets& p, Quantum::CellSockets& i, Quantum::CellSockets& o)
    {
        i.declare(&PyTest::in_,"in");
        o.declare(&PyTest::out_,"out");
    }
    void configure(const CellSockets&, const CellSockets&, const CellSockets&)
    {
        (*in_)["a"] = 1;
        (*in_)["b"] = 2;
    }
    int process(const CellSockets& /*inputs*/, const CellSockets& /*outputs*/)
    {
        (*out_)["ans"] = (*in_)["a"] + (*in_)["b"];
        return Quantum::OK;
    }
    SocketHandle<bp::dict> in_, out_;
};

}//Quantum namespace




#endif /* TESTS_CELLS_HPP_ */
