#include "functions.h"
#include <ncurses.h>
#include <iostream>
#include <vector>


std::vector<int> read_args(const std::string& func_name, int n = -1){

    if(n < 0){
        std::cout << "Enter number of arguments: ";
        std::cin >> n;
    }
    std::vector<int> result(n);
    std::cout << "Enter " << n << " arguments of function " << func_name << ": ";
    for(int i = 0; i < n; i++){
        std::cin >> result[i];
    }
    return result;
}

int main(){
    std::cout << "Enter 1 to enter demo mode. Enter 2 to enter testing mode. Enter anyting else to quit." << std::endl;
    char choice;
    std::cin >> choice;
    std::vector<int> f_args, g_args;
    int N = 6;

    if(choice == '1'){
        f_args = {0,1,2,3,4,5};
        g_args = {0,1,2,3,4,5};
        for(int i = 0; i < N; i++){
            main_demo(f_args[i], g_args[i]);
        }
    } else if(choice == '2') {
        f_args = read_args("F");
        N = f_args.size();
        g_args = read_args("G", N);
        for(int i = 0; i < N; i++){
            main_test(f_args[i], g_args[i]);
        }
    } else {
        std::cout << "Good bye!" << std::endl;;
        return 0;
    }

    return 0;
}
