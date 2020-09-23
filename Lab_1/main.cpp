#include <iostream>
#include <thread>
#include <future>

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

int main()

{

    std::thread worker(worker_thread);

    data = "Example data";

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
