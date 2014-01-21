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
#include <cmath>
#include <functional>
#include <algorithm>
#include <vector>
#include <numeric>
#include <ostream>
#include <queue>

template<typename T>
class queue
{
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;
public:
    queue()
    {}
    queue(queue const& other)
    {
        std::lock_guard<std::mutex> lk(other.mut);
        
        data_queue=other.data_queue;
    }
    void push(T data)
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(data);
        data_cond.notify_one();
    }
    void wait_and_pop(T& value)
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk,[&](){return !data_queue.empty();});
        value=data_queue.front();
        data_queue.pop();
    }
    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk,[&](){return !data_queue.empty();});
        std::shared_ptr<T> res(new T(data_queue.front()));
        data_queue.pop();
        return res;
    }
    bool try_pop(T& value)
    {
        std::lock_guard<std::mutex> lk(mut);
        if(data_queue.empty())
            return false;
        value=data_queue.front();
        data_queue.pop();
        return true;
    }
    std::shared_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lk(mut);
        if(data_queue.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> res(new T(data_queue.front()));
        data_queue.pop();
        return res;
    }
    bool empty() const
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
};

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


// main notify thread
// 1 worker thread
namespace T10 {
    
    void exucute(std::future<std::string>& fut)
    {
        std::cout << fut.get();
    }
    
    void run()
    {
        std::promise<std::string> data;
        std::future<std::string> fut = data.get_future();
        std::thread th(exucute, std::ref(fut));
        data.set_value("ABC");
        th.join();
        
    }
}

// main notify multiple worker threads.
// must use shared_thread
namespace T11 {
    
    void execute(std::shared_future<std::string>& fut)
    {
        std::cout << std::this_thread::get_id(); //<< std::endl;
        std::cout << fut.get();
    }
    
    void run()
    {
        // promise and future
        std::promise<std::string> data;
        std::shared_future<std::string> fut = data.get_future();
        int numThreads = 5;
        std::vector<std::thread> threads(numThreads);
        
        // init threads
        for (int i = 0; i < numThreads; i++) {
            threads[i] = std::thread(execute, std::ref(fut));
        }
        
        // heart beat
        for (int i = 0; i < 50000; i++) {
            for (int j = 0; j < 50000; j++) {
                int x = i - j;
            }
            
            if (i % 10000 == 0) {
                std::cout << i << std::endl;
            }
        }
        
        // data is ready
        data.set_value("A");
        
        // Join threads
        for (int i = 0; i < numThreads; i++) {
            threads[i].join();
        }
        
    }
    
}

// thread notify main, global promise
namespace T12 {
    
    std::promise<std::string> data;

    void execute()
    {
        data.set_value("A");

        // heart beat
        for (int i = 0; i < 50000; i++) {
            for (int j = 0; j < 50000; j++) {
                int x = i - j;
            }
            
            if (i % 10000 == 0) {
                std::cout << i << std::endl;
            }
        }
        
    }
    
    void run()
    {
        std::future<std::string> fut = data.get_future();
        std::thread t(&execute);
        
        std::cout << "before" << std::endl;
        std::cout << fut.get();
        std::cout << "after";
        
        t.join();
    }
}

// thread notify main, promise as Ref parameter.
namespace T13 {
    
    void execute(std::promise<std::string>& data)
    {
        data.set_value("A");
        
        // heart beat
        for (int i = 0; i < 50000; i++) {
            for (int j = 0; j < 50000; j++) {
                int x = i - j;
            }
            
            if (i % 10000 == 0) {
                std::cout << i << std::endl;
            }
        }
        
    }
    void run()
    {
        std::promise<std::string> data;
        std::future<std::string> fut = data.get_future();
        std::thread t(&execute, std::ref(data));
        
        std::cout << "before" << std::endl;
        std::cout << fut.get() << std::endl;
        std::cout << "after";
        
        t.join();
    }

}

// thread notify main. async
namespace T14 {
    
    bool execute(int v)
    {
        // heart beat
        for (int i = 0; i < 50000; i++) {
            for (int j = 0; j < 50000; j++) {
                int x = i - j;
            }
            
            if (i % 10000 == 0) {
                std::cout << i << std::endl;
            }
        }
        
        return true;
    }
    
    void run()
    {
        std::future<bool> fut = std::async(std::launch::async, &execute, 5000);
        std::cout << fut.get();
    }
}

