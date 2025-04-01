#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

volatile int sum = 0;
std::atomic<int> LOCK{ 0 };

bool CAS(std::atomic<int>* addr, int expected, int new_val)
{
    return std::atomic_compare_exchange_strong(
        addr, &expected, new_val);
}

void CAS_LOCK() {
    while (!CAS(&LOCK, 0, 1)) {}
}

void CAS_UNLOCK() {
    LOCK.store(0);
}

void worker(int num_threads) {
    const int loop_count = 50000000 / num_threads;
    for (int i = 0; i < loop_count; ++i) {
        CAS_LOCK();
        sum = sum + 2;
        CAS_UNLOCK();
    }
}

int main() {
    std::vector<int> num_threads_list = { 1, 2, 4, 8 };

    for (int num_threads : num_threads_list) {
        sum = 0; // sum 초기화
        auto start_time = std::chrono::high_resolution_clock::now();

        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back(worker, num_threads);
        }

        for (auto& thread : threads) {
            thread.join();
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        std::cout << num_threads << "개 스레드 실행 시간: " << duration.count() << " ms, sum: " << sum << std::endl;
    }

    return 0;
}