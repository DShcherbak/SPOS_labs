#include <functional>
#include "functions.h"
#include "custom_functions.h"
#include "terminal.h"

using spos::lab1::demo::f_func;
using spos::lab1::demo::g_func;
const spos::lab1::demo::op_group AND = spos::lab1::demo::AND;

std::condition_variable cond_f, cond_g;
std::mutex m, mf, mg;
bool f_ready, g_ready, f_processed, g_processed;
bool f_result, g_result;
std::vector<std::pair<int, int>> args;
int current_test_case;

void f_function(int test_case){
    int x = args[test_case].first;
    std::unique_lock<std::mutex> lock_f(mf);
    cond_f.wait(lock_f, []{return f_ready;});
    lock_f.unlock();

    f_ready = false;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    bool temp = f_func<AND>(x);

    std::unique_lock<std::mutex> lock(m);
    if(test_case == current_test_case){
        lock_f.lock();
        f_processed = true;
        f_result = temp;
        cond_f.notify_one();
    }
}

void g_function(int test_case){
    int x = args[test_case].second;
    std::unique_lock<std::mutex> lock_g(mg);
    cond_g.wait(lock_g, []{return g_ready;});
    lock_g.unlock();

    g_ready = false;
    std::this_thread::sleep_for(std::chrono::seconds(4));
    bool temp = g_func<AND>(x);

    std::unique_lock<std::mutex> lock(m);
    if(test_case == current_test_case){
        lock_g.lock();
        g_processed = true;
        g_result = temp;
        cond_g.notify_one();
    }

}

void f_manual(int test_case){
    int x = args[test_case].first;
    std::unique_lock<std::mutex> lock_f(mf);
    cond_f.wait(lock_f, []{return f_ready;});
    lock_f.unlock();

    f_ready = false;
    bool temp = my_F(x);

    std::unique_lock<std::mutex> lock(m);
    if(test_case == current_test_case){
        lock_f.lock();
        f_processed = true;
        f_result = temp;
        cond_f.notify_one();
    }
}

void g_manual(int test_case){
    int x = args[test_case].first;
    std::unique_lock<std::mutex> lock_g(mg);
    cond_g.wait(lock_g, []{return g_ready;});
    lock_g.unlock();

    g_ready = false;
    bool temp = my_G(x);

    std::unique_lock<std::mutex> lock(m);
    if(test_case == current_test_case){
        lock_g.lock();
        g_processed = true;
        g_result = temp;
        cond_g.notify_one();
    }
}

void start_threads(int test_case){
    {
        std::lock_guard<std::mutex> lk(m);
        current_test_case = test_case;
    }
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
}

void main_thread(int fx, int gx, bool demo_mode = true){
    static int test_case = 0;


    std::cout << "Testing F(" << fx << ") AND G(" << gx << ");" << std::endl;
    args.emplace_back(fx, gx);
    std::thread f, g;
    if(demo_mode) {
        f = std::thread(f_function, test_case);
        g = std::thread(g_function, test_case);
    }
    else {
        f = std::thread(f_manual, test_case);
        g = std::thread(g_manual, test_case);
    }
    start_threads(test_case);
    test_case++;

    const bool full_log = true;
    bool f_alive = true, g_alive = true, monitor = true;

    int repeated = 0;


    auto terminal = replace_terminal();
    monitor = true;


    std::cout << "Press Q to quit." << std::endl;
    while(monitor){//!f_processed && !g_processed && !cancelled
        {
            std::unique_lock<std::mutex> lk_f(mf);
            if(cond_f.wait_for(lk_f, std::chrono::milliseconds(100), []{return f_processed;}))
                break;
        }
        {
            std::unique_lock<std::mutex> lk_g(mg);
            if(cond_g.wait_for(lk_g, std::chrono::milliseconds(100), []{return g_processed;}))
                break;
        }
        char c = getchar();
        if (c == 'q') {
            monitor = false;
        }
        if(full_log && repeated % 20 == 0)
            std::cout << "Waiting along..."  << std::endl;
        repeated++;
    }

    if(!monitor){ // Calculations were interupted
        std::cout << "Calculations were canceled!" << std::endl;
        {
            std::unique_lock<std::mutex> lk_f(mf); // no need to calculate f
            f.detach();
        }
        {
            std::unique_lock<std::mutex> lk_g(mg); // no need to calculate g
            g.detach();
        }
        g_alive = false; // no need to join f and g (may go on endlessly)
        f_alive = false;

    } else if(!g_processed) { // F(x) has been precessed

        std::cout << "F(" << fx << ") = " << f_result << std::endl;
        if(f_result == 0){
            std::cout << "Calculation of G is unnecessary" << std::endl; // F returned 0
            std::cout << "Result = 0" << std::endl;
            g_alive = false;
            g.detach();
        } else {
            while(monitor) {//!g_processed && !cancelled
                {
                    std::unique_lock<std::mutex> lk_g(mg);
                    if(cond_g.wait_for(lk_g, std::chrono::milliseconds(100), []{return g_processed;}))
                        break;
                }
                char c = getchar();
                if (c == 'q') {
                    monitor = false;
                }
                if(full_log && repeated % 20 == 0)
                    std::cout << "Waiting for g..."  << std::endl;
                repeated++;
            }
            if(!monitor) { // Calculations cancelled
                g.detach();
                g_alive = false;
                std::cout << "Calculation of g is cancelled!" << std::endl;
            }else{
                std::cout << "G(" << gx << ") = " << g_result << std::endl;
                std::cout << "RESULT: F(" << fx << ") AND G(" << gx << ") = " << (f_result & g_result) << std::endl;
            }
        }
    } else { // F(x) has been precessed
        std::cout << "G(" << gx << ") = " << g_result << std::endl;
        if(g_result == 0){
            std::cout << "Calculation of F is unnecessary" << std::endl; // F returned 0
            std::cout << "Result = 0" << std::endl;
            f_alive = false;
            f.detach();
        } else {
            while (monitor) {//!g_processed && !cancelled
                {
                    std::unique_lock<std::mutex> lk_f(mf);
                    if (cond_f.wait_for(lk_f, std::chrono::milliseconds(100), [] { return f_processed; }))
                        break;
                }
                char c = getchar();
                if (c == 'q') {
                    monitor = false;
                }
                if (full_log && repeated % 20 == 0)
                    std::cout << "Waiting for f..." << std::endl;
                repeated++;
            }
            if (!monitor) {  // Calculations cancelled
                std::cout << "Calculation of f is cancelled!" << std::endl;
                f.detach();
                f_alive = false;
            } else {
                std::cout << "F(" << fx << ") = " << f_result << std::endl;
                std::cout << "RESULT: F(" << fx << ") AND G(" << gx << ") = " << (f_result & g_result) << std::endl;
            }
        }

    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    set_terminal(terminal);

    if(f_alive)
        f.join();

    if(g_alive)
        g.join();

    std::cout << "----------------------------------------------------" << std::endl;
    std::cout.flush();

}