// thread notify main, packaged_task
namespace T15 {
    
    bool execute()
    {
        // heart beat
        for (int i = 0; i < 50000; i++) {
            for (int j = 0; j < 50000; j++) {
                int x = i - j;
            }
            
            if (i % 10000 == 0) {
                std::cout << i << std::endl;
            }
        }
        return true;
    }
    void run()
    {
        std::packaged_task<bool()> task(&execute);
        std::future<bool> fut = task.get_future();
        std::thread t(std::move(task));
        std::cout << fut.get();
        t.join();
    }
}

// single consumer-producer
namespace T16 {
    
    void consumer(std::future<int>& f)
    {
        std::cout << "consumer: " << f.get();
    }
    
    void producer(std::promise<int>& p)
    {
        // heart beat
        for (int i = 0; i < 50000; i++) {
            for (int j = 0; j < 50000; j++) {
                int x = i - j;
            }
            
            if (i % 10000 == 0) {
                std::cout << i << std::endl;
            }
        }
        
        p.set_value(10);
    }
    
    void run()
    {
        std::promise<int> p;
        std::future<int> f = p.get_future();
        
        std::thread t1(&producer, std::ref(p));
        std::thread t2(&consumer, std::ref(f));
        
        t1.join();
        t2.join();
    }
}

namespace T17 {
    
    class ThreadPool
    {
    public:
        ThreadPool()
        : mThreadJoiner(mThreads), mDone(false)
        {
            // max threads number
            const unsigned numThreads = std::thread::hardware_concurrency();
            
            // init threads
            for (unsigned i = 0; i < numThreads; i++) {
                mThreads.push_back(std::thread(&ThreadPool::run, this));
            }
            
        }
        
        ~ThreadPool()
        {
            mDone = true;
        }
        
        template <typename Func>
        void submit(Func f)
        {
            mTasks.push(f);
        }
        
        
    private:
        void run()
        {
            while (!mDone) {
                
                std::lock_guard<std::mutex> lk(mMutex); // for thread-safe.
                
                std::function<void()> task;
                if (mTasks.size() > 0) {
                    
                    // get the task.
                    task = mTasks.front();
                    mTasks.pop();
                    
                    // execute the task
                    task();
                }
                else{
                    std::this_thread::yield();
                }
            }
        }
        
    private:
        std::vector<std::thread> mThreads;
        JointThreads mThreadJoiner;
        std::atomic_bool mDone;
        std::queue<std::function<void()> > mTasks; // TODO: thread-safe

        std::mutex mMutex; // for thread-safe
    };
    
    void run()
    {
        // define tasks
        auto f1 = [](){
            std::cout << "begin" << std::this_thread::get_id() << std::endl;
            
            for (int i = 0; i < 50000; i++) {
                for (int j = 0; j < 20000; j++) {
                    int x = i - j;
                }
                
                if (i % 10000 == 0) {
                    std::cout << std::this_thread::get_id() << std::endl;
                }
            }
            
            std::cout << "end" << std::this_thread::get_id() << std::endl;
            
        };
        
        // init thread pool
        ThreadPool pool;
        typedef std::function<void()> TaskType;
        
        // submit task
        for (int i = 0; i < 20; i++) {
            pool.submit<TaskType>(f1);
        }
     
        // main thread continue working
        for (int i = 0; i < 500000; i++) {
            for (int j = 0; j < 500000; j++) {
                int x = i - j;
            }
            
            if (i % 100000 == 0) {
                std::cout << i << std::endl;
            }
        }
    }
}

// Using thread-safe queue to imporve concurency.

namespace T18 {

    class ThreadPool
    {
    public:
        ThreadPool()
        : mThreadJoiner(mThreads), mDone(false)
        {
            // max threads number
            const unsigned numThreads = std::thread::hardware_concurrency();
            
            // init threads
            for (unsigned i = 0; i < numThreads; i++) {
                mThreads.push_back(std::thread(&ThreadPool::run, this));
            }
            
        }
        
        ~ThreadPool()
        {
            mDone = true;
        }
        
        template <typename Func>
        void submit(Func f)
        {
            mTasks.push(f);
        }
        
        
    private:
        void run()
        {
            while (!mDone) {
                
                std::function<void()> task;
                if (mTasks.try_pop(task)) {
                    
                    // execute the task
                    task();
                }
                else{
                    std::this_thread::yield();
                }
            }
        }
        
