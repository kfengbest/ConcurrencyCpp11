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
#include <vector>
#include <numeric>
#include <ostream>
#include <stack>


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
    

    class Command
    {
    public:
        Command(){std::cout << "ctor() \n";}
        virtual ~Command(){std::cout << "~Command() \n";}
        
    public:
        void execute(){ std::cout << "Command::execute() \n";}
    };
    
    void test_for_each()
    {
        // init vector.
        int data[] = {1,2,3,4,5,6};
        std::vector<int> vec(data, data + sizeof(data)/sizeof(int));
        
        std::cout << "std::for_each: \n";
        std::for_each(vec.begin(),vec.end(), [](int e){
            std::cout << e << " ";
        });
        
        std::cout << std::endl << "for(:) \n" ;
        for (auto it : vec) { std::cout << it << " ";}
        
        std::cout << std::endl << "for(:) \n" ;
        for (int it : vec) { std::cout << it << " ";}
        
        std::cout << std::endl << "for(:) \n" ;
        for (int it : data) { std::cout << it << " "; }
        
        for (auto e : vec){ std::cout << e << " ";}
        for (auto& e : vec){ std::cout << e++ << " ";}
        for (const auto& e : vec){ std::cout << e << " ";}

        // init
        std::vector<Command*> cmdVec;
        for (int i = 0; i < 10; i++) {
            Command* cmd = new Command();
            cmdVec.push_back(cmd);
        }
        
        // mem_fn
        std::for_each(cmdVec.begin(), cmdVec.end(), std::mem_fn(&Command::execute));
        
        // clear
        std::for_each(cmdVec.begin(), cmdVec.end(), [&](Command* cmd){ delete cmd; cmd = nullptr;});
        cmdVec.clear();
        
    }
    
    void test_copy()
    {
        int data[] = {1,2,3,4,5};
        std::vector<int> vec1;
        int size = sizeof(data) / sizeof(int);
        vec1.resize(size); // must!
        std::copy(data, data + size, vec1.begin());
        std::for_each(vec1.begin(), vec1.end(), [](int e){
            std::cout << e << " ";
        });
    }
    
    void test_sort()
    {
        // init
        int data[] = {4,2,5,1,3};
        std::vector<int> vec(data, data + sizeof(data)/sizeof(int));
        
        // sort low -> high
        std::sort(vec.begin(), vec.end());
        for (auto e : vec){ std::cout << e << " ";}
        std::cout << "\n";
        
        // sort high -> low
        std::sort(vec.begin(), vec.end(), [](int i, int j){ return i > j;});
        for (auto& e : vec){ std::cout << e++ << " "; }
        std::cout << "\n";

        // sort partially
        std::sort(vec.begin(), vec.begin() + 3);
        for (const auto& e : vec){ std::cout << e << " "; }
        std::cout << "\n";

    }

    void test_find()
    {
        // init
        int data[] = {4,2,5,1,3,6,3,7};
        std::vector<int> vec(data, data + sizeof(data)/sizeof(int));

        for (const auto& e : vec){ std::cout << e << " ";}
        std::cout << std::endl;
        
        auto it = std::find(vec.begin(), vec.end(), 5);
        std::cout << "find(5): " << *it << std::endl;
        
        auto it2 = std::find(vec.begin(), vec.end(), 1000);
        std::cout << "find(1000): " << *it2 << "  " << (it2 != vec.end()) << std::endl;

        auto it3 = std::find_if(vec.begin(), vec.end(), [](const int& e){ return e % 2 == 0;});
        std::cout << "find_if(e%2==0): " << *it3 << "  " << (it3 != vec.end()) << std::endl;

        bool b1 = std::any_of(vec.begin(), vec.end(), [](const int& e){return e % 3 == 0; });
        std::cout << "any_of(e%3==0): " << b1 << std::endl;

        auto c3 = std::count(vec.begin(), vec.end(), 3);
        std::cout << "count(3): " << c3 << std::endl;
        
        auto oddn = std::count_if(vec.begin(), vec.end(), [](int e){ return e%2==0;});
        std::cout << "count_if(e%2): " << oddn << std::endl;


    }

    std::ostream& operator<<(std::ostream& ostr, const std::list<int>& ls)
    {
        for (auto& e : ls){
            ostr << " " << e;
        }
    
        return ostr;
    }

    void test_splice()
    {
        std::list<int> ls1 = {1,2,3,4,5,6,7,8,9};
        std::list<int> ls2 = {20,21,22,23,24,25,26};
        std::list<int> ls;
        
        auto it = ls1.begin();
        std::advance(it, 2);
        ls1.splice(it, ls2);
        ls.splice(ls.begin(), ls1);
        
        std::cout << "ls1 " << ls1 << std::endl;
        std::cout << "ls2 " << ls2 << std::endl;
        
        auto ite = ls1.end();
        
        ls2.splice(ls2.begin(), ls1, it, ls1.end());
        
        std::cout << "ls1 " << ls1 << std::endl;
        std::cout << "ls2 " << ls2 << std::endl;
        
    }
    

    
    
    void test_partition()
    {
        std::list<int> ls1 = {1,2,3,4,5,6,7,8,9};
        auto it11 = ls1.begin();
        auto it12 = std::next(it11, 3);
        std::advance(it11, 3);
        
        
        auto it1 = std::partition(ls1.begin(), ls1.end(), [](int e){ return e % 2 == 0;});
        std::cout << "ls1 " << ls1 << std::endl;

        auto it2 = std::partition(ls1.begin(), ls1.end(), [](int e){ return e > 4;});
        std::cout << "ls1 " << ls1 << std::endl;
       // std::cout << "pivot" << *it2;
    }
    
    template<typename T>
    std::list<T> parallel_quicksort1(std::list<T> input)
    {
        if (input.empty()) {
            return input;
        }
        
        // choose the pivot
        std::list<T> result;
        result.splice(result.begin(), input, input.begin());
        const T& pivot_value = *result.begin();
        
        // divide to 2 parts
        typename std::list<T>::iterator pivot_it = std::partition(input.begin(), input.end(), [&](const T& t){return t < pivot_value;});
        
        // dump
        std::cout << "round: " << input << std::endl;
        
        // get the lower parts
        std::list<T> lower_part;
        lower_part.splice(lower_part.begin(), input, input.begin(), pivot_it);
        
        // recursively sort the lower and higher part.
        std::list<T> lower = parallel_quicksort1(lower_part);
        std::list<T> higher = parallel_quicksort1(input);
        
        // compose the result.
        result.splice(result.begin(), lower);
        result.splice(result.end(), higher);
        
        return result;
    }
    
    template<typename F, typename A>
    std::future< typename std::result_of<F(A&&)>::type> spawn_task(F&& func, A&& arg)
    {
        typedef typename std::result_of<F(A&&)>::type result_type;
        std::packaged_task<result_type(A&&)> task(std::move(func));
        std::future<result_type> fut = task.get_future();
        std::thread(std::move(task), std::move(arg)).detach();
        
        return fut;
    }
    
    std::mutex g_mtx;
    
    template<typename T>
    std::list<T> parallel_quicksort2(std::list<T> input)
    {
        if (input.empty()) {
            return input;
        }
        
        std::list<T> result;

        // choose pivot
        result.splice(result.begin(), input, input.begin());
        const T& pivot_value = *result.begin();
        
        // divid to two parts
        typename std::list<T>::iterator pivot_it = std::partition(input.begin(), input.end(), [&](const T& t){ return t < pivot_value;});
        
        g_mtx.lock();
        std::cout << "round:" << std::this_thread::get_id() << " " << input << std::endl;
        g_mtx.unlock();
        
        // sort lower
        std::list<T> lower_parts;
        lower_parts.splice(lower_parts.begin(), input, input.begin(), pivot_it);
        std::future<std::list<T> > lower = spawn_task(&parallel_quicksort2<T>, std::move(lower_parts)); // using a new thread
        
        // sort higher
        std::list<T> higher = parallel_quicksort2<T>(input); // using current thread
        
        // combine result
        result.splice(result.end(), higher);
        result.splice(result.begin(), lower.get());
        
        return result;
    }
    
    template<typename T>
    struct ThreadSafeStack{
    public:
        ThreadSafeStack(){}
        virtual ~ThreadSafeStack(){}
        
        ThreadSafeStack(const ThreadSafeStack& rhs)
        {
            std::lock_guard<std::mutex> lk(mMutex);
            mData = rhs.mData;
        }
        
        ThreadSafeStack& operator=(const ThreadSafeStack&) = delete;
        
        void push(T t)
        {
            std::lock_guard<std::mutex> lk(mMutex);
            mData.push(t);
        }
        
        void pop(T& t)
        {
            std::lock_guard<std::mutex> lk(mMutex);
            t = mData.top();
            mData.pop();
        }
        
        std::shared_ptr<T> pop()
        {
            std::lock_guard<std::mutex> lk(mMutex);
            if (mData.empty()) {
                return nullptr;
            }
            
            std::shared_ptr<T> res(new T(mData.top()));
            mData.pop();
            return res;
        }
        
        bool empty()
        {
            std::lock_guard<std::mutex> lk(mMutex);
            return mData.empty();
        }
        
    public:
        std::stack<T> mData;
        mutable std::mutex mMutex;
    };
    
    template<typename T>
    struct ParallelQuickSorter
    {
    public:
        
        struct ChunkToSort{
            std::list<T> data;
            std::promise<std::list<T> > promise;
            bool mDone;
            
            ChunkToSort(bool done = false) : mDone(done){}
            ChunkToSort(const ChunkToSort& rhs)
            : mDone(rhs.mDone), data(std::move(rhs.data))
            {
            }
        };
        
        ParallelQuickSorter()
        : mMaxThreads(std::thread::hardware_concurrency())
        {
            for (int i = 0; i < mMaxThreads - 1; i++) {
                mThreads.push_back(std::thread(&ParallelQuickSorter<T>::sort_thread, this));
            }
        }
        
        virtual ~ParallelQuickSorter()
        {
            for(unsigned i=0;i<mMaxThreads;++i)
            {
                mChunks.push(new ChunkToSort(true));
            }
            
            for (auto& e : mThreads){
                e.join();
            }
        }
        
        std::list<T> do_sort(std::list<T> input)
        {
            if (input.empty()) {
                return input;
            }
            
            std::list<T> result;

            // choose pivot
            result.splice(result.begin(), input, input.begin());
            const T& pivot_value = *result.begin();
            
            // divide
            typename std::list<T>::iterator pivot_it = std::partition(input.begin(), input.end(), [&](const T& t){ return t < pivot_value;});
            
            // prepare and push data to stack
            ChunkToSort* newChunk = new ChunkToSort();
            newChunk->data.splice(newChunk->data.begin(), input, input.begin(), pivot_it);
            std::future<std::list<T> > lower_fut = newChunk->promise.get_future();
            mChunks.push(newChunk);
            
            // sort higher, in current thread
            std::list<T> higher = do_sort(input);
            result.splice(result.end(), higher);
            
            // sort lower, from thread pool.
            std::list<T> lower = lower_fut.get();
            result.splice(result.begin(), lower);
            
            return result;
        }
        
        void sort_thread()
        {
            while (try_sort_chunk()) {
                std::this_thread::yield();
            }
        }
        
        bool try_sort_chunk()
        {
            std::shared_ptr<ChunkToSort*> chunk = mChunks.pop();
            if (chunk) {
                std::list<T> result = do_sort((*chunk)->data);
                (*chunk)->promise.set_value(result);
            }
            return true;
        }
        
    private:
        int mMaxThreads;
        ThreadSafeStack<ChunkToSort*> mChunks;
        std::vector<std::thread> mThreads;
    };
    
    template<typename T>
    std::list<T> parallel_quicksort3(std::list<T> input)
    {
        ParallelQuickSorter<T> s;
        return s.do_sort(input);
    }
    
    
    #define random(x) (rand()%x)
    
    void test_parallel_quicksort()
    {
        //std::list<int> input = { 4,5,3,9,7,5,6,1,2,8 };
        std::list<int> input;
        for (int i = 0; i < 100; i++) {
            input.push_back(random(100));
        }

        std::list<int> output = parallel_quicksort3<int>(input);
        
        std::cout << "output: " << output;
    }
    
    void run()
    {
        //test_copy();
        //test_for_each();
        //test_sort();
        //test_find();
        //test_splice();
        //test_partition();
        test_parallel_quicksort();
    }
}

