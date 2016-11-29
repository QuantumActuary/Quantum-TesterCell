/*
 * if.cpp
 *
 * Copyright (c) Thomas - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Thomas <tkchen@gmail.com>, Feb 6, 2016
 */

#include "TesterCell/tester.h"

namespace Quantum {
namespace TesterCell {

If::If():ret_msg_("")
{}

void If::declare_params(CellSockets &p)
{}

void If::declare_io(const CellSockets &p, CellSockets &i, CellSockets &o)
{
    i.declare<bool>("condition", "A true or false condition");
    i["condition"]->required(true);
    i["condition"]->str = [=, &i](){
        return std::to_string(i.get<bool>("condition"));
    };
    i.declare<ReturnCode>(">>", "Execution flow");
    i[">>"]->required(true);
    i[">>"]->str = [=, &i](){
        return std::to_string(static_cast<int>(i.get<ReturnCode>(">>")));
    };

    o.declare<ReturnCode>("true >>", "Execute if true");
    o["true >>"]->str = [=, &o](){
        return std::to_string(static_cast<int>(o.get<ReturnCode>("true >>")));
    };
    o.declare<ReturnCode>("false >>", "Else execute if false");
    o["false >>"]->str = [=, &o](){
        return std::to_string(static_cast<int>(o.get<ReturnCode>("false >>")));
    };
}
/*
void If::configure(const CellSockets &p, const CellSockets &i,
        const CellSockets &o)
{}*/

ReturnCode If::process(const CellSockets& i, const CellSockets& o)
{
    if(i.get<bool>("condition"))
    {
        o["true >>"] << Quantum::OK;
    }
    else
    {
        o["false >>"] << Quantum::OK;
    }
    return Quantum::OK;
}

Print::Print():ret_msg_(""){}

void Print::declare_params(CellSockets &p){}

void Print::declare_io(const CellSockets &p, CellSockets &i, CellSockets &o)
{
    i.declare<ReturnCode>(">>", "Put status here.", Quantum::UNKNOWN);
    i[">>"]->required(true);
    i[">>"]->str = [=, &i](){
        return std::to_string(static_cast<int>(i.get<ReturnCode>(">>")));
    };
    i.declare<std::string>("msg", "Put message here.", "Hello World!");
    i["msg"]->str = [=, &i](){
        return i.get<std::string>("msg");
    };

    o.declare<ReturnCode>(">>", "", Quantum::UNKNOWN);
    o[">>"]->str = [=, &o](){
        return std::to_string(static_cast<int>(o.get<ReturnCode>(">>")));
    };
}

ReturnCode Print::process(const CellSockets &i, const CellSockets &o)
{
    std::cout<<i.get<std::string>("msg")<<std::endl;
    o[">>"] << Quantum::OK;
    return Quantum::OK;
}

Start::Start():ret_msg_(""){}

void Start::declare_io(const CellSockets &p, CellSockets &i, CellSockets &o)
{
    o.declare<ReturnCode>(">>", "", Quantum::OK);
    o[">>"]->str = [=, &o](){
        return std::to_string(static_cast<int>(o.get<ReturnCode>(">>")));
    };
}

ReturnCode Start::process(const CellSockets &i, const CellSockets &o)
{
    o[">>"] << Quantum::OK;
    return Quantum::OK;
}

Hello::Hello(){}

void Hello::declare_io(const CellSockets &p, CellSockets &i, CellSockets &o)
{
    o.declare<std::string>("msg", "Say hello", "Hi there!");
    o["msg"]->str = [=, &o](){
        return o.get<std::string>("msg");
    };
}

ReturnCode Hello::process(const CellSockets &i, const CellSockets &o)
{
    o["msg"] << std::string("Helloooooo!");
    return Quantum::OK;
}

}//namespace TesterCell
}//namespace Quantum


