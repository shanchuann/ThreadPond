#include <thread>
#include <iostream>
#include "Logger.hpp"
#include "ScheduledThreadPool.hpp"

#define TIMERQUEUE_TEST 0
#define SCHEDULEDTHREADPOOL_TEST 1
#if SCHEDULEDTHREADPOOL_TEST

void funa()
{
    static int num = 0;
    std::cout << "funa num: " << ++num << std::endl;
}
void funb()
{
    static int num = 0;
    std::cout<<"funb num: "<<++num<<std::endl;
}

int print() 
{
    shanchuan::ScheduledThreadPool mypool;
    std::thread t1([&mypool](){
        auto id1 = mypool.addRunEvery(1000, funa);
        std::this_thread::sleep_for(std::chrono::seconds(20));
        mypool.Cancel(id1);
        std::this_thread::sleep_for(std::chrono::seconds(5));
    });
    std::thread t2([&mypool](){
        auto id2 = mypool.addRunAfter(2000, funb);
        std::this_thread::sleep_for(std::chrono::seconds(10));
        mypool.Cancel(id2);
        std::this_thread::sleep_for(std::chrono::seconds(10));
    });
    std::this_thread::sleep_for(std::chrono::seconds(30));
    t1.join();
    t2.join();
    return 0;
}
int main()
{
    LOG_INFO << "starting scheduled test";
    print();
    return 0;
}
#endif
#if TIMERQUEUE_TEST
#include "TimerQueue.hpp"
#include <iostream>

void func(){
    static int num = 0;
    std::cout<<"func num: "<<++num<<std::endl;
}

int main()
{
    LOG_INFO << "starting scheduled test";
    shanchuan::TimerQueue timerQueue;
    timerQueue.addTimer(func, shanchuan::addmsTime(shanchuan::Timestamp::Now(), 2000), 1000);
    std::this_thread::sleep_for(std::chrono::seconds(10));
    return 0;  
}
#endif