#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

struct Memory {
	int value;
};

bool CAS(Memory memory, Memory expected, Memory update) {
	if (memory.value == expected.value) {
		memory.value = update.value;
		return true;
	}
	else
		return false;
}

int main() {

	return 0;
}