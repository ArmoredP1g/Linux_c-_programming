#pragma once
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>

using namespace std;
using callback = void (*)(void* arg); //c++ 11特性

//任务对象
typedef struct Task
{
    //void (*function)(void* arg);
    callback function;
    void* arg;


    Task()
    {
        function = nullptr;
        arg = nullptr;
    }

    Task(callback f,void* arg)
    {
        this->arg = arg;
        this->function = f;
    }
}Task;


//任务队列
class TaskQueue{

public:
    TaskQueue();
    ~TaskQueue();

    void AddTask(Task task);
    void AddTask(callback f, void* arg);
    Task TakeTask();
    inline int taskNumber(){
        //共功能简单，没分支判断，用内联提高效率
        return m_taskQ.size();
    }

private:
    mutex mtx;
    queue<Task> m_taskQ;
};

//线程池
class ThreadPool
{
public:
    ThreadPool(int min, int max);
    ~ThreadPool();

    // 添加任务
    void addTask(Task task);
    // 获取忙线程的个数
    int getBusyNumber();
    // 获取活着的线程个数
    int getAliveNumber();

private:
    // 工作的线程的任务函数
    void* worker(void* arg);
    // 管理者线程的任务函数
    void* manager(void* arg);
    //单个线程退出
    void threadExit();

private:
    TaskQueue* taskQ;

    thread managerID;           //管理者线程id
    thread* threadIDs;          //工作线程指针的数组
    bool* thread_avaliable;     //当前线程是否在运行
    int minNum;                     //最小线程数量
    int maxNum;                     //做大线程数量
    int busyNum;                    //忙线程个数
    int liveNum;                    //存活线程个数
    int exitNum;                    //待销毁线程个数

    mutex mtxPool;                  //全局锁
    condition_variable_any notEmpty; //全局条件变量

    bool shutdown;
};