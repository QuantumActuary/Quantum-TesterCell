/*
 * Copyright (c) Thomas Chen - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Thomas Chen <tkchen@gmail.com>, August 2015
 *
 * logiccell.cpp
 */

#include "testercell.h"
#include "Tests/cells.hpp"

#include "TesterCell/tester.h"

extern "C" TESTERCELL_API int getEngineVersion()
{
    return 1;
}

extern "C" TESTERCELL_API void registerPlugin(Quantum::Kernel &kernel)
{
    using namespace Quantum;
    std::vector<cell_ptr> cells_to_add;

    Cell_<TesterCell::If>::SHORT_DOC = "If/Else";
    Cell_<TesterCell::If>::MODULE_NAME = "TesterCellPlugin";
    Cell_<TesterCell::If>::CELL_NAME = "If";
    cell_ptr If(new Cell_<TesterCell::If>());
    If->name("If");
    cells_to_add.push_back(If);

    Cell_<TesterCell::Print>::SHORT_DOC = "Printer";
    Cell_<TesterCell::Print>::MODULE_NAME = "TesterCellPlugin";
    Cell_<TesterCell::Print>::CELL_NAME = "Print";
    cell_ptr print(new Cell_<TesterCell::Print>());
    print->name("Print");
    cells_to_add.push_back(print);

    Cell_<TesterCell::Start>::SHORT_DOC = "Starter";
    Cell_<TesterCell::Start>::MODULE_NAME = "TesterCellPlugin";
    Cell_<TesterCell::Start>::CELL_NAME = "Start";
    cell_ptr start(new Cell_<TesterCell::Start>());
    start->name("Start");
    cells_to_add.push_back(start);

    Cell_<TesterCell::Hello>::SHORT_DOC = "Hello";
    Cell_<TesterCell::Hello>::MODULE_NAME = "TesterCellPlugin";
    Cell_<TesterCell::Hello>::CELL_NAME = "Hello";
    cell_ptr hello(new Cell_<TesterCell::Hello>());
    hello->name("Hello");
    cells_to_add.push_back(hello);

    Cell_<NeverOutput>::SHORT_DOC = "Creates infinite loop";
    Cell_<NeverOutput>::MODULE_NAME = "TesterCellPlugin";
    Cell_<NeverOutput>::CELL_NAME = "NeverOutput";
    cell_ptr never_output(new Cell_<NeverOutput>());
    never_output->name("NeverOutput");
    cells_to_add.push_back(never_output);

    Cell_<Pause>::SHORT_DOC = "Pause";
    Cell_<Pause>::MODULE_NAME = "TesterCellPlugin";
    Cell_<Pause>::CELL_NAME = "Pause";
    cell_ptr pause(new Cell_<Pause>());
    pause->name("Pause");
    cells_to_add.push_back(pause);

    for(cell_ptr c: cells_to_add)
    {
        c->declare_params();
        c->declare_io();
        c->init();
        kernel.getCellRegistry().addCell(c, c->name());
    }
}
