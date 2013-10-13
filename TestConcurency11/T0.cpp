//
//  T0.cpp
//  TestConcurency11
//
//  Created by Kaven Feng on 10/2/13.
//  Copyright (c) 2013 Kaven Feng. All rights reserved.
//

#include "T0.h"
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


namespace T03 {
    
    std::mutex mtx;
    std::condition_variable cv;
    bool gReady = false;
    
    void print(int id)
    {
        std::unique_lock<std::mutex> lk(mtx);
        while (!gReady) {
            std::cout << id << " is waiting. \n";
            cv.wait(lk);
        }
        std::cout << "print in thread: " << id << std::endl;
    }
    
    void releaseAll()
    {
        std::unique_lock<std::mutex> lk(mtx);
        gReady = true;
        std::cout << "before wake up all \n";
        cv.notify_all();
        std::cout << "after wake up all \n";
    }
    
    void run()
    {
        std::cout << "T0::run() \n";
        
        std::thread threads[10];
        for (int i = 0; i < 10; i++) {
            threads[i] = std::thread(print, i);
        }
        
        std::cout << "10 threads are ready for race condition \n";
        
        releaseAll();
        
        for (auto & th:threads) {
            th.join();
        }
        
        std::cout << "T0::run() end. \n";
    }
}

// function
// member function
// lambda
// functor
namespace T04 {

    std::mutex g_mutex;
    
    void f1(int count)
    {
        std::cout << "AAA" << count << std::endl;

    }
    
    void f2(int& count)
    {
        std::cout << "BBB" << count++ << std::endl;
    }
    
    void f3(const std::string& str1, const std::string& str2)
    {
        std::cout << "string in thread: " << str1 << str2 << std::endl;
    }
    
    class Book{
      
    public:
        void print() const {
            std::lock_guard<std::mutex> lock(g_mutex);
            std::cout << "CCC" << std::this_thread::get_id() << std::endl;
        }
        void add(int a, int b){
            std::lock_guard<std::mutex> lock(g_mutex);
            std::cout << "DDD" << std::this_thread::get_id() << " " << a+b << std::endl;
        }
    };
    
    void f4(Book* pbook)
    {
        pbook->print();
    }
    
    void run()
    {
        int n = 9;
        std::string name = "EEE";
        std::string addr = "FFF";
        Book* pb = new Book;
        pb->add(2,4);
        pb->add(1,4);
        
        std::thread t1(f1,n);                           // pass by value;
        //std::thread t2(f2,n);                         // pass by ref; not work.
        std::thread t2(f2, std::ref(n));                // pass by ref;
        std::thread t3(f3, name, std::cref(name));      // pass by const ref;
        std::thread t4(f4, pb);                         // pass by pointer;
        std::thread t5;                                 // t5 is not a thread;
        std::thread t6(std::move(t1));                  // t6 is runing f1(), t1 is no longer a thread;
        std::thread t7([](const std::string& k){        // pass by lambda
            std::cout << k;
        }, addr);
        std::thread t8(&Book::add, pb, 1,2);            // pass by member function
        std::thread t9(std::bind(f2,std::ref(n)));      // pass by std::bind
        std::thread t10(std::bind(&Book::add,pb,3,4));  // pass by std::bind
        
        //t1.join();                                    // t1 is no longer a thread. call join() will throw exception.
        t2.join();
        t3.join();
        t4.join();
        t6.join();
        t7.join();
        t8.join();
        t9.join();
        t10.join();
        delete pb; pb = nullptr;
        
        std::cout << "now n:" << n;

    }
}

// std::async
namespace T05 {
    
    int task(int n)
    {
        int res = n;
        for (int i = 0; i < 50000; i++) {
            for (int j = 0; j < 50000; j++) {
                res = i - j;
            }
            
            if ( i % 10000 == 0) {
                std::cout << i << std::endl;
            }
        }
        return res;
    }
    
    void step1()
    {
        std::cout << "start calculating \n";
        std::future<int> res = std::async(task, 10);
        
        std::cout << "before res.get() \n";
        int r = res.get();
        std::cout << "after res.get: " << r << std::endl; // will be executed after res.get() is returned.
    }
    
    void step2()
    {
        std::cout << "step 2 \n"; 
    }
    
    void run()
    {
        step1();
        step2();
    }
}

// promise/future:  thread1 notify main.
namespace T06 {
    
    std::promise<int> g_res;
    
