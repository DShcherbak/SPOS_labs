#include <iostream>
#include <thread>
#include <future>
#include <termios.h>
#include <unistd.h>
#include "cmake-build-debug/demofuncs"

using spos::lab1::demo::f_func;
using spos::lab1::demo::g_func;
const spos::lab1::demo::op_group AND = spos::lab1::demo::AND;

std::condition_variable f_var, g_var; std::mutex m;
bool ready, processed;
std::string data;

int somefunc(){

}

void f_function(){
    std::unique_lock<std::mutex> lk(m);
    f_var.wait(lk, []{return ready;});// after main has sent data, we own the lock.

    std::cout << "Worker thread is processing data\n";
    data += " after processing";

    processed = true;
    std::cout << "Worker thread signals data processing completed\n";
    lk.unlock();
    f_var.notify_one(); // Send data back to main()
}



int main_thread(){

    std::thread f(f_function);

// send data to the worker thread
    {
        std::cout << "main() start\n";
        std::lock_guard<std::mutex> f_lk(m);
        ready = true;
        std::cout << "main() signals data ready for processing\n";

    }
// wait for the worker
    {
        std::unique_lock<std::mutex> f_lk(m);
        f_var.wait(f_lk, []{return processed;});
    }


    std::cout << "Back in main(), data = " << data << '\n';
    f.join();

}


int main(){
    main_thread();
    return 0;
}
