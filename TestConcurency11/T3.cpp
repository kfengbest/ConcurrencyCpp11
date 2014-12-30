//
//  T3.cpp
//  TestConcurency11
//
//  Created by Kaven Feng on 12/30/14.
//  Copyright (c) 2014 Kaven Feng. All rights reserved.
//

#include "T3.h"
#include <list>
#include <thread>
#include <mutex>
#include <future>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <chrono>
#include <cmath>
#include <functional>
#include <algorithm>
#include <vector>
#include <numeric>
#include <ostream>
#include <stack>

// structured fork-join parallelism
// fork tasks, computing, no need to wait for results.
namespace T30 {
    
    void run()
    {
        unsigned cores = std::thread::hardware_concurrency();
        std::vector<std::thread> threadPool;
        
        // fork
        for (int i = 0; i < cores; i++) {
            auto task = [](){std::cout << "working...\n";};
            std::thread t(task);
            threadPool.push_back(std::move(t));
        }
        
        // join
        for (std::thread& t : threadPool) {
            t.join();
        }
    }
    
    /* result
     
     working...
     working...
     wwwowoorworwrkorkokirkirinkinkngingig.ng.n..g..g.......
     ..
     .
     .
     .
     
    */
}

// fork tasks, computing, wait for results.
// the total time depends on longest one.
namespace T31 {
    
    void run()
    {
        unsigned cores = std::thread::hardware_concurrency();
        std::vector<std::future<int> > futures;
        
        for (int i = 0; i < cores; i++){
            auto sum = [](int from, int to)->int
            {
                int r = 0;
                for (int i = from; i <= to; i++) {
                    r += i;
                }
                std::cout << r << std::endl;
                return r;
            };
            // fork a thread
            std::future<int> f = std::async(std::launch::async, sum, 0, i);

            // push to waiting list
            futures.push_back(std::move(f));
        }
        
        // wait for results.
        int r = 0;
        for (auto& f : futures){
            r += f.get();
        }
        std::cout << "total: " << r << std::endl;
    }
    
    /*
     0
     1
     3
     6
     10
     15
     21
     28
     total: 84
    */
}