    void task()
    {
        int res = 0;
        for (int i = 0; i < 50000; i++) {
            for (int j = 0; j < 50000; j++) {
                res = i - j;
            }
            
            if ( i % 10000 == 0) {
                std::cout << i << std::endl;
            }
        }
        
        g_res.set_value(res);
    }
    
    void run()
    {
        std::future<int> fut = g_res.get_future();

        std::cout << "start calculating \n";
        std::thread t(task);
        
        std::cout << "before res.get() \n";
        int r = fut.get();
        std::cout << "after res.get: " << r << std::endl; // will be executed after res.get() is returned.

        t.join();
    }
}

// promise/future. main notify thread1
namespace T07 {
    
    void task(std::future<int>& fut)
    {
        std::cout << "before res.get() \n";
        int r = fut.get();
        std::cout << "after res.get() " << r << std::endl; // will be executed after res.get() is returned.

    }
    
    void heartbeat()
    {
        int res = 0;
        for (int i = 0; i < 50000; i++) {
            for (int j = 0; j < 50000; j++) {
                res = i - j;
            }
            
            if ( i % 10000 == 0) {
                std::cout << i << std::endl;
            }
        }
    }
    
    void run()
    {
        std::promise<int> pro;
        std::future<int> fut = pro.get_future();
        std::thread t(task, std::ref(fut));
        heartbeat();
        pro.set_value(10);
        t.join();
    }
}

namespace T08 {
    
    bool heartbeat()
    {
        int res = 0;
        for (int i = 0; i < 50000; i++) {
            for (int j = 0; j < 50000; j++) {
                res = i - j;
            }
            
            if ( i % 10000 == 0) {
                std::cout << i << std::endl;
            }
        }
        
        return true;
    }
    
    int countdown(int from, int to) {
        for (int i=from; i!=to; --i) {
            std::cout << i << '\n';
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        std::cout << "Finished!\n";
        return from - to;
    }
    
    
    void task_lambda()
    {
        std::packaged_task<int(int,int)> task([](int a, int b){
            heartbeat();
            return a + b;
        });
        
        //task(2,9); // run in main thread?
        
        //std::packaged_task<int(int,int)> taskObj = std::move(task);
        auto taskObj = std::move(task);
        std::future<int> fut = taskObj.get_future();     // must be called before thread started!
        std::thread t(std::move(taskObj), 3,5); 
        
        std::cout << "task lambda: before res.get() \n";
        int r = fut.get();
        std::cout << "task lambda: " << r << std::endl;
        
        t.join();
    }
    
    void task_thread()
    {
        std::packaged_task<int(int,int)> task(&countdown);  // 设置 packaged_task
        std::future<int> ret = task.get_future();           // 获得与 packaged_task 共享状态相关联的 future 对象.
        std::thread th(std::move(task), 9, 1);              // 创建一个线程完成计算任务.
        
        std::cout << "task_thread before fut.get() " << std::endl;
        int r = ret.get();                                  // 等待任务完成并获取结果
        std::cout << "task_thread after fut.get() " << r << std::endl;
        th.join();
        
    }
    
    void task_thread2()
    {
        std::packaged_task<bool()> task(&heartbeat);
        std::cout << "before get_future:  task.valid() " << task.valid() << std::endl;
        std::future<bool> ret = task.get_future();
        std::cout << "after get_future:  task.valid() " << task.valid() << "future::valid() " << ret.valid() << std::endl;

        std::thread th(std::move(task));
        bool r = ret.get();                                  // 等待任务完成并获取结果
        std::cout << "task_thread2 after fut.get() " << r << std::endl;
        
        th.join();
    }
    
    int triple(int v)
    {
        heartbeat();
        return 3*v;
    }
    
    void task_thread3()
    {
        std::cout << "task_thread3 begin \n";
        
        std::packaged_task<int(int)> task(&triple);
        std::future<int> fut = task.get_future();
        std::thread(std::move(task), 100).detach();
        std::cout << "task 100 " << fut.get() << std::endl;
        
        task.reset(); // throw exception on xcode. not sure why.
        fut = task.get_future();
        std::thread(std::move(task), 200).detach();
        std::cout << "task 200 " << fut.get() << std::endl;

    }
    
    void task_function()
    {
        std::function<bool()> func(&heartbeat);
        std::packaged_task<bool()> task(func);
        std::future<bool> ret = task.get_future();
        std::thread(std::move(task)).detach();
        std::cout << "finished " << ret.get() << std::endl;
    }
    
    class Command
    {
    public:
        void execute()
        {
            std::packaged_task<bool(int)> task(std::bind(&Command::doTask, this, std::placeholders::_1));
            std::future<bool> fut = task.get_future();
            std::thread(std::move(task), 10).detach();
            std::cout << "execute finished " << fut.get() << std::endl;
        }
        
        bool doTask(int v)
        {
            heartbeat();
            return true;
        }
    };
    
    void run()
    {
      //  task_lambda();
      //  task_thread();
      //  task_thread2();
      ////  task_thread3();
      //  task_function();
        
        Command cmd;
        cmd.execute();
        std::cout << "after execute() " << std::endl;

    }
}

// function, bind
namespace T09 {
    
