#include <iostream>
#include <unistd.h>
#include <thread>

typedef struct{
    bool flag_stop = false;
    int sec = 0;
    void run();
}Counter;

typedef struct{
    bool flag_stop = false;
    void run();
}Murmer;

void Counter::run(){
    while(!flag_stop){
        std::cout<<"has been running for: "<<sec<<" secs"<<std::endl;
        sec++;
        sleep(1);        
    }
}

void Murmer::run(){
    while (!flag_stop)
    {
        std::cout<<"有人要鲨我"<<std::endl;
        sleep(5);
    }
}

int main(){
    Counter counter;
    Murmer murmer;

    //调用类的成员函数需要传递类的一个对象作为参数(即成员函数的隐藏参数，如果在某一成员
    //内创建线程则传入this即可)
    std::thread th_counter(&Counter::run,counter);
    std::thread th_murmer(&Murmer::run,murmer);

    th_counter.join();
    th_murmer.join();
    return 0;
}