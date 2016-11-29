/*
 * test_scheduler.hpp
 *
 * Copyright (c) Thomas - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Thomas <tkchen@gmail.com>, Feb 3, 2016
 */

#ifndef TESTS_TEST_SCHEDULER_HPP_
#define TESTS_TEST_SCHEDULER_HPP_

#include "Engine/all.hpp"
#include "cells.hpp"
#include "gtest/gtest.h"
#include "primesieve.hpp"


namespace Quantum
{
    TEST(Scheduler, Prime_generator)
    {
        std::vector<int> primes;
        //Store the primes <=10
        primesieve::generate_primes(10, &primes);
        EXPECT_EQ(2, primes[0]);
        EXPECT_EQ(3, primes[1]);
        EXPECT_EQ(5, primes[2]);
        EXPECT_EQ(7, primes[3]);

        primes.clear();
        primesieve::generate_n_primes(1000, &primes);
        EXPECT_EQ(7919, primes[999]);
        EXPECT_EQ(541, primes[99]);
        /*
        int prime_prod = 1;
        for(int i = 0; i<1000; i++)
        {
            prime_prod *= primes[i];
            std::cout<<prime_prod<<std::endl;
        }
        */
    }
    TEST(Scheduler, Break_deadlock)
    {
        cell_ptr c1 = std::make_shared<Cell_<NeverOutput>>();
        c1->declare_params();
        c1->declare_io();
        circuit_ptr c(new Circuit);
        c->insert(c1);
        Scheduler sched(c);
        EXPECT_NO_THROW(sched.execute(10));
        EXPECT_FALSE(c1->outputs["done"]->get<bool>());
    }

    TEST(Scheduler, Long_process_is_not_deadlock)
    {
        cell_ptr blocker = std::make_shared<Cell_<NeverOutput>>();
        blocker->declare_params();
        blocker->declare_io();
        cell_ptr processor = std::make_shared<Cell_<Pause>>();
        processor->declare_params();
        processor->declare_io();
        cell_ptr unreachable = std::make_shared<Cell_<Pause>>();
        unreachable->declare_params();
        unreachable->declare_io();
        circuit_ptr c(new Circuit);
        c->insert(blocker);
        c->insert(processor);
        c->insert(unreachable);
        c->connect(processor, "done", blocker, "a");
        c->connect(blocker, "done", unreachable, "link");
        processor->inputs["milliseconds"] << 500;
        unreachable->inputs["milliseconds"] << 100;
        blocker->inputs["ret"] << 0;
        Scheduler sched(c);
        EXPECT_NO_THROW(sched.execute(1));
        EXPECT_TRUE(sched.finished());
        EXPECT_TRUE(processor->outputs["done"]->get<bool>()); //long process
        EXPECT_TRUE(blocker->outputs["link"]->get<bool>()); //still reachable
        EXPECT_FALSE(unreachable->outputs["done"]->get<bool>()); //Deadlock here
    }

    TEST(Scheduler, Interrupt_infinit_loop)
    {
        cell_ptr blocker = std::make_shared<Cell_<NeverOutput>>();
        blocker->declare_params();
        blocker->declare_io();
        cell_ptr processor = std::make_shared<Cell_<Pause>>();
        processor->declare_params();
        processor->declare_io();
        cell_ptr timer = std::make_shared<Cell_<Pause>>();
        timer->declare_params();
        timer->declare_io();
        cell_ptr breaker = std::make_shared<Cell_<NeverOutput>>();
        breaker->declare_params();
        breaker->declare_io();
        circuit_ptr c(new Circuit);
        c->insert(blocker);
        c->insert(processor);
        c->insert(timer);
        c->insert(breaker);
        c->connect(processor, "done", blocker, "a"); //infinite loop
        c->connect(timer, "done", breaker, "a"); //breaks out after timer
        processor->inputs["milliseconds"] << 10;
        timer->inputs["milliseconds"] << 500;
        blocker->inputs["ret"] << 2; //causes infinite DO_OVER
        breaker->inputs["ret"] << 3; //break out of execution
        Scheduler sched(c);
        EXPECT_NO_THROW(sched.execute(1));
        EXPECT_TRUE(processor->outputs["done"]->get<bool>());
        EXPECT_TRUE(blocker->outputs["link"]->get<bool>());
        EXPECT_TRUE(breaker->outputs["link"]->get<bool>());

    }

