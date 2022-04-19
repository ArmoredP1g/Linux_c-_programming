#include <ThreadPool.h>
#include <iostream>
#include <string.h>
#include <string>

using namespace std;
TaskQueue::TaskQueue()
{

}

void TaskQueue::AddTask(Task task)
{
    this->mtx.lock();
    this->m_taskQ.push(task);
    this->mtx.unlock();
}

void TaskQueue::AddTask(callback f, void* arg)
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
        this->thread_avaliable = new(std::nothrow) bool[max];
        if (threadIDs == nullptr || thread_avaliable == nullptr || taskQ == nullptr){
            cout << "malloc threadIDs or taskQueue failed.." << endl;
            break;
        }
        memset(this->threadIDs, 0, sizeof(thread)*max);
        memset(this->thread_avaliable, false, sizeof(bool)*max);
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

void* ThreadPool::worker(void* arg)
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
            this->thread_avaliable[i];
            cout << "threadExit() called, " << std::this_thread::get_id() <<"exciting..."<<endl;
            break;
        }
    }
    pthread_exit(NULL);
}