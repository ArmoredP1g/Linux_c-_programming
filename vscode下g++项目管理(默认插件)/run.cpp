#include<iostream>
#include<module1.h>
#include<module2.h>
using namespace std;

int main(){
    int a;
    while(cin >> a){
        if (a =='\n')
            break;
        a = mul(a,a);
        if(isOdd(a)){
            cout<< a << "是奇数"<<endl;
        }
        else{
            cout<< a << "是偶数"<<endl;
        }

    }
    
    return 0;
}