class JointThreads
{
public:
    explicit JointThreads(std::vector<std::thread>& threads)
    : mThreads(threads)
    {
    }
    
    ~JointThreads()
    {
        for (unsigned long i = 0; i < mThreads.size(); i++) {
            if (mThreads[i].joinable()) {
                mThreads[i].join();
            }
        }
    }
    
private:
    std::vector<std::thread>& mThreads;
    
};

/*
 min, distance,advance, move
*/
namespace T0B {
    template<typename Iterator, typename Func>
    void parallel_for_each(Iterator first, Iterator last, Func f)
    {
        const unsigned long length = std::distance(first, last);
        if (length == 0) {
            return;
        }
        
        const unsigned long min_per_thread = 25;
        const unsigned long max_threads = (length +  min_per_thread - 1) / min_per_thread;
        const unsigned long hardware_threads = std::thread::hardware_concurrency();
        const unsigned long num_threads = std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
        const unsigned long block_size = length / num_threads;
        
        std::vector<std::thread> threads(num_threads - 1);
        JointThreads joiner(threads);
        
        Iterator block_first = first;
        for (unsigned long i = 0; i < num_threads - 1; i++) {
            Iterator block_end = block_first;
            std::advance(block_end, block_size);
            
            // Using task
            std::packaged_task<void()> task([=](){
                std::for_each(block_first, block_end, f);
            });
            //threads[i] = std::thread(std::move(task));
            
            // Using thread directly.
            threads[i] = std::thread([=](){
                std::for_each(block_first, block_end, f);
            });
            
            block_first = block_end;
        }
        
        std::for_each(block_first,last,f);
        
    }
    
