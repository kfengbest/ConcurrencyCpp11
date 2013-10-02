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

// sub threads are mutexed.
// Main and sub thread are still race condition.
namespace T01 {
    
    std::mutex mtx;
    
    void print(int id)
    {
        std::lock_guard<std::mutex> lk(mtx);
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
/* T1 may look like this:
 
 Hello, World!
 T0::run()
 print in thread: 1
 print in thread: 0
 1p0r itnhtr eiand st harreea dr:e ady for rac2e
 condition
 print in thread: 3
 print in thread: 4
 print in thread: 5
 print in thread: 6
 print in thread: 7
 print in thread: 8
 print in thread: 9
 T0::run() end.

 */


// Main and sub threads are all mutex.
namespace T02 {
    
    std::mutex mtx;
    
    void print(int id)
    {
        std::lock_guard<std::mutex> lk(mtx);
        std::cout << "print in thread: " << id << std::endl;
    }
    
    void run()
    {
        std::cout << "T0::run() \n";
        
        std::thread threads[10];
        for (int i = 0; i < 10; i++) {
            threads[i] = std::thread(print, i);
        }
        
        mtx.lock();
        std::cout << "10 threads are ready for race condition \n";
        mtx.unlock();
        
        for (auto & th:threads) {
            th.join();
        }
        
        std::cout << "T0::run() end. \n";
    }
}
/*
 Hello, World!
 T0::run()
 print in thread: 0
 print in thread: 1
 print in thread: 2
 print in thread: 4
 print in thread: 3
 print in thread: 5
 print in thread: 6
 print in thread: 8
 10 threads are ready for race condition
 print in thread: 9
 print in thread: 7
 T0::run() end.
*/