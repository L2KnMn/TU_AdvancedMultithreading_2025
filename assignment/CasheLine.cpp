#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>


int done = false;
int* volatile bound = nullptr;

int error = 0;

std::mutex mtx;

void ThreadFunc1() {
	for (int j = 0; j <= 2500000; ++j) {
		mtx.lock();
		*bound = -(1 + *bound);
		mtx.unlock();
	}
	done = true;
}

void ThreadFunc2() {
	while (done == false) {
		mtx.lock();
		int v = *bound;
		mtx.unlock();
		if (v != 0 && v != -1) {
			error += 1;
		}
	}
}

int main() {
	std::thread r{ ThreadFunc1 };
	std::thread w{ ThreadFunc2 };
	
	int arr[32];

	long long temp = reinterpret_cast<long long>(&arr[31]);
	temp = temp - (temp % 64);
	temp = temp - 1;
	bound = reinterpret_cast<int*>(temp);
	*bound = 0;

	r.join();
	w.join();	

	std::cout << "error: " << error << std::endl;

	return	0;
}