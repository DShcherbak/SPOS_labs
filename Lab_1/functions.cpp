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
    std::this_thread::sleep_for(std::chrono::seconds(5));
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

    std::unique_lock<std::mutex> lock_1(mg);
    g_processed = true;
    lock_1.unlock();

    cond_g.notify_one();
}

void f_function_custom(int x){
    std::unique_lock<std::mutex> lock(mf);
    cond_f.wait(lock, []{return f_ready;});
    lock.unlock();

    f_ready = false;

    f_result =my_F(x);

    lock.lock();
    f_processed = true;
    lock.unlock();

    cond_f.notify_one();
}

void g_function_custom(int x){
    std::unique_lock<std::mutex> lock(mg);
    cond_g.wait(lock, []{return g_ready;});
    lock.unlock();

    g_ready = false;
    g_result =my_G(x);

    std::unique_lock<std::mutex> lock_1(mg);
    g_processed = true;
    lock_1.unlock();

    cond_g.notify_one();
}

void start_threads(){
    {
        std::lock_guard<std::mutex> lk(mf);
        f_ready = true;
        f_processed = false;
        cond_f.notify_one();

    }
    {
        std::lock_guard<std::mutex> lk(mg);
        g_ready = true;
        g_processed = false;
        cond_g.notify_one();
    }
    {
        std::lock_guard<std::mutex> lk(mc);
        cancelled = false;
        monitoring_in_progress = true;
        cond_cancel.notify_one();
    }
}



void monitoring(){
    {
        std::unique_lock<std::mutex> lock(mc);
        cond_cancel.wait(lock, []{return monitoring_in_progress;});
        lock.unlock();
    }
    bool siin = false;
    if(siin){
        char c;
        while(c != 'q'){
            std::cin >> c;
        }
    }else{
        auto terminal = replace_terminal();
        while (true) {
            std::cout << "wait " << std::endl;
            char c = getchar();
            if (c == 'q') {
                {
                    std::cout << "bye!" << std::endl;
                }
                break;
            }
            std::unique_lock<std::mutex> lk_c(mc);
            if(cond_cancel.wait_for(lk_c, std::chrono::milliseconds(100), []{return !monitoring_in_progress;})){
                std::cout << "left cycle" << std::endl;
                lk_c.unlock();
                break;
            }
        }
        std :: cout << "terminal is" << std::endl;
        set_terminal(terminal);
        std::cout << " reset!" << std::endl;
        {
            std::unique_lock<std::mutex> lock(mc);
            cancelled = monitoring_in_progress;
            monitoring_in_progress = false;
            lock.unlock();
        }
    }


}

void stop_monitoring(){
    std::cout << "STOP!" << std::endl;
    monitoring_in_progress = false;
    std::cout << "STOPPED!" << std::endl;
    cond_cancel.notify_one();
}

void main_demo(int fx, int gx){
    std::cout << "DEMO MODE: Press Q to quit." << std::endl;
    std::cout << "Testing F(" << fx << ") AND G(" << gx << ");" << std::endl;
    std::thread f(f_function, fx);
    std::thread g(g_function, gx);
    std::thread monitor(monitoring);

    start_threads();

    const bool debug = false;
    bool f_alive = true, g_alive = true, cancel_alive = true;

    int repeated = 0;
    std::unique_lock<std::mutex> lk_f(mf);
    std::unique_lock<std::mutex> lk_g(mg);
    std::unique_lock<std::mutex> lk_c(mc);
    while(true){//!f_processed && !g_processed && !cancelled
        if(cond_f.wait_for(lk_f, std::chrono::milliseconds(100), []{return f_processed;}))
            break;
        if(cond_g.wait_for(lk_g, std::chrono::milliseconds(100), []{return g_processed;}))
            break;
        if(cond_cancel.wait_for(lk_c, std::chrono::milliseconds(100), []{return cancelled;}))
            break;
        if(repeated % 20 == 0)
            std::cout << "Waiting along..."  << std::endl;
        repeated++;
    }
    lk_f.unlock();
    lk_g.unlock();
    lk_c.unlock();

    if(cancelled){
        std::cout << "Calculations were canceled!" << std::endl;
        f.detach();
        g.detach();
        monitor.join();
        g_alive = false;
        f_alive = false;
        return;
    }

    if(!g_processed) {
        std::cout << "F(" << fx << ") = " << f_result << std::endl;
        {
            while(true){//!g_processed && !cancelled
                if(cond_g.wait_for(lk_g, std::chrono::milliseconds(100), []{return g_processed;}))
                    break;
                if(cond_cancel.wait_for(lk_c, std::chrono::milliseconds(100), []{return cancelled;}))
                    break;
                if(repeated % 20 == 0)
                    std::cout << "Waiting for g..."  << std::endl;
                repeated++;
            }
            if(cancelled) {
                std::cout << "Calculation of g is cancelled!" << std::endl;
                g.detach();
                g_alive = false;
            }else{
                std::cout << "G(" << gx << ") = " << g_result << std::endl;
                std::cout << "RESULT: F(" << fx << ") AND G(" << gx << ") = " << (f_result & g_result) << std::endl;
                stop_monitoring();
            }
        }
    }
    else{
        std::cout << "G(" << gx << ") = " << g_result << std::endl;
        {
            while(true){//!g_processed && !cancelled
                if(cond_f.wait_for(lk_f, std::chrono::milliseconds(100), []{return f_processed;}))
                    break;
                if(cond_cancel.wait_for(lk_c, std::chrono::milliseconds(100), []{return cancelled;}))
                    break;
                if(repeated % 20 == 0)
                    std::cout << "Waiting for f..."  << std::endl;
                repeated++;
            }
            if(cancelled) {
                std::cout << "Calculation of f is cancelled!" << std::endl;
                f.detach();
                f_alive = false;
            }else{
                std::cout << "F(" << fx << ") = " << f_result << std::endl;
                std::cout << "RESULT: F(" << fx << ") AND G(" << gx << ") = " << (f_result & g_result) << std::endl;
                stop_monitoring();
            }
        }
    }
    cond_cancel.notify_one();
    monitor.join();
    if(f_alive)
        f.join();
    if(g_alive)
        g.join();

    std::cout << "----------------------------------------------------" << std::endl;
    std::cout.flush();
    //std::this_thread::sleep_for(std::chrono::seconds(1));
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