    private:
        std::vector<std::thread> mThreads;
        JointThreads mThreadJoiner;
        std::atomic_bool mDone;
        queue<std::function<void()> > mTasks;
        
    };
    
    void run()
    {
        // define tasks
        auto f1 = [](){
            std::cout << "begin" << std::this_thread::get_id() << std::endl;
            
            for (int i = 0; i < 50000; i++) {
                for (int j = 0; j < 50000; j++) {
                    int x = i - j;
                }
                
                if (i % 10000 == 0) {
                    std::cout << std::this_thread::get_id() << std::endl;
                }
            }
            
            std::cout << "end" << std::this_thread::get_id() << std::endl;
            
        };
        
        ThreadPool pool;
        typedef std::function<void()> TaskType;
        
        for (int i = 0; i < 1000; i++) {
            pool.submit<TaskType>(f1);
        }
    }
}

namespace T19 {
    
    std::condition_variable cv;
    std::mutex mtx;
    std::atomic_bool b_ready(false);
    
    void consumer()
    {
        std::unique_lock<std::mutex> lk(mtx);
        while (!b_ready) {
            std::cout << "C";
            cv.wait(lk);
        }
        std::cout << "Done";
    }
    
    void producer()
    {
        // long time work.
        for (int i = 0; i < 90000; i++) {
            for (int j = 0; j < 40000; j++) {
                int x = i - j;
            }
            if (i % 10000 == 0) {
                std::cout << "P";
            }
        }
        
        b_ready = true;
        cv.notify_all();
    }
    
    void run()
    {
        std::thread tc1(&consumer);
        std::thread tp(&producer);
        std::thread tc2(&consumer);
        std::thread tc3(&consumer);
        std::thread tc4(&consumer);
        std::thread tc5(&consumer);

        tc1.join();
        tc2.join();
        tc3.join();
        tc4.join();
        tc5.join();
        tp.join();
        
    }
}

namespace T1A {
    
    std::condition_variable cv;
    std::mutex mtx;
    bool b_ready = false;
    
    void consumer()
    {
        std::unique_lock<std::mutex> lk(mtx);
        std::cout << "C";
        cv.wait(lk, [](){return b_ready;});
        std::cout << "Done";
    }
    
    void producer()
    {
        // long time work.
        for (int i = 0; i < 50000; i++) {
            for (int j = 0; j < 40000; j++) {
                int x = i - j;
            }
            if (i % 10000 == 0) {
                std::cout << "P";
            }
        }
        
        b_ready = true;
        cv.notify_one();  // Problemlic, there are 4 threads that can't come to end. App hangs.
    }
    
    void run()
    {
        std::thread tp(&producer);
        std::thread tc1(&consumer);
        std::thread tc2(&consumer);
        std::thread tc3(&consumer);
        std::thread tc4(&consumer);
        std::thread tc5(&consumer);
        
        tc1.join();
        tc2.join();
        tc3.join();
        tc4.join();
        tc5.join();
        tp.join();
        
    }
}

namespace T1B {
    
    std::condition_variable cv;
    std::mutex mtx;
    bool b_ready = false;
    
    void consumer()
    {
        std::unique_lock<std::mutex> lk(mtx);
        std::cout << "C";
        cv.wait_for(lk, std::chrono::seconds(10),[](){return b_ready;}); // Though producer only notify one, but here the others thread will wait at least for 10 seconds.
        std::cout << "Done";
    }
    
    void producer()
    {
        // long time work.
        for (int i = 0; i < 50000; i++) {
            for (int j = 0; j < 40000; j++) {
                int x = i - j;
            }
            if (i % 10000 == 0) {
                std::cout << "P";
            }
        }
        
        b_ready = true;
        cv.notify_one();
    }
    
    void run()
    {
        std::thread tp(&producer);
        std::thread tc1(&consumer);
        std::thread tc2(&consumer);
        std::thread tc3(&consumer);
        std::thread tc4(&consumer);
        std::thread tc5(&consumer);
        
        tc1.join();
        tc2.join();
        tc3.join();
        tc4.join();
        tc5.join();
        tp.join();
        
    }
}

namespace T1C {
    
    template <typename _Tp>
    class ThreadSafeQueue
    {
    public:
        ThreadSafeQueue()
        {
            
        }
        
        virtual ~ThreadSafeQueue(){}
        
