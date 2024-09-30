#include<vector>
#include<iostream>
using namespace std ;
int main(){
    vector<int> values = {1,2,3,5,6,7,8};
    values.resize(4);
    for(auto val : values)
        std::cout<<val<<" ";
}