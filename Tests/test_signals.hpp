#include "boost/signals2.hpp"
#include "boost/shared_ptr.hpp"
#include "gtest/gtest.h" //https://code.google.com/p/googletest/wiki/Documentation
#include "Engine/cellsocket.hpp"

#include <vector>
#include <memory>
#include <thread>
#include <mutex>

namespace Quantum
{
    class MulSlot
    {
    public:
        int operator()(const int x, const int y) const
        {
            return x*y;
        }
    };

    class AddSlot
    {
    public:
        int operator()(const int x, const int y) const
        {
            return x+y;
        }
    };

    class GenericObject
    {
    public:
        int genericFunction(int a, int b)
        {
            MulSlot multiply;
            return multiply(a, b);
        }
    };

    class MaxCombiner
    {
    public:
        typedef int result_type;
        template<typename InputIterator>
        int operator()(InputIterator first, InputIterator last) const
        {
            if(first == last) return int();
            int max_value = *first++;
            while(first != last)
            {
                if(max_value < *first)
                {
                    max_value = *first;
                }
                ++first;
            }
            return max_value;
        }
    };

    class SumCombiner
    {
    public:
        typedef int result_type;
        template<typename InputIterator>
        int operator()(InputIterator first, InputIterator last) const
        {
            if(first == last) return int(); //only one element
            int sum_value = *first++;
            while(first != last)
            {
                sum_value += *first;
                ++first;
            }
            return sum_value;
        }
    };

    class ConnectionManager
    {
        MulSlot mul;
    public:
        boost::signals2::signal<int (int, int), MaxCombiner> publishCombine;
        boost::signals2::signal<int (int, int), SumCombiner> publishSum;
        void makeConnection()
        {
            publishCombine.connect(mul);
        }
        void breakConnection()
        {
            publishCombine.disconnect_all_slots();
        }
    };

	class Signals2Test: public ::testing::Test
	{
	protected:
		boost::signals2::signal<int (int, int)> publish;
		boost::signals2::signal<int (int, int), MaxCombiner> publishCombine;
		boost::signals2::connection c;
		MulSlot mul;
		AddSlot add;

		int result=0;
		std::mutex m;
	public:
		void loop()
		{
			std::lock_guard<std::mutex> lock(m);
			for(int i=0; i<100; ++i)
			{
				result += publish(0,i).value();
			}
		}
	};


	TEST_F(Signals2Test, connect)
	{
		c = publish.connect(mul);
		EXPECT_TRUE(c.connected());
	}

	TEST_F(Signals2Test, disconnect)
	{
		c = publish.connect(mul);
		EXPECT_TRUE(c.connected());
		c.disconnect();
		EXPECT_FALSE(c.connected());
	}

	TEST_F(Signals2Test, publish)
	{
		c = publish.connect(mul);
		EXPECT_EQ(6, publish(2,3));
	}

	TEST_F(Signals2Test, publishCombine)
	{
		publishCombine.connect(mul);
		publishCombine.connect(add);
		EXPECT_EQ(3, publishCombine(2,1)); //max of mul and add
		EXPECT_EQ(6, publishCombine(3,2));
	}

	TEST_F(Signals2Test, publish_with_order)
	{
	    publish.connect(0, mul);
	    publish.connect(1, add);
	    EXPECT_EQ(3, publish(2,1).value()); //executes mul then add
	    EXPECT_EQ(5, publish(3,2).value());
	    publish.disconnect_all_slots();
	    publish.connect(1, mul);
	    publish.connect(0, add);
	    EXPECT_EQ(2, publish(2,1).value()); //executes add then mul
	    EXPECT_EQ(6, publish(3,2).value());
	}

	TEST_F(Signals2Test, useMemberFunction)
	{
		std::unique_ptr<GenericObject> obj(new GenericObject());
		publish.connect(boost::bind(&GenericObject::genericFunction, obj.get(), _1, _2));
		EXPECT_EQ(6, publish(2,3));

	}

	TEST_F(Signals2Test, autoRelease)
	{
		boost::shared_ptr<GenericObject> obj(new GenericObject());
		publish.connect(boost::signals2::signal<int (int, int)>::slot_type(&GenericObject::genericFunction, obj.get(), _1, _2).track(obj));
		EXPECT_EQ(6, publish(2,3));
		EXPECT_EQ(1, publish.num_slots());
		obj.reset();
		EXPECT_EQ(0, publish.num_slots());
	}

	TEST_F(Signals2Test, signalConnection) //create new connections through a signal
	{
		boost::shared_ptr<ConnectionManager> obj(new ConnectionManager());
		boost::signals2::signal<void()> s;
		s.connect(boost::signals2::signal<void ()>::slot_type(&ConnectionManager::makeConnection, obj.get()).track(obj));
		EXPECT_EQ(0, obj->publishCombine.num_slots());
		obj->publishCombine.connect(add);
		EXPECT_EQ(1, obj->publishCombine.num_slots());
		EXPECT_EQ(5, obj->publishCombine(2,3));
		s();
		EXPECT_EQ(2, obj->publishCombine.num_slots());
		EXPECT_EQ(6, obj->publishCombine(2,3));
	}

	TEST_F(Signals2Test, signalDisconnect) //cause a disconnect through a signal
	{
		boost::shared_ptr<ConnectionManager> obj(new ConnectionManager());
		boost::signals2::signal<void()> s;
		s.connect(boost::signals2::signal<void ()>::slot_type(&ConnectionManager::breakConnection, obj.get()).track(obj));
		EXPECT_EQ(0, obj->publishCombine.num_slots());
		obj->publishCombine.connect(add);
		EXPECT_EQ(1, obj->publishCombine.num_slots());
		EXPECT_EQ(5, obj->publishCombine(2,3));
		s();
		EXPECT_EQ(0, obj->publishCombine.num_slots());
	}

	TEST_F(Signals2Test, multithreading)
	{
		result = 0;
		publish.connect(add);
		std::thread t1(&Signals2Test::loop, this);
		std::thread t2(&Signals2Test::loop, this);
		t1.join();
		t2.join();
		EXPECT_EQ(9900, result);
	}
/*
	TEST_F(Signals2Test, CellSocket_signals)
	{
	    CellSocket cs(1, "An int");
	    cs.set_callback<int>([](int pid){std::cout<<pid<<std::endl;});
	    cs << 100;
	    cs.token_id(50);
	    cs.dirty(true);
	    cs.notify();
	}
*/
}
