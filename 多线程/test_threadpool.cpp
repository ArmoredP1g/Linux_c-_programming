#include "ThreadPool.h"
#include <unistd.h>
#include <iostream>
using namespace std;

void taskFunc(int* arg){
    int num = *(int*)arg;
    cout << "num is: "<<num<<endl;
    sleep(1);
}

int main(){
    //创建线程池
    ThreadPool pool(3, 10);
    for (int i=0; i < 100; i++)
    {
        int* num = new int(i);
        pool.addTask(Task(taskFunc, num));
    }

    sleep(24);

    for (int i=100; i < 200; i++)
    {
        int* num = new int(i);
        pool.addTask(Task(taskFunc, num));
    }

    sleep(24);

    for (int i=200; i < 300; i++)
    {
        int* num = new int(i);
        pool.addTask(Task(taskFunc, num));
    }

    sleep(24);

    for (int i=300; i < 400; i++)
    {
        int* num = new int(i);
        pool.addTask(Task(taskFunc, num));
    }

    sleep(200);
    return 0;
}