        void push(const _Tp& e)
        {
            std::lock_guard<std::mutex> lk(mMutex);
            mQueue.push(e);
            mCV.notify_one();
        }
        
        bool waitAndPop(_Tp& e)
        {
            std::unique_lock<std::mutex> lk(mMutex);
            mCV.wait(lk, [&](){return !mQueue.empty();});
            e = mQueue.front();
            mQueue.pop();
            return true;
        }
        
        std::shared_ptr<_Tp> pop()
        {
            std::lock_guard<std::mutex> lk(mMutex);
            std::shared_ptr<_Tp> ret(new _Tp(mQueue.front()));
            mQueue.pop();
            return ret;
        }
        
    private:
        std::queue<_Tp> mQueue;
        std::mutex mMutex;
        std::condition_variable mCV;
    };
    
    ThreadSafeQueue<int> g_queue;
    std::mutex g_mutex;
    
    void producer()
    {
        for (int i = 0; i < 100; i++) {
            for (int x = 0; x < 10000; x++) {
                for (int y = 0; y < 20000; y++) {
                    int z = x - y;
                }
            }
            g_queue.push(i);
        }
    }
    
    void consumer(char c)
    {
        for (int z = 0; z < 20; z++)
        {
            for (int i = 0; i < 20000; i++) {
                for (int j = 0; j < 20000; j++) {
                    int k = i -j;
                }
            }
            int e = 0;
            g_queue.waitAndPop(e);
            
            {
                std::lock_guard<std::mutex> lk(g_mutex);
                std::cout << c << " " << e << std::endl;
            }
        }
    }
    
    void run()
    {
        std::thread tp1(&producer);
        std::thread tp2(&producer);
        std::thread tp3(&producer);
        std::thread tp4(&producer);
        std::thread tp5(&producer);
        
        std::thread tc1(&consumer,'A');
        std::thread tc2(&consumer,'B');
        std::thread tc3(&consumer,'C');
        std::thread tc4(&consumer,'D');
        std::thread tc5(&consumer,'E');
        std::thread tc6(&consumer,'F');
        std::thread tc7(&consumer,'F');
        std::thread tc8(&consumer,'F');

        tp1.join();
        tp2.join();
        tp3.join();
        tp4.join();
        tp5.join();
        
        tc1.join();
        tc2.join();
        tc3.join();
        tc4.join();
        tc5.join();
        tc6.join();
        tc7.join();
        tc8.join();

    }
}

namespace T1D {
    
    std::mutex g_mtx;
    int nJobShared = 0;
    int nExcluded = 0;
    
    void job1()
    {
        while (true) {
            
            if (g_mtx.try_lock()) {
                std::cout << "try_lock() " << nJobShared << std::endl;
                g_mtx.unlock();
            }
            else{
                std::cout << "not locked" << nExcluded++ << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    }
    
    void job2()
    {
        g_mtx.lock();
        std::this_thread::sleep_for(std::chrono::seconds(5));
        ++nJobShared;
        g_mtx.unlock();
    }
    
    void run()
    {
        std::thread t1(&job2);
        std::thread t2(&job1);
        
        t1.join();
        t2.join();
    }
}

namespace T1E {
    
    std::ostream& operator<<(std::ostream& ostr, const std::list<int>& ls)
    {
        for (auto& e : ls){
            ostr << " " << e;
        }
        
        return ostr;
    }
    
    //template<typename T>
    std::list<int> parallel_quicksort(const std::list<int>& input)
    {
        if (input.empty()) {
            return input;
        }
        
        std::list<int> result;
        auto it = result.begin();
//        input.splice(it, input);
//        
//        const int& pivot_value = *result.begin();
//
//        typename std::list<T>::iterator pivot_iter = std::partition(input.begin(), input.end(), [&](const T& t){ return t < pivot_value;});
//        
//        std::list<T> lower_parts;
//        lower_parts.splice(lower_parts.begin(), input, input.begin(), pivot_iter);
//        
//        std::list<T> lower = parallel_quicksort(lower_parts);
//        std::list<T> higher = parallel_quicksort(input);
//        
//        result.splice(result.begin(), lower);
//        result.splice(result.end(), higher);
        return result;
    }
    
    void run(){
        std::list<int> input = {4,5,3,9,7,5,6,1,2,8};
        std::list<int> output = parallel_quicksort(input);
        
        std::cout << "output: " << output;
        
    }
}

