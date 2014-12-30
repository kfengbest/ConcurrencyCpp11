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