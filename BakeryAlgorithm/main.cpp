#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>


class Bakery {
	const int n;
	bool* volatile flags;
	int* volatile labels;

public:
	Bakery(int n) : n(n) {
		flags = new bool[n];
		labels = new int[n];

		for (int i = 0; i < n; ++i) {
			flags[i] = false;
			labels[i] = 0;
		}
	}

	~Bakery() {
		delete[] flags;
		delete[] labels;
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

	void unlock(const int i) {
		flags[i] = false;
	}
};

class TestCase {
	std::mutex mtx;
	Bakery* bakery;
	const int n;
	const int m;
	const int t;
	volatile int s;
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
			s = s + 1;
		}
	}

	void thread_function_mutex(int i) {
		for (int j = 0; j < t; ++j) {
			mtx.lock();
			s = s + 1;
			mtx.unlock();
		}
	}

	void thread_function_bakery(int i) {
		for (int j = 0; j < t; ++j) {
			bakery->lock(i);
			s = s + 1;
			bakery->unlock(i);
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
		switch (m)
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
		default:
			break;
		}
		std::cout << "Expected: " << n * t << ", Result: " << s << std::endl;
		std::cout << "Elapsed time: " << elapsed_time.count() << "s" << std::endl;
	}

	void result() {
		run();
		print_result();
	}
};

int main() {
	int mConstant = 10000000;
	for (int mode = 0; mode < 3; mode++)
		for (int i = 1; i <= 8; i = i * 2)
			TestCase(i, mConstant, mode).result();

	return 0;
}