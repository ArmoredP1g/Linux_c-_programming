#include "ThreadPool.h"
#include <unistd.h>
#include <iostream>
using namespace std;

void taskFunc(int* arg){
    int num = *(int*)arg;
    cout << "thread " << this_thread::get_id() <<" is working, num is: "<<num<<endl;
    sleep(1);
}

int main(){
    //创建线程池
    ThreadPool pool(3, 10);
    for (int i=0; i < 100; i++)
    {
        int* num = new int(i+100);
        pool.addTask(Task(taskFunc, num));
    }

    sleep(20);
    return 0;
}

