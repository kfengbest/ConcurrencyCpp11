//
//  T1.cpp
//  TestConcurency11
//
//  Created by Kaven Feng on 10/3/13.
//  Copyright (c) 2013 Kaven Feng. All rights reserved.
//

#include "T1.h"
#include <list>
#include <thread>
#include <mutex>
#include <future>
#include <atomic>
#include <condition_variable>
#include <functional>

namespace T10 {
    
    
    void print(std::future<int>& fut)
    {
        std::cout << "before fut.get() \n";
        int x = fut.get();
        std::cout << "after fut.get()" << x << std::endl;
    }
    
    void run()
    {
        std::promise<int> src;
        std::future<int> fut = src.get_future();
        std::thread t(print, std::ref(fut));

        std::cout << "before set_value \n";
        src.set_value(10);
        std::cout << "after set_value \n";
        t.join();
    }
}