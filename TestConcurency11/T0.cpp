//
//  T0.cpp
//  TestConcurency11
//
//  Created by Kaven Feng on 10/2/13.
//  Copyright (c) 2013 Kaven Feng. All rights reserved.
//

#include "T0.h"
#include <thread>
#include <mutex>
#include <condition_variable>

// The printed result is difference every time.
namespace T0 {
    
    void print(int id)
    {
        std::cout << "print in thread: " << id << std::endl;
    }
    
    void run()
    {
        std::cout << "T0::run() \n";
        
        std::thread threads[10];
        for (int i = 0; i < 10; i++) {
            threads[i] = std::thread(print, i);
        }
        std::cout << "10 threads are ready for race condition \n";
        
        for (auto & th:threads) {
            th.join();
        }
        
        std::cout << "T0::run() end. \n";
    }
}
