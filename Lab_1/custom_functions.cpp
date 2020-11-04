#include "custom_functions.h"

bool my_F(int x){
    if(x <= 0 || x % 2 == 0){
        std::this_thread::sleep_for(std::chrono::seconds(25));
        return false;
    } else {
        std::this_thread::sleep_for(std::chrono::seconds(x%4));
        return (x % 4 == 1);
    }
}

bool my_G(int x){
    if(x <= 0 || x % 3 == 0){
        std::this_thread::sleep_for(std::chrono::seconds(25));
        return false;
    } else {
        std::this_thread::sleep_for(std::chrono::seconds(2*x%3));
        return ((x % 3) % 2 == 1);
    }
}