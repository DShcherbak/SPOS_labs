#include "functions.h"
#include "custom_functions.h"
#include "terminal.h"

using spos::lab1::demo::f_func;
using spos::lab1::demo::g_func;
const spos::lab1::demo::op_group AND = spos::lab1::demo::AND;

std::condition_variable cond_f, cond_g, cond_cancel;
std::mutex m, mf, mg, mc;
bool f_ready, g_ready, f_processed, g_processed;
bool f_result, g_result, cancelled, monitoring_in_progress;
std::string data;

void f_function(int x){
    std::unique_lock<std::mutex> lock(mf);
    cond_f.wait(lock, []{return f_ready;});
    lock.unlock();

    f_ready = false;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    f_result =f_func<AND>(x);

    lock.lock();
    f_processed = true;
    lock.unlock();

    cond_f.notify_one();
}

void g_function(int x){
    std::unique_lock<std::mutex> lock(mg);
    cond_g.wait(lock, []{return g_ready;});
    lock.unlock();

    g_ready = false;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    g_result =g_func<AND>(x);

    lock.lock();
    g_processed = true;
    lock.unlock();

    cond_g.notify_one();
}

void start_threads(){
    {
        std::cout << "here" <<std::endl;
        std::lock_guard<std::mutex> lk(mf);
        std::cout << "here" <<std::endl;
        f_ready = true;
        f_processed = false;
        cond_f.notify_one();
    }
    {
        std::cout << "there" <<std::endl;
        std::lock_guard<std::mutex> lk(mg);
        std::cout << "there" <<std::endl;

        g_ready = true;
        g_processed = false;
        cond_g.notify_one();
    }
}

void main_demo(int fx, int gx){

    std::cout << "Testing F(" << fx << ") AND G(" << gx << ");" << std::endl;
    std::thread f(f_function, fx);
    std::thread g(g_function, gx);
    //std::thread monitor(monitoring);

    start_threads();

    const bool debug = false;
    bool f_alive = true, g_alive = true, monitor = true;

    int repeated = 0;


    auto terminal = replace_terminal();

    //std :: cout << "terminal is" << std::endl;

    std::unique_lock<std::mutex> lk_f(mf);
    std::unique_lock<std::mutex> lk_g(mg);
    std::cout << "Press Q to quit." << std::endl;
    while(monitor){//!f_processed && !g_processed && !cancelled
        if(cond_f.wait_for(lk_f, std::chrono::milliseconds(100), []{return f_processed;}))
            break;
        if(cond_g.wait_for(lk_g, std::chrono::milliseconds(100), []{return g_processed;}))
            break;
        char c = getchar();
        if (c == 'q') {
            {
                std::cout << "bye!" << std::endl;
            }
            monitor = false;
        }
        if(repeated % 20 == 0)
            std::cout << "Waiting along..."  << std::endl;
        repeated++;
    }

    lk_f.unlock();
    lk_g.unlock();

    if(!monitor){
        std::cout << "Calculations were canceled!" << std::endl;
        f.detach();
        g.detach();
        g_alive = false;
        f_alive = false;
        return;
    }

    if(!g_processed) {
        std::cout << "F(" << fx << ") = " << f_result << std::endl;
        {
            while(monitor){//!g_processed && !cancelled
                if(cond_g.wait_for(lk_g, std::chrono::milliseconds(100), []{return g_processed;}))
                    break;
                char c = getchar();
                if (c == 'q') {
                    {
                        std::cout << "bye!" << std::endl;
                    }
                    monitor = false;
                }
                if(repeated % 20 == 0)
                    std::cout << "Waiting for g..."  << std::endl;
                repeated++;
            }
            if(!monitor) {
                std::cout << "Calculation of g is cancelled!" << std::endl;
                g.detach();
                g_alive = false;
            }else{
                std::cout << "G(" << gx << ") = " << g_result << std::endl;
                std::cout << "RESULT: F(" << fx << ") AND G(" << gx << ") = " << (f_result & g_result) << std::endl;
            }
        }
    }
    else{
        std::cout << "G(" << gx << ") = " << g_result << std::endl;
        {
            while(monitor){//!g_processed && !cancelled
                if(cond_f.wait_for(lk_f, std::chrono::milliseconds(100), []{return f_processed;}))
                    break;
                char c = getchar();
                if (c == 'q') {
                    {
                        std::cout << "bye!" << std::endl;
                    }
                    monitor = false;
                }
                if(repeated % 20 == 0)
                    std::cout << "Waiting for f..."  << std::endl;
                repeated++;
            }
            if(!monitor) {
                std::cout << "Calculation of f is cancelled!" << std::endl;
                f.detach();
                f_alive = false;
            }else{
                std::cout << "F(" << fx << ") = " << f_result << std::endl;
                std::cout << "RESULT: F(" << fx << ") AND G(" << gx << ") = " << (f_result & g_result) << std::endl;
            }
        }
    }
    set_terminal(terminal);
    std::cout << "terminal is reset!" << std::endl;
    if(f_alive)
        f.join();
    std::cout << "f closed" << std::endl;
    if(g_alive)
        g.join();
    std::cout << "g closed" << std::endl;

    std::cout << "----------------------------------------------------" << std::endl;
    std::cout.flush();
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void main_test(int fx, int gx){

    std::cout << "Testing F(" << fx << ") AND G(" << gx << ");" << std::endl;
    std::thread f(f_function, fx);
    std::thread g(g_function, gx);

    const bool debug = false;
    bool f_alive = true, g_alive = true;

    start_threads();

    std::unique_lock<std::mutex> lk(m);
    std::unique_lock<std::mutex> lk_f(mf);
    std::unique_lock<std::mutex> lk_g(mg);
    int repeated = 0;
    while(true){//!f_processed && !g_processed && !cancelled
        if(cond_f.wait_for(lk_f, std::chrono::milliseconds(100), []{return f_processed;}))
            break;
        if(cond_g.wait_for(lk_g, std::chrono::milliseconds(100), []{return g_processed;}))
            break;

        if(repeated % 20 == 0)
            std::cout << "Waiting along..."  << std::endl;
        repeated++;
    }
    lk_f.unlock();
    lk_g.unlock();

    if(!g_processed) {
        std::cout << "F(" << fx << ") = " << f_result << std::endl;
        {
            if(!g_processed){
                std::unique_lock<std::mutex> lk_1(mg, std::try_to_lock);
                cond_g.wait_for(lk_1, std::chrono::seconds(5), []{return g_processed;});
            }
            if(!g_processed) {
                std::cout << "I believe g hangs..." << std::endl;
                g.detach();
                g_alive = false;
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
                std::unique_lock<std::mutex> lk_1(mf, std::try_to_lock);
                cond_f.wait_for(lk_1, std::chrono::seconds(5), []{return f_processed;});
            }
            if(!f_processed) {
                std::cout << "I believe f hangs..." << std::endl;
                f_alive = false;
                f.detach();
            }
            else {
                std::cout << "F(" << fx << ") = " << f_result << std::endl;
                std::cout << "RESULT: F(" << fx << ") AND G(" << gx << ") = " << (f_result & g_result) << std::endl;
            }
        }
    }

    if(f_alive)
        f.join();
    if(g_alive)
        g.join();
    std::cout << "----------------------------------------------------" << std::endl;
    std::cout.flush();
    std::this_thread::sleep_for(std::chrono::seconds(1));
}



