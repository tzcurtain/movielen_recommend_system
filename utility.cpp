//
// Created by 周柯辰 on 10/06/2018.
//

#include "utility.h"
#include <string>
#include <sstream>
int getID(const std::string& str){
    int len = (int)str.length();
    int base = 1,res = 0;
    for (int i=len-1;i>=0;--i){
        if (!isdigit(str[i])){
            return -1;
        }
        res += base * (str[i] - '0');
        base *= 10;
    }
    return res;
}

double getRate(const std::string& str){
    std::stringstream ss(str);
    double res;
    ss >> res;
    return res;
}
