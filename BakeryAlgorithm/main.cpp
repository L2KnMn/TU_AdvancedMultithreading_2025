#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include <algorithm>
#include <atomic>

class Bakery {
	const int n;
	bool*  volatile flags;
	int* volatile  labels;
	
	std::atomic<bool>* atomic_flags;
	std::atomic<int>* atomic_labels;

public:
	Bakery(int n) : n(n) {
		flags = new bool[n];
		labels = new int[n];

		atomic_flags = new std::atomic<bool>[n];
		atomic_labels = new std::atomic<int>[n];


		for (int i = 0; i < n; ++i) {
			flags[i] = false;
			labels[i] = 0;
		}

		for (int i = 0; i < n; ++i) {
			atomic_flags[i].store(false);
			atomic_labels[i].store(0);
		}

	}

	~Bakery() {
		delete[] flags;
		delete[] labels;
		delete[] atomic_flags;
		delete[] atomic_labels;
	}

	void lock(const int i) {
		labels[i] = 1 + *std::max_element(labels, labels + n);
		flags[i] = true;
		for (int k = 0; k < n; ++k) {
			if (k == i) continue;
			while (flags[k] &&
				(labels[i] > labels[k] || (labels[i] == labels[k] && i > k))) {
			}
		}
	}

	void atomic_lock(const int i) {
		atomic_labels[i].store(1 + *std::max_element(labels, labels + n));
		atomic_flags[i].store(true);
		for (int k = 0; k < n; ++k) {
			if (k == i) continue;
			while (atomic_flags[k].load() &&
				(atomic_labels[i].load() > atomic_labels[k].load() || (atomic_labels[i].load() == atomic_labels[k].load() && i > k))) {
			}
		}
	}

	void unlock(const int i) {
		flags[i] = false;
	}

	void atomic_unlock(const int i) {
		atomic_flags[i].store(false);
	}
};

class TestCase {
	std::mutex mtx;
	Bakery* bakery;
	const int n;
	const int m;
	const int t;
	volatile int s;
	std::atomic<int> atomic_s;
	
	std::vector<std::thread> threads;
	std::chrono::duration<double> elapsed_time;

public:
	TestCase(int n, int t, int m) : n(n), t(t / n), m(m) {
		this->s = 0;
		bakery = new Bakery(n);
		elapsed_time = std::chrono::duration<double>::zero();
	}
	~TestCase() {
		delete bakery;
	}

	void thread_function_natural(int i) {
		for (int j = 0; j < t; ++j) {
			atomic_s.store(atomic_s.load() + 1);
		}
	}

	void thread_function_mutex(int i) {
		for (int j = 0; j < t; ++j) {
			mtx.lock();
			atomic_s.store(atomic_s.load() + 1);
			mtx.unlock();
		}
	}

	void thread_function_bakery(int i) {
		for (int j = 0; j < t; ++j) {
			bakery->lock(i);
			atomic_s.store(atomic_s.load() + 1);
			bakery->unlock(i);
		}
	}

	void thread_function_atomic_bakery(int i) {
		for (int j = 0; j < t; ++j) {
			bakery->atomic_lock(i);
			atomic_s.store(atomic_s.load() + 1);
			bakery->atomic_unlock(i);
		}
	}

	void run() {
		std::chrono::time_point<std::chrono::system_clock> start_time = std::chrono::system_clock::now();
		for (int i = 0; i < n; ++i) {
			switch (m)
			{
			case 0:
				threads.emplace_back([this, i] { thread_function_natural(i); });
				break;
			case 1:
				threads.emplace_back([this, i] { thread_function_mutex(i); });
				break;
			case 2:
				threads.emplace_back([this, i] { thread_function_bakery(i); });
				break;
			case 3:
				threads.emplace_back([this, i] { thread_function_atomic_bakery(i); });
				break;
			default:
				break;
			}
		}
		for (auto& thread : threads) {
			thread.join();
		}
		elapsed_time = std::chrono::system_clock::now() - start_time;
	}

	void print_result() {
		std::cout << "Expected: " << n * t << ", Result: " << atomic_s.load() << std::endl;
		std::cout << "Elapsed time: " << elapsed_time.count() << "s" << std::endl;
	}

	void result() {
		run();
		print_result();
	}
};

int main() {
	int mConstant = 10000000;
	for (int mode = 0; mode < 4; mode++) {
		switch (mode)
		{
		case 0:
			std::cout << "Natural" << std::endl;
			break;
		case 1:
			std::cout << "Mutex" << std::endl;
			break;
		case 2:
			std::cout << "Bakery" << std::endl;
			break;
		case 3:
			std::cout << "Atomic Bakery" << std::endl;
			break;
		default:
			break;
		}
		for (int i = 1; i <= 8; i = i * 2) {
			TestCase(i, mConstant, mode).result();
		}
	}
	return 0;
}