    void run()
    {
        std::vector<int> data;
        for (int i = 1; i <= 1000; i++) {
            data.push_back(i);
        }
        
        parallel_for_each(data.begin(), data.end(), [](int& e){
            e += 2;
            
            int res = 0;
            for (int i = 0; i < 50000; i++) {
                for (int j = 0; j < 20000; j++) {
                    res = i - j;
                }
                
                
                if ( i % 10000 == 0) {
                    std::cout << std::this_thread::get_id() << std::endl;
                }
            }
        });

        std::for_each(data.begin(), data.end(), [](int i){std::cout << i << " ";});
        
    }
}

// accumulate
namespace T0C {
    
    template<typename Iterator, typename T>
    T parallel_accumulate(Iterator first, Iterator last, T init)
    {
        const unsigned long length = std::distance(first, last);
        if (length == 0) {
            return init;
        }
        
        const unsigned long min_per_thread = 25;
        const unsigned long max_threads_num = (length + min_per_thread - 1) / min_per_thread;
        const unsigned long hardware_threads = std::thread::hardware_concurrency();
        const unsigned long threads_num = std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads_num);
        const unsigned long block_size = length / threads_num;
        
        std::vector<std::thread> threads(threads_num - 1);
        std::vector<T> results(threads_num);
        
        Iterator block_start = first;
        for (int i = 0; i < threads_num-1; i++) {
            Iterator block_end = block_start;
            std::advance(block_end, block_size);
            auto task = [=](Iterator first, Iterator last, T& result){ result = std::accumulate(first, last, result);};
            threads[i] = std::thread(task, block_start, block_end, std::ref(results[i]));
            block_start = block_end;
        }
        results[threads_num - 1] = std::accumulate(block_start, last, 0);
    
        for (int i = 0; i < threads_num-1; i++) {
            threads[i].join();
        }
        
        return std::accumulate(results.begin(), results.end(), init);
    }
    
    void run()
    {
        std::vector<int> data;
        for (int i = 1; i <= 100000; i++) {
            data.push_back(i);
        }
        
        std::cout << "Single threads results: " << std::accumulate(data.begin(), data.end(), 0) << std::endl;
        
        int r = parallel_accumulate(data.begin(), data.end(), 0);
        std::cout << "Multi threads results: " << r << std::endl;
    }
    
}
