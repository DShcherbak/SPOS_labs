#include <iostream>
#include <thread>
#include <future>
#include <termios.h>
#include <unistd.h>
#include "cmake-build-debug/demofuncs"

using spos::lab1::demo::f_func;
using spos::lab1::demo::g_func;
const spos::lab1::demo::op_group AND = spos::lab1::demo::AND;

std::condition_variable cv; std::mutex m;
bool ready, processed;
std::string data;
void worker_thread()

{

// Wait until main() sends data

    std::unique_lock<std::mutex> lk(m);

    cv.wait(lk, []{return ready;});

// after the wait, we own the lock.

    std::cout << "Worker thread is processing data\n";

    data += " after processing";

// Send data back to main()

    processed = true;

    std::cout << "Worker thread signals data processing completed\n";

    lk.unlock();

    cv.notify_one();
}

int main_thread()

{

    std::thread worker(worker_thread);


// send data to the worker thread
    {
        std::lock_guard<std::mutex> lk(m);
        ready = true;
        std::cout << "main() signals data ready for processing\n";

    }
    cv.notify_one();
// wait for the worker
    {
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, []{return processed;});
    }
    std::cout << "Back in main(), data = " << data << '\n';
    worker.join();

}

void perform_test(int x){
    std::cout << "Completing test number " << x << std::endl;
    std::cout << "Press 'Ctrl-Z' to quit.\n";
    char ch;
    while ( (ch=fgetc(stdin)) != EOF ) {

        std::cout << x++ << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }


}

int main(){
    std::cout << "Press something to exit\n";
    termios oldt{};
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt{oldt};
    newt.c_lflag &= (~ICANON & ~ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::string s;
    getchar();

    std::cout << "k" << std::endl;
    return 0;

    for(int i = 0; i <= 5; i++){
        perform_test(i);
    }
    return 0;
}
