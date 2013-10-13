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
#include <chrono>

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

namespace T11 {
    
    
    void print(std::shared_future<int>& fut)
    {
        std::cout << "A" << std::endl;
        int x = fut.get();
        std::cout << "B" << x << std::endl;
    }
    
    void run()
    {
        std::promise<int> src;
        std::shared_future<int> fut = src.get_future();
        std::thread t(print, std::ref(fut));
        
        std::thread threads[10];
        for (int i = 0; i < 10; i++) {
            threads[i] = std::thread(print, std::ref(fut));
        }
        
        //fut.wait();
        std::cout << "C \n";
        src.set_value(9);
        std::cout << "D \n";
        
        for (auto & th:threads) {
            th.join();
        }
    }
}

namespace T12 {
    
    void run()
    {
        
        std::packaged_task<int()> task([=](){
            int from = 9, to = 0;
            std::cout << "begin counting \n";
            for (int i = from; i != to; --i) {
                std::cout << i << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            std::cout << "end counting \n";
            return from - to;
        });
        
        std::future<int> ret = task.get_future();
        std::thread th(std::move(task));
        
        // main thread
        for (int i = 100; i <150; ++i) {
            std::cout << i << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // waiting for worker thread
        int r = ret.get();

        std::cout << "result: " << r;
        th.join();
        
    }
}

namespace T13 {
    
    void run()
    {
        
    }
}