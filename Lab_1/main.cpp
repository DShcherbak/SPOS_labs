#include <iostream>
#include <thread>
#include <future>
#include <termios.h>
#include <unistd.h>
#include "cmake-build-debug/demofuncs"

using spos::lab1::demo::f_func;
using spos::lab1::demo::g_func;
const spos::lab1::demo::op_group AND = spos::lab1::demo::AND;

std::condition_variable f_var, g_var;
std::mutex m_f, m_g;
bool ready, processed;
std::string data;

int somefunc(){

}

void f_function(){
    std::unique_lock<std::mutex> lk_f(m_f);
    f_var.wait(lk_f, []{return ready;});// after main has sent data, we own the lock.

    std::cout << "Worker thread is processing data\n";
    data += " f after processing";

    processed = true;
    std::cout << "Worker thread signals data processing completed\n";
    lk_f.unlock();
    f_var.notify_one(); // Send data back to main()
}

void g_function(){
    std::unique_lock<std::mutex> lk_g(m_g);
    g_var.wait(lk_g, []{return ready;});// after main has sent data, we own the lock.

    std::cout << "Worker thread is processing data\n";
    data += " g after processing";

    processed = true;
    std::cout << "Worker thread signals data processing completed\n";
    lk_g.unlock();
    g_var.notify_one(); // Send data back to main()
}





int main_thread(){

    std::thread f(f_function);
    std::thread g(g_function);

// send data to the worker thread
    {
        std::cout << "main() start\n";
        std::lock_guard<std::mutex> f_lk(m_f);
        ready = true;
        std::cout << "main() signals data ready for processing\n";

    }
    {
        std::cout << "main() start\n";
        std::lock_guard<std::mutex> g_lk(m_g);
        ready = true;
        std::cout << "main() signals data ready for processing\n";

    }
// wait for the worker
    {
        std::unique_lock<std::mutex> f_lk(m_f);
        f_var.wait(f_lk, []{return processed;});
        f_lk.unlock();
    }

    {
        std::unique_lock<std::mutex> g_lk(m_g);
        f_var.wait(g_lk, []{return processed;});
        g_lk.unlock();
    }

    std::cout << "Back in main(), data = " << data << '\n';
    f.join();
    g.join();
    std::cout << "Back in main(), data = " << data << '\n';
}


int main(){
    main_thread();
    return 0;
}
