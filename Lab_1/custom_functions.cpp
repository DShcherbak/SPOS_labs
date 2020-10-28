#include "custom_functions.h"

bool my_F(int x){
    if(x < 0){
        std::this_thread::sleep_for(std::chrono::seconds(25));
    } else {
        std::this_thread::sleep_for(std::chrono::seconds(x%5));
    }
}

bool my_G(int x){
    if(x < 0){
        std::this_thread::sleep_for(std::chrono::seconds(25));
    } else {
        std::this_thread::sleep_for(std::chrono::seconds(x+3%5));
    }
}