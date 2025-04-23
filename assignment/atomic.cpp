#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

#include <iostream>
#include <thread>

thread_local int thread_id = -1;

void worker(int id) {
    thread_id = id;
    std::cout << "Thread " << id << " has thread_id = " << thread_id << std::endl;
    std::cout << "Thread " << id << " has thread_id = " << thread_id << std::endl;
    std::cout << "Thread " << id << " has thread_id = " << thread_id << std::endl;
    std::cout << "Thread " << id << " has thread_id = " << thread_id << std::endl;
    std::cout << "Thread " << id << " has thread_id = " << thread_id << std::endl;
    std::cout << "Thread " << id << " has thread_id = " << thread_id << std::endl;
    std::cout << "Thread " << id << " has thread_id = " << thread_id << std::endl;
    std::cout << "Thread " << id << " has thread_id = " << thread_id << std::endl;
}

int main() {
    std::thread t1(worker, 1);
    std::thread t2(worker, 2);

    t1.join();
    t2.join();

    std::cout << "Main thread's thread_id = " << thread_id << std::endl; // -1
    return 0;
}