    void print()
    {
        std::cout << "print() \n";
    }
    
    void dump()
    {
        std::cout << "dump() \n";
    }
    
    void plus(int a)
    {
        std::cout << "plus " << a << std::endl;
    }
    
    int add(int a, int b)
    {
        return a + b;
    }
    
    void increase(int& a)
    {
        a++;
    }
    
    class Book
    {
    public:
        int add(int a, int b)
        {
            return a + b;
        }
        
        void switchTo(const std::function<void(int,int)>& func)
        {
            mFunc = func;
        }
        
        void execute(int a, int b)
        {
            mFunc(a,b);
        }
        
        std::function<void(int,int)> mFunc;
    };
    
    typedef std::function<void()> Func1;
    
    void run()
    {
        Func1 f1(print);
        f1();
        
        std::function<void()> f2(print);
        f2();
        
        auto f3 = std::function<void()>(print);
        f3();
        
        std::function<void()> f4 = f2;
        f4();
        
        f4 = std::function<void()>(dump);
        f4();
        
        std::function<void(int)> f5(plus);
        f5(5);
        
        Book b1;
        std::function<int(int,int)> f6(std::bind(&Book::add, &b1, std::placeholders::_1, std::placeholders::_2));
        int i6 = f6(2,3);
        std::cout << "Book::add: " << i6 << std::endl;
        
        b1.switchTo([](int a, int b){
            std::cout << "switchto " << a + b << std::endl;
        });
        b1.execute(1, 1);
        
        auto f7 = [](int a, int b){std::cout << "[](){} 7 " << a + b << std::endl;};
        b1.switchTo(f7);
        b1.execute(2, 3);
        
        // global function
        Func1 f11 = print;
        f11();
        
        // lambda
        std::function<int(int,int)> f12 = [](int a, int b)->int{ std::cout << "[](int,int){} \n"; return a + b; }; 
        f12(3,1);
        
        // std::bind
        std::function<int(int)> f13 = std::bind(&add, 100, std::placeholders::_1); 
        std::cout << "f13 bind " << f13(3) << std::endl;
        
        // member function via bind
        std::function<int(int,int)> f14 = std::bind(&Book::add, &b1, std::placeholders::_1, std::placeholders::_2);
        std::cout << "bind(&Book::add, &b1) " << f14(2,3) << std::endl;
        
        // test std::mem_fn
        auto f20 = std::mem_fn(&Book::add);
        std::cout << "mem_fn(&Book::add) " << f20(&b1, 3, 4) << std::endl;
        
        std::cout << "mem_fn(&Book::add)(&b1, 2,3) " << std::mem_fn(&Book::add)(&b1,2,3) << std::endl;

        // std::bind
        auto f30 = std::bind(&Book::add, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        std::cout << "bind(&Book::add,_1,_2,_3) " << f30(&b1, 3, 4) << std::endl;
        
        auto f31 = std::bind(&Book::add, &b1, std::placeholders::_1, std::placeholders::_2);
        std::cout << "bind(&Book::add,&b1,_1,_2) " << f31(3, 4) << std::endl;

        auto f32 = std::bind(&Book::add, b1, std::placeholders::_1, std::placeholders::_2);
        std::cout << "bind(&Book::add,b1,_1,_2) " << f32(3, 4) << std::endl;
        
        auto f33 = std::bind(&Book::add, b1, 1000, std::placeholders::_1);
        std::cout << "bind(&Book::add,b1, 1000,_1,) " << f33(3) << std::endl;
        
        // std::ref
        int num = 1;
        auto rNum = std::ref(num);
        increase(rNum);
        std::cout << "ref " << num  << " rNum.get() " << rNum.get() << std::endl;
        
    }
    
}

namespace T0A {
    
    void run()
    {
        std::plus<int> p1;
        std::cout << "plus(1,1) = " << p1(1,1) << std::endl;
        
        
    }
}
