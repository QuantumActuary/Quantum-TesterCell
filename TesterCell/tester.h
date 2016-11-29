/*
 * if.h
 *
 * Copyright (c) Thomas - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Thomas <tkchen@gmail.com>, Feb 6, 2016
 */

#ifndef TESTERCELL_TESTER_H_
#define TESTERCELL_TESTER_H_

#include "Engine/kernel.h"
#include "testercell_config.h"

namespace Quantum {
namespace TesterCell {

class TESTERCELL_API If
{
public:
    If();
    static void declare_params(CellSockets&);
    static void declare_io(const CellSockets&, CellSockets&, CellSockets&);
    //void configure(const CellSockets&, const CellSockets&, const CellSockets&);
    ReturnCode process(const CellSockets&, const CellSockets&);
    const std::string return_msg(){return ret_msg_;}
private:
    std::string ret_msg_;
};

class TESTERCELL_API Print
{
public:
    Print();
    static void declare_params(CellSockets&);
    static void declare_io(const CellSockets&, CellSockets&, CellSockets&);
    //void configure(const CellSockets&, const CellSockets&, const CellSockets&);
    ReturnCode process(const CellSockets&, const CellSockets&);
    const std::string return_msg(){return ret_msg_;}
private:
    std::string ret_msg_;
};

class TESTERCELL_API Start
{
public:
    Start();
    static void declare_params(CellSockets &p){}
    static void declare_io(const CellSockets &p, CellSockets &i, CellSockets &o);
    //void configure(const CellSockets&, const CellSockets&, const CellSockets&);
    ReturnCode process(const CellSockets&, const CellSockets&);
    const std::string return_msg(){return ret_msg_;}
private:
    std::string ret_msg_;
};

class TESTERCELL_API Hello
{
public:
    Hello();
    static void declare_params(CellSockets &p){}
    static void declare_io(const CellSockets &p, CellSockets &i, CellSockets &o);
    ReturnCode process(const CellSockets&, const CellSockets&);
};

}//namespace TesterCell
}//namespace Quantum



#endif /* TESTERCELL_TESTER_H_ */
