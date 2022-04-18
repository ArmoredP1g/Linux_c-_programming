/*
调用动态库
*/
#include<iostream>
#include"dyclib/dyclib.h"
using namespace std;


int main(){
    int a=4,b=6;
    cout<<add(a,b);
    return 0;
}