    TEST(Scheduler, debugger_resets_at_end)
    {
        cell_ptr adder1 = std::make_shared<Cell_<Add>>();
        adder1->declare_params();
        adder1->declare_io();
        cell_ptr adder2 = adder1->clone();
        circuit_ptr c(new Circuit());
        c->insert(adder1);
        c->insert(adder2);
        c->connect(adder1, "out", adder2, "left");
        adder1->inputs["left"] << 1.0;
        adder1->inputs["right"] << 2.0; //1 + 2
        adder2->inputs["right"] << 0.0; //3 + 0
        Scheduler s(c);

        s.debug(true);
        s.execute(2);
        EXPECT_EQ(0, adder1->outputs["out"]->token_id());
        EXPECT_EQ(-1, adder2->outputs["out"]->token_id());
        s.execute(2);
        EXPECT_EQ(0, adder2->outputs["out"]->token_id());
        EXPECT_EQ(3.0, adder2->outputs["out"]->get<double>());
        //note adder2 is also processed in this step, is this desired?!
        //wont there be a possibility to overwrite tokens that havent been consumed?
        EXPECT_EQ(1, adder1->outputs["out"]->token_id());

        EXPECT_TRUE(s.executing());
        s.execute(2);
        EXPECT_EQ(1, adder1->outputs["out"]->token_id());
        EXPECT_EQ(1, adder2->outputs["out"]->token_id());
        EXPECT_EQ(3.0, adder2->outputs["out"]->get<double>());

        //the scheduler should now have reset and is ready for another run
        adder1->inputs["left"] << 2.0; //2 + 2
        EXPECT_FALSE(s.executing());
        EXPECT_FALSE(s.running());
        s.execute(1);
        EXPECT_EQ(0, adder1->outputs["out"]->token_id());
        EXPECT_EQ(4.0, adder1->outputs["out"]->get<double>());
        EXPECT_TRUE(adder1->outputs["out"]->is_new());

        s.execute(1);
        EXPECT_EQ(0, adder1->outputs["out"]->token_id());
        //note adder2 is not re-processed like before, why?
        EXPECT_EQ(0, adder2->outputs["out"]->token_id());
        EXPECT_EQ(4.0, adder2->outputs["out"]->get<double>());
    }

    TEST(Scheduler, Parallel_scheduling)
    {
        cell_ptr sleeper = std::make_shared<Cell_<Pause>>();
        sleeper->declare_params();
        sleeper->declare_io();

        cell_ptr s1 = sleeper->clone();
        cell_ptr s2 = sleeper->clone();
        cell_ptr s3 = sleeper->clone();
        cell_ptr s4 = sleeper->clone();
        circuit_ptr c(new Circuit);
        c->insert(s1);
        c->insert(s2);
        c->insert(s3);
        c->insert(s4);
        c->connect(s3, "done", s1, "link");
        c->connect(s4, "done", s2, "link");
//        Graph:
//            s3--s1
//            s4--s2
        s1->inputs["milliseconds"] << 100;
        s2->inputs["milliseconds"] << 100;
        s3->inputs["milliseconds"] << 100;
        s4->inputs["milliseconds"] << 100;

        Scheduler sched(c);
        auto t1 = std::chrono::high_resolution_clock::now();
        sched.execute(1);
        auto t2 = std::chrono::high_resolution_clock::now();
        std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1);

        EXPECT_LT(ms.count(), 400);
        SocketHandle<bool> done1 = s1->outputs["done"];
        SocketHandle<bool> done2 = s2->outputs["done"];
        SocketHandle<bool> done3 = s3->outputs["done"];
        SocketHandle<bool> done4 = s4->outputs["done"];
        EXPECT_TRUE(*done1 && *done2 && *done3 && *done4);

        circuit_ptr c1(new Circuit);
        c1->insert(s1);
        c1->insert(s2);
        c1->insert(s3);
        c1->insert(s4);
        c1->connect(s1, "done", s2, "link");
        c1->connect(s1, "done", s3, "link");
        c1->connect(s1, "done", s4, "link");
//        Graph:
//              /-s2
//           s1 --s3
//              \-s4
        Scheduler sched1(c1);
        t1 = std::chrono::high_resolution_clock::now();
        sched1.execute(1);
        t2 = std::chrono::high_resolution_clock::now();
        ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1);
        EXPECT_LT(ms.count(), 400);

    }
    TEST(Scheduler, Circuit_editing)
    {
        cell_ptr sleeper = std::make_shared<Cell_<Pause>>();
        sleeper->declare_params();
        sleeper->declare_io();

        cell_ptr s1 = sleeper->clone();
        cell_ptr s2 = sleeper->clone();
        SocketHandle<bool> done1 = s1->outputs["done"];
        SocketHandle<bool> done2 = s2->outputs["done"];
        circuit_ptr c(new Circuit);
        c->insert(s1);
        c->insert(s2);
        c->connect(s1, "done", s2, "link");
        Scheduler S(c);
        S.execute(1);
        EXPECT_TRUE(*done1 && *done2);
        //c->disconnect(s1, "done", s2, "link");
        c->remove(s2);
        cell_ptr s3 = sleeper->clone();
        SocketHandle<bool> done3 = s3->outputs["done"];
        c->insert(s3);
        c->connect(s1, "done", s3, "link");
        Scheduler S2(c);
        //S.execute(2);
        //After editing the circuit, we can execute again in a different
        //scheduler
        S2.execute(1);
        EXPECT_TRUE(*done3);
    }
}

#endif /* TESTS_TEST_SCHEDULER_HPP_ */
