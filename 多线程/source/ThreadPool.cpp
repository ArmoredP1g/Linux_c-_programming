#include <ThreadPool.h>
#include <iostream>
#include <string.h>
#include <string>
#include <unistd.h>

using namespace std;
#define NUMBER 2

TaskQueue::TaskQueue()
{

}

TaskQueue::~TaskQueue()
{

}

void TaskQueue::AddTask(Task task)
{
    this->mtx.lock();
    this->m_taskQ.push(task);
    this->mtx.unlock();
}

void TaskQueue::AddTask(callback f, int* arg)
{
    this->mtx.lock();
    this->m_taskQ.push(Task(f,arg));
    this->mtx.unlock();
}

Task TaskQueue::TakeTask()
{
    Task task;

    this->mtx.lock();
    if (!this->m_taskQ.empty())
    {
        task = this->m_taskQ.front();
        m_taskQ.pop();
    }
    this->mtx.unlock();

    return task;
}

//线程池部分
ThreadPool::ThreadPool(int min, int max)
{
    this->taskQ = new(std::nothrow) TaskQueue;
    do
    {
        this->threadIDs = new(std::nothrow) thread[max];
        this->thread_running = new(std::nothrow) bool[max];
        if (threadIDs == nullptr || thread_running == nullptr || taskQ == nullptr){
            cout << "malloc threadIDs or taskQueue failed.." << endl;
            break;
        }
        memset(this->threadIDs, 0, sizeof(thread)*max);
        memset(this->thread_running, false, sizeof(bool)*max);
        this->minNum = min;
        this->maxNum = max;
        this->busyNum = 0;
        this->liveNum = min;    //和最小值相等
        this->exitNum = 0;

        this->shutdown = false;

        //创建管理者线程
        this->managerID = thread(&ThreadPool::manager,this);
        //创建工作线程
        for (int i=0;i<min;i++){
            this->threadIDs[i] = thread(&ThreadPool::worker,this);
        }
        return;

    } while (0);
    
    //若空间未申请成功，释放资源
    if (threadIDs) delete[]threadIDs;
    if (taskQ) delete taskQ;
}

ThreadPool::~ThreadPool(){
    //关闭线程池
    this->shutdown = true;
    //阻塞回收管理者线程
    this->managerID.join();
    //唤醒阻塞的消费者线程
    for (int i=0;i < this->liveNum; i++){
        this->notEmpty.notify_all();
    }

    // 释放堆内存
    if (this->taskQ){
        delete this->taskQ;
    }
    if (this->threadIDs){
        delete[] threadIDs;
    }

}

void* ThreadPool::worker()
{
    //ThreadPool* pool = static_cast<ThreadPool*>(arg);
    //等价于
    //ThreadPool* pool = (ThreadPool*)(arg);

    while(true){
        this->mtxPool.lock();
        //判断队列是否为空
        while (this->taskQ->taskNumber() == 0 && !this->shutdown){//这个while有必要码？
            //阻塞工作线程
            this->notEmpty.wait(this->mtxPool);

            //判断是否需要销毁线程
            if (this->exitNum > 0){
                this->exitNum--;
                if (this->liveNum > this->minNum){
                    this->liveNum--;
                    this->mtxPool.unlock();
                    threadExit();
                }
            }
        }

        //线程池是否被关闭
        if (this->shutdown){
            this->mtxPool.unlock();
            threadExit();
        }

        //从任务队列中取出一个任务
        Task task = this->taskQ->TakeTask();
        //unlock
        this->busyNum++;
        this->mtxPool.unlock();
        cout << "thread " << to_string(pthread_self()) << " start working.." << endl;
        task.function(task.arg);
        delete task.arg;        //由于参数在堆里，fun执行完毕后需要释放内存
        task.arg = nullptr;

        cout << "thread " << to_string(pthread_self()) << " end working.." << endl;
        this->mtxPool.lock();
        this->busyNum--;
        this->mtxPool.unlock();

    }
    return nullptr;

}

void ThreadPool::threadExit()
{
    for (int i=0;i<this->maxNum;i++){
        if (this->threadIDs[i].get_id()==std::this_thread::get_id()){
            this->thread_running[i] = false;
            cout << "threadExit() called, " << std::this_thread::get_id() <<"exciting..."<<endl;
            break;
        }
    }
    pthread_exit(NULL);
}


void* ThreadPool::manager(){
    while(this->shutdown){
        sleep(3);

        //取出线程池中任务的数量和当前线程的数量
        //取出忙线程的数量
        this->mtxPool.lock();
        int queueSize = this->taskQ->taskNumber();
        int liveNum = this->liveNum;
        int busyNum = this->busyNum;
        this->mtxPool.unlock();

        //添加线程
        //任务个数 > 存活的线程 && 存活的线程 < 最大线程数
        if (queueSize > liveNum && liveNum < this->maxNum){
            this->mtxPool.lock();
            int counter = 0;
            for (int i=0; i < this->maxNum && counter < NUMBER//?
                && this->liveNum < this->maxNum; i++)
            {
                if (this->thread_running[i] == 0)
                {
                    this->thread_running[i] = true;
                    this->threadIDs[i] = thread(&ThreadPool::worker,this);
                    counter++;
                    this->liveNum++;
                }
            }
            this->mtxPool.unlock();
        }

        // 销毁线程
        // 忙线程*2 < 存活线程 && 存活线程 > 最小线程
        if (this->busyNum*2 < this->liveNum > this->minNum){
            this->mtxPool.lock();
            this->exitNum = NUMBER;
            this->mtxPool.unlock();
            //让工作线程自杀
            for (int i=0; i < NUMBER; i++){
                this->notEmpty.notify_all();
            }
        }
    }
    return nullptr;
}

void ThreadPool::addTask(Task task){

    if (this->shutdown){
        return;
    }

    //添加任务
    this->taskQ->AddTask(task);
    this->notEmpty.notify_all();
}

int ThreadPool::getBusyNumber(){
    this->mtxPool.lock();
    int busyNum = this->busyNum;
    this->mtxPool.unlock();
    return busyNum;
}

int ThreadPool::getAliveNumber(){
    this->mtxPool.lock();
    int aliveNum = this->liveNum;
    this->mtxPool.unlock();
    return aliveNum;
}