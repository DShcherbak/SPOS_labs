#include <iostream>
#include <thread>
#include <future>
#include <termios.h>
#include <unistd.h>
#include <vector>
#include "cmake-build-debug/demofuncs"

using spos::lab1::demo::f_func;
using spos::lab1::demo::g_func;
const spos::lab1::demo::op_group AND = spos::lab1::demo::AND;

std::condition_variable condvar;
std::mutex m, mf, mg;
bool f_ready, g_ready, f_processed, g_processed;
bool f_result, g_result;
std::string data;


void f_function(int x){
    std::unique_lock<std::mutex> lock(mf);
    condvar.wait(lock, []{return f_ready;});// after main has sent data, we own the lock.

    std::this_thread::sleep_for(std::chrono::seconds(6));
    std::cout << "f thread is processing data" << std::endl;
    data += " f after processing";
    f_ready = false;
    f_processed = true;
    f_result = f_func<AND>(x);
    std::cout << "F thread signals data processing completed" << std::endl;
    lock.unlock();
    condvar.notify_one(); // Send data back to main()
}

void g_function(int x){
    std::unique_lock<std::mutex> lock(mg);
    condvar.wait(lock, []{return g_ready;});// after main has sent data, we own the lock.

    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "g thread is processing data" << std::endl;;
    data += " g after processing";
    g_ready = false;
    g_processed = true;
    g_result = g_func<AND>(x);
    std::cout << "G thread signals data processing completed" << std::endl;;
    lock.unlock();
    condvar.notify_one(); // Send data back to main()
}

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

int main_thread(int fx, int gx){

    std::cout << "Testing f(" << fx << ") AND g(" << gx << ");" << std::endl;
    std::thread f(f_function, fx);
    std::thread g(g_function, gx);

// send data to the worker thread
    {
        std::cout << "main-f start" << std::endl;;
        std::lock_guard<std::mutex> lk(mf);
        f_ready = true;
        std::cout << "main-f signals data ready for processing" << std::endl;;
        condvar.notify_one();

    }
    {
        std::cout << "main-g start" << std::endl;;
        std::lock_guard<std::mutex> lk(mg);
        g_ready = true;
        std::cout << "main-g signals data ready for processing" << std::endl;;
        condvar.notify_one();
    }
    std::unique_lock<std::mutex> lk(m);
    while(!f_processed && !g_processed){
        condvar.wait_for(lk, std::chrono::seconds(2));
        std::cout << "Waiting..."  << std::endl;
    }
    lk.unlock();
    if(!g_processed) {
        std::cout << "F(" << fx << ") = " << f_result << std::endl;
        {
            if(!g_processed){
                std::unique_lock<std::mutex> lk_1(mg);
                condvar.wait_for(lk_1, std::chrono::seconds(5), []{return g_processed;});
            }
            if(!g_processed) {
                std::cout << "I believe g hangs..." << std::endl;
                return 0;
            }
            else {
                std::cout << "G(" << gx << ") = " << g_result << std::endl;
                std::cout << "RESULT: F(" << fx << ") AND G(" << gx << ") = " << (f_result & g_result) << std::endl;
            }
        }
    }
    else{
        std::cout << "G(" << gx << ") = " << g_result << std::endl;
        {
            if(!f_processed){
                std::unique_lock<std::mutex> lk_1(mf);
                condvar.wait_for(lk_1, std::chrono::seconds(5), []{return f_processed;});
            }
            if(!f_processed) {
                std::cout << "I believe f hangs..." << std::endl;
                return 0;
            }
            else {
                std::cout << "F(" << fx << ") = " << f_result << std::endl;
                std::cout << "RESULT: F(" << fx << ") AND G(" << gx << ") = " << (f_result & g_result) << std::endl;
            }
        }
    }
    /*if(!f_processed){
        std::cout << "G(" << gx << ") = " << g_processed;
        {
            std::unique_lock<std::mutex> lk(m);
            condvar.wait_for(lk, std::chrono::seconds(5), []{return g_processed;});
            lk.unlock();
        }
    }
    else{
        std::cout << "F(" << fx << ") = " << f_processed;
        {
            std::unique_lock<std::mutex> lk(m);
            condvar.wait_for(lk, std::chrono::seconds(5), []{return g_processed;});
            lk.unlock();
        }*/



    //lk.unlock();
    std::cout << "Back in main(), data = " << data << '\n';
    f.join();
    g.join();
    std::cout << "Back in main(), data = " << data << '\n';
}


int main(){
    std::cout << "Enter 1 to enter demo mode. Enter 2 to enter testing mode. Enter anyting else to quit." << std::endl;
    char choice;
    std::cin >> choice;
    std::vector<int> f_args, g_args;
    int N = 5;
    if(choice == '1'){
        f_args = {1,2,3,4,5};
        g_args = {1,2,3,4,5};
    } else if(choice == '2') {
        f_args = read_args("F");
        N = f_args.size();
        g_args = read_args("F", N);
    } else {
        std::cout << "Good bye!" << std::endl;;
        return 0;
    }
    for(int i = 0; i < N; i++){
        main_thread(f_args[i], g_args[i]);
    }
    return 0;
}
