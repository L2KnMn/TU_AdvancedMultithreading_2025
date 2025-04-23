#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
#include <array>
#include <queue>

class NODE {
public:
	int ebr_counter = 0;
	int key;
	NODE* next;
	std::mutex sm;
	volatile bool mark;

	NODE() { 
		key = 0;
		next = nullptr; 
	}

	NODE(int key_value) {
		next = nullptr;
		key = key_value;
	}

	~NODE() {}

	void lock()
	{
		sm.lock();
	}

	void unlock()
	{
		sm.unlock();
	}
};

class LIST{
	NODE* head, * tail;
public:
	LIST()
	{
		head = new NODE{ std::numeric_limits<int>::min() };
		tail = new NODE{ std::numeric_limits<int>::max() };
		head->next = tail;
	}
	~LIST()
	{
		clear();
		delete head;
		delete tail;
	}
	void clear()
	{
		while (head->next != tail) {
			auto ptr = head->next;
			head->next = head->next->next;
			delete ptr;
		}
	}
	bool Add(int key)
	{
		NODE* pred = head;
		NODE* curr = pred->next;
		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}
		if (curr->key == key) {
			return false;
		}
		else {
			auto n = new NODE{ key };
			n->next = curr;
			pred->next = n;
			return true;
		}
	}
	bool Remove(int key)
	{
		NODE* pred = head;
		NODE* curr = pred->next;
		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}
		if (curr->key == key) {
			auto n = curr;
			pred->next = n->next;
			delete n;
			return true;
		}
		else {
			return false;
		}
	}
	bool Contains(int key)
	{
		NODE* pred = head;
		NODE* curr = pred->next;
		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}
		return curr->key == key;
	}
	void print20()
	{
		auto p = head->next;
		for (int i = 0; i < 20; ++i) {
			if (tail == p) break;
			std::cout << p->key << ", ";
			p = p->next;
		}
		std::cout << std::endl;
	}
};

class CLIST {
	NODE* head;
	NODE* tail;
	std::mutex mtx;

public:
	CLIST()
	{
		head = new NODE{ std::numeric_limits<int>::min() };
		tail = new NODE{ std::numeric_limits<int>::max() };
		head->next = tail;
	}
	~CLIST() {
		clear();
		delete head;
		delete tail;
	}

	void clear()
	{

		while (head->next != tail) {
			mtx.lock();
			auto ptr = head->next;
			head->next = head->next->next;
			delete ptr;
			mtx.unlock();
		}
	}
	bool Add(int key)
	{
		mtx.lock();
		NODE* pred = head;
		NODE* curr = pred->next;

		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}

		if (curr->key == key) {
			mtx.unlock();
			return false;
		}
		else {
			auto n = new NODE{ key };
			n->next = curr;
			pred->next = n;
			mtx.unlock();
			return true;
		}
	}
	bool Remove(int key)
	{
		mtx.lock();
		NODE* pred = head;
		NODE* curr = pred->next;

		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}

		if (curr->key == key) {
			auto n = curr;
			pred->next = n->next;
			delete n;
			mtx.unlock();
			return true;
		}
		else {
			mtx.unlock();
			return false;
		}
	}
	bool Contains(int key)
	{
		mtx.lock();
		NODE* pred = head;
		NODE* curr = pred->next;

		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}

		if (curr->key == key) {
			mtx.unlock();
			return true;
		}
		else {
			mtx.unlock();
			return false;
		}
	}
	void print20()
	{
		auto p = head->next;

		for (int i = 0; i < 20; ++i) {
			if (tail == p) break;
			std::cout << p->key << ", ";
			p = p->next;
		}
		std::cout << std::endl;
	}
};

class FLIST {
	NODE* head, * tail;
public:
	FLIST()
	{
		head = new NODE{ std::numeric_limits<int>::min() };
		tail = new NODE{ std::numeric_limits<int>::max() };
		head->next = tail;
	}
	~FLIST()
	{
		clear();
		delete head;
		delete tail;
	}

	void clear()
	{
		while (head->next != tail) {
			auto ptr = head->next;
			head->next = head->next->next;
			delete ptr;
		}
	}
	bool Add(int key)
	{
		head->lock();
		NODE* pred = head;
		NODE* curr = pred->next;
		curr->lock();

		while (curr->key < key) {
			pred->unlock(); 
			pred = curr;
			curr = curr->next;
			curr->lock();
		}

		if (curr->key == key) {
			pred->unlock(); curr->unlock();
			return false;
		}
		else {
			auto n = new NODE{ key };
			n->next = curr;
			pred->next = n;
			pred->unlock(); curr->unlock();
			return true;
		}
	}

	bool Remove(int key)
	{
		head->lock();
		NODE* pred = head;
		NODE* curr = pred->next;
		curr->lock();

		while (curr->key < key) {
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}

		if (curr->key == key) {
			auto n = curr;
			pred->next = n->next;
			pred->unlock(); 
			curr->unlock();
			delete n;
			return true;
		}
		else {
			pred->unlock(); 
			curr->unlock();
			return false;
		}
	}

	bool Contains(int key)
	{
		NODE* pred = head;
		pred->lock();
		NODE* curr = pred->next;
		curr->lock();

		while (curr->key < key) {
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}

		if (curr->key == key) {
			pred->unlock(); curr->unlock();
			return true;
		}
		else {
			pred->unlock(); curr->unlock();
			return false;
		}
	}

	void print20()
	{
		auto p = head->next;

		for (int i = 0; i < 20; ++i) {
			if (tail == p) break;
			std::cout << p->key << ", ";
			p = p->next;
		}
		std::cout << std::endl;
	}
};

class OLIST {
	NODE* head, * tail;
public:
	OLIST()
	{
		head = new NODE{ std::numeric_limits<int>::min() };
		tail = new NODE{ std::numeric_limits<int>::max() };
		head->next = tail;
	}
	~OLIST()
	{
		clear();
		delete head;
		delete tail;
	}

	void clear()
	{
		while (head->next != tail) {
			auto ptr = head->next;
			head->next = head->next->next;
			delete ptr;
		}
	}

	bool validate(NODE* pred, NODE* curr)
	{
		NODE* n = head;
		while(n->key <= pred->key) {
			if (n == pred) {
				return curr == pred->next;
			}
			n = n->next;
		}
		return false;
	}

	bool Add(int key)
	{
		while (true) {
			NODE* pred = head;
			NODE* curr = pred->next;

			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}
			
			std::lock_guard<std::mutex> lp(pred->sm);
			std::lock_guard<std::mutex> lc(curr->sm);
			if (!validate(pred, curr)) {
				continue;
			}
			if (curr->key == key) {
				return false;
			}
			else {
				auto n = new NODE{ key };
				n->next = curr;
				pred->next = n;
				return true;
			}
		}
	}

	bool Remove(int key)
	{
		while (true) {
			NODE* pred = head;
			NODE* curr = pred->next;

			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}

			std::lock_guard<std::mutex> lp(pred->sm);
			std::lock_guard<std::mutex> lc(curr->sm);
			if (!validate(pred, curr)) {
				continue;
			}
			if (curr->key == key) {
				auto n = curr;
				pred->next = n->next;
				//delete n;
				return true;
			}
			else {
				return false;
			}
		}
	}

	bool Contains(int key)
	{
		while (true) {
			NODE* pred = head;
			NODE* curr = pred->next;

			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}

			std::lock_guard<std::mutex> lp(pred->sm);
			std::lock_guard<std::mutex> lc(curr->sm);
			if (!validate(pred, curr)) {
				continue;
			}
			if (curr->key == key) {
				return true;
			}
			else {
				return false;
			}
		}
	}

	void print20()
	{
		auto p = head->next;

		for (int i = 0; i < 20; ++i) {
			if (tail == p) break;
			std::cout << p->key << ", ";
			p = p->next;
		}
		std::cout << std::endl;
	}
};

class LLIST {
	NODE* head, * tail;
public:
	LLIST()
	{
		head = new NODE{ std::numeric_limits<int>::min() };
		tail = new NODE{ std::numeric_limits<int>::max() };
		head->next = tail;
	}
	~LLIST()
	{
		delete head;
		delete tail;
	}

	void clear()
	{
		while (head->next != tail) {
			auto ptr = head->next;
			head->next = head->next->next;
			delete ptr;
		}
	}

	bool validate(NODE* pred, NODE* curr)
	{
		return !pred->mark && !curr->mark && pred->next == curr;
	}

	bool Add(int key)
	{
		NODE* pred = head;
		NODE* curr = pred->next;
		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}


		std::lock_guard <std::mutex> pl{ pred->sm };
		std::lock_guard <std::mutex> cl{ curr->sm };
		if (validate(pred, curr)) {
			if (curr->key == key) {
				return false;
			}
			else {
				auto n = new NODE{ key }; 
				n->next = curr;
				pred->next = n;
				return true;
			}
		}
		return false;
	}
	bool Remove(int key)
	{
		NODE* pred = head;
		NODE* curr = pred->next;
		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}

		std::lock_guard <std::mutex> pl{ pred->sm };
		std::lock_guard <std::mutex> cl{ curr->sm };
		if (true == validate(pred, curr)) {
			if (curr->key == key) {
				auto n = curr;
				curr->mark = true;
				pred->next = n->next;
				return true;
			}
			else {
				return false;
			}
		}
		return false;
	}
	bool Contains(int key)
	{
		NODE* pred = head;
		NODE* curr = pred->next;

		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}
		return curr->key == key && !curr->mark;
	}
	void print20()
	{
		auto p = head->next;

		for (int i = 0; i < 20; ++i) {
			if (tail == p) break;
			std::cout << p->key << ", ";
			p = p->next;
		}
		std::cout << std::endl;
	}
};

struct NODE_SP {
	int key;
	std::shared_ptr<NODE_SP> next{ nullptr };
	std::mutex sm;
	volatile bool removed{ false };
	NODE_SP() : key(-1) {}
	NODE_SP(int x) : key(x) {}
	void lock()
	{
		sm.lock();
	}
	void unlock()
	{
		sm.unlock();
	}
};

class LLIST_SP {
	std::shared_ptr<NODE_SP> head, tail;
public:
	LLIST_SP()
	{
		head = std::make_shared<NODE_SP>(std::numeric_limits<int>::min());
		tail = std::make_shared<NODE_SP>(std::numeric_limits<int>::max());
		head->next = tail;
	}
	~LLIST_SP()
	{
	}

	void clear()
	{
		head->next = tail;
	}

	bool validate(const std::shared_ptr<NODE_SP>& pred, const std::shared_ptr<NODE_SP>& curr)
	{
		return (pred->removed == false) && (curr->removed == false)
			&& (pred->next == curr);
	}

	bool Add(int key)
	{
		while (true) {
			std::shared_ptr<NODE_SP> pred = head;
			std::shared_ptr<NODE_SP> curr = pred->next;
			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}

			pred->lock(); curr->lock();
			if (true == validate(pred, curr)) {
				if (curr->key == key) {
					pred->unlock(); curr->unlock();
					return false;
				}
				else {
					auto n = std::make_shared<NODE_SP>(key);
					n->next = curr;
					pred->next = n;
					pred->unlock(); curr->unlock();
					return true;
				}
			}
			else {
				pred->unlock(); curr->unlock();
			}
		}
	}
	bool Remove(int key)
	{
		while (true) {
			std::shared_ptr<NODE_SP> pred = head;
			std::shared_ptr<NODE_SP> curr = pred->next;
			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}

			pred->lock(); curr->lock();
			if (true == validate(pred, curr)) {
				if (curr->key == key) {
					auto n = curr;
					curr->removed = true;
					pred->next = n->next;
					pred->unlock(); curr->unlock();
					//delete n;
					return true;
				}
				else {
					pred->unlock(); curr->unlock();
					return false;
				}
			}
			else {
				pred->unlock(); curr->unlock();
				continue;
			}
		}
	}
	bool Contains(int key)
	{
		std::shared_ptr<NODE_SP> curr = head;
		while (curr->key < key) {
			curr = curr->next;
		}
		return (curr->removed == false) && (curr->key == key);
	}
	void print20()
	{
		auto p = head->next;

		for (int i = 0; i < 20; ++i) {
			if (tail == p) break;
			std::cout << p->key << ", ";
			p = p->next;
		}
		std::cout << std::endl;
	}
};

struct LFNODE;

class AMR
{
	std::atomic_llong data;
public:
	AMR() : data(0) {}
	~AMR() {}

	bool get_mark()
	{
		return (data & 1) == 1;
	}
	LFNODE* get_ptr()
	{
		long long temp = data & 0xFFFFFFFFFFFFFFFE;
		return reinterpret_cast<LFNODE*>(temp);
	}

	LFNODE* get_ptr(bool* is_removed)
	{
		long long temp = data;
		*is_removed = (temp & 1) == 1;
		return reinterpret_cast<LFNODE*>(temp & 0xFFFFFFFFFFFFFFFE);
	}
	void set_ptr(LFNODE* p)
	{
		long long temp = reinterpret_cast<long long>(p);
		temp = temp & 0xFFFFFFFFFFFFFFFE;
		data = temp;
	}

	bool CAS(LFNODE* old_p, LFNODE* new_p, bool old_m, bool new_m)
	{
		long long old_value = reinterpret_cast<long long>(old_p);
		long long new_value = reinterpret_cast<long long>(new_p);
		if (true == old_m)
			old_value = old_value | 1;
		else
			old_value = old_value & 0xFFFFFFFFFFFFFFFE;
		if (true == new_m)
			new_value = new_value | 1;
		else
			new_value = new_value & 0xFFFFFFFFFFFFFFFE;

		return std::atomic_compare_exchange_strong(&data, &old_value, new_value);
	}
};

struct LFNODE {
	int key;
	AMR next;
	std::atomic<int> ebr_counter = 0;
	LFNODE() : key(-1) {}
	LFNODE(int x) : key(x) {}
};

class LFLIST {
	LFNODE* head, * tail;
public:
	LFLIST()
	{
		head = new LFNODE{ std::numeric_limits<int>::min() };
		tail = new LFNODE{ std::numeric_limits<int>::max() };
		head->next.set_ptr(tail);
	}
	~LFLIST()
	{
		delete head;
		delete tail;
	}

	void clear()
	{
		while (head->next.get_ptr() != tail) {
			auto ptr = head->next.get_ptr();
			head->next.set_ptr(head->next.get_ptr()->next.get_ptr());
			delete ptr;
		}
	}

	void Find(LFNODE*& pred, LFNODE*& curr, int x)
	{
	retry:
		pred = head;
		while (true) {
			curr = pred->next.get_ptr();
			bool is_removed = false;
			LFNODE* succ = curr->next.get_ptr(&is_removed);
			while (true == is_removed) {
				if (false == pred->next.CAS(curr, succ, false, false))
					goto retry;
				curr = succ;
				succ = curr->next.get_ptr(&is_removed);
			}
			if (curr->key >= x) return;
			pred = curr;
			curr = succ;
		}
	}

	bool Add(int key)
	{
		while (true) {
			LFNODE* pred = head;
			LFNODE* curr = pred->next.get_ptr();

			Find(pred, curr, key);

			if (curr->key == key)
				return false;
			else {
				auto n = new LFNODE{ key };
				n->next.set_ptr(curr);
				if (true == pred->next.CAS(curr, n, false, false))
					return true;
			}
		}
	}
	bool Remove(int key)
	{
		while (true) {
			LFNODE* pred, * curr;
			Find(pred, curr, key);
			if (curr->key == key) {
				LFNODE* succ = curr->next.get_ptr();
				if (false == curr->next.CAS(succ, succ, false, true))
					continue;
				pred->next.CAS(curr, succ, false, false);
				return true;
			}
			else return false;
		}
	}
	bool Contains(int key)
	{
		LFNODE* curr = head;
		while (curr->key < key) {
			curr = curr->next.get_ptr();
		}
		return (curr->next.get_mark() == false) && (curr->key == key);
	}
	void print20()
	{
		auto p = head->next.get_ptr();

		for (int i = 0; i < 20; ++i) {
			if (tail == p) break;
			std::cout << p->key << ", ";
			p = p->next.get_ptr();
		}
		std::cout << std::endl;
	}
};

constexpr int CACHE_LINE_PADDING = 16;
constexpr int MAX_THREADS = 16;

std::atomic_int g_ebr_counter = 0;
std::atomic<int> thread_count{ 0 };

thread_local std::queue<LFNODE*> free_list;
thread_local std::queue<NODE*> free_list_lazy;
thread_local volatile int thread_id;

std::atomic_int thread_ebr[MAX_THREADS * CACHE_LINE_PADDING];
int g_num_threads;

LFNODE* ebr_new(int x)
{
	if (free_list.empty()) return new LFNODE(x);

	LFNODE* p = free_list.front();

	int ebr_counter = p->ebr_counter;

	for (int i = 0; i < g_num_threads; ++i)
		if (thread_ebr[i * 16] < ebr_counter) {
			return new LFNODE(x);
		}

	if (p->next.CAS(p->next.get_ptr(), nullptr, true, false)) {
		free_list.pop();
		p->key = x;
		p->ebr_counter = 0;
		return p;
	}else
	{	
		std::cout << "EBR 경합 있었음\n" << std::endl;
		return new LFNODE(x);
	}
}

std::mutex mtx;
NODE* ebr_new_lazy(int x) {
	if (free_list_lazy.empty()) return new NODE(x);

	std::lock_guard<std::mutex> lock(mtx);
	NODE* p = free_list_lazy.front();
	int ebr_counter = p->ebr_counter;
	for (int i = 0; i < g_num_threads; ++i)
		if (thread_ebr[i * 16] < ebr_counter) {
			return new NODE(x);
		}
	free_list_lazy.pop();
	p->key = x;
	p->next = nullptr;
	p->mark = false;
	p->ebr_counter = 0;
	return p;
}

void ebr_delete(LFNODE* p)
{
	p->ebr_counter.store(g_ebr_counter);
	free_list.push(p);
}

void ebr_delete(NODE* p)
{
	p->ebr_counter = g_ebr_counter;
	free_list_lazy.push(p);
}

// 스레드 시작 시 epoch 초기화 (main 또는 스레드 함수 시작 부분에서 호출)
void ebr_thread_initialize(int id) {
	thread_id = id;
	if (thread_id == -1) {
		// 스레드 ID가 초기화되지 않았으면 오류 처리 또는 예외 발생
		throw std::runtime_error("EBR thread not initialized");
	}
	// 스레드의 epoch를 비활성 상태(max 값)로 초기화
	thread_ebr[thread_id * CACHE_LINE_PADDING].store(std::numeric_limits<int>::max(), std::memory_order_relaxed);
}

void ebr_thread_finalize(int id) {
	while (!free_list.empty()) {
		auto node = free_list.front();
		free_list.pop();
		delete node;
	}
	while (!free_list_lazy.empty()) {
		auto node = free_list_lazy.front();
		free_list_lazy.pop();
		delete node;
	}
}

// 스레드 연산 시작 시 epoch 등록
void ebr_enter_critical() {
	if (thread_id == -1) {
		// 스레드 ID가 초기화되지 않았으면 오류 처리 또는 예외 발생
		throw std::runtime_error("EBR thread not initialized");
	}
	// 현재 전역 epoch 값을 읽어 스레드 로컬 epoch로 설정 (memory_order_acquire 사용)
	thread_ebr[thread_id * CACHE_LINE_PADDING].store(g_ebr_counter.fetch_add(1), std::memory_order_release);
}

// 스레드 연산 종료 시 epoch 해제
void ebr_exit_critical() {
	if (thread_id == -1) {
		throw std::runtime_error("EBR thread not initialized");
	}
	// 스레드 로컬 epoch를 비활성 상태(max 값)로 설정
	thread_ebr[thread_id * CACHE_LINE_PADDING].store(std::numeric_limits<int>::max(), std::memory_order_release);
}
class EBR_LFLIST {
	LFNODE* head, * tail;
public:
	EBR_LFLIST()
	{
		head = new LFNODE{ std::numeric_limits<int>::min() };
		tail = new LFNODE{ std::numeric_limits<int>::max() };
		head->next.set_ptr(tail);

		static bool ebr_initialized = false;
		if (!ebr_initialized) {
			for (int i = 0; i < MAX_THREADS * CACHE_LINE_PADDING; ++i) {
				thread_ebr[i].store(std::numeric_limits<int>::max(), std::memory_order_relaxed);
			}
			ebr_initialized = true;
		}
	}
	~EBR_LFLIST()
	{
		LFNODE* curr = head->next.get_ptr();
		while (curr != tail) {
			LFNODE* next = curr->next.get_ptr();
			delete curr; // EBR을 거치지 않고 직접 삭제 (소멸 시점 가정이므로)
			curr = next;
		}
		delete head;
		delete tail;
	}

	void clear()
	{
		// 반드시 threads의 작업이 끝나고 불리니까 
		// 안심하고 싱글스레드 기준으로 전부 삭제
		LFNODE* curr = head->next.get_ptr();
		while (curr != tail) {
			LFNODE* next = curr->next.get_ptr();
			delete curr;
			curr = next;
		}
		// head의 next를 tail로 다시 연결 혹시 모르니
		head->next.set_ptr(tail);
	}

	void Find(LFNODE*& pred, LFNODE*& curr, int x)
	{
	retry:
		pred = head;
		while (true) {
			curr = pred->next.get_ptr();
			bool is_removed = false;
			LFNODE* succ = curr->next.get_ptr(&is_removed);
			while (true == is_removed) {
				if (false == pred->next.CAS(curr, succ, false, false))
					goto retry;
				ebr_delete(curr);
				curr = succ;
				succ = curr->next.get_ptr(&is_removed);
			}
			if (curr->key >= x) return;
			pred = curr;
			curr = succ;
		}
	}

	bool Add(int key)
	{
		ebr_enter_critical();
		while (true) {
			LFNODE* pred = head;
			LFNODE* curr = pred->next.get_ptr();

			Find(pred, curr, key);

			if (curr->key == key) {
				ebr_exit_critical();
				return false;
			}
			else {
				auto n = ebr_new(key);
				n->next.set_ptr(curr);
				if (true == pred->next.CAS(curr, n, false, false)) {
					ebr_exit_critical();
					return true;
				}
			}
		}
	}
	bool Remove(int key)
	{
		ebr_enter_critical();
		while (true) {
			LFNODE* pred, * curr;
			Find(pred, curr, key);
			if (curr->key == key) {
				LFNODE* succ = curr->next.get_ptr();
				if (false == curr->next.CAS(succ, succ, false, true))
					continue;
				if (true == pred->next.CAS(curr, succ, false, false))
					ebr_delete(curr);
				ebr_exit_critical();
				return true;
			}
			else {
				ebr_exit_critical();
				return false;
			}
		}
	}
	bool Contains(int key)
	{
		ebr_enter_critical();
		LFNODE* curr = head;
		while (curr->key < key) {
			curr = curr->next.get_ptr();
		}
		ebr_exit_critical();
		return (curr->next.get_mark() == false) && (curr->key == key);
	}

	void print20()
	{
		auto p = head->next.get_ptr();

		for (int i = 0; i < 20; ++i) {
			if (tail == p) break;
			std::cout << p->key << ", ";
			p = p->next.get_ptr();
		}
		std::cout << std::endl;
	}
};

class EBR_LLIST {
	NODE* head, * tail;
public:
	EBR_LLIST()
	{
		head = new NODE{ std::numeric_limits<int>::min() };
		tail = new NODE{ std::numeric_limits<int>::max() };
		head->next = tail;
	}
	~EBR_LLIST()
	{
		delete head;
		delete tail;
	}

	void clear()
	{
		while (head->next != tail) {
			auto ptr = head->next;
			head->next = head->next->next;
			delete ptr;
		}
	}

	bool validate(NODE* pred, NODE* curr)
	{
		return !pred->mark && !curr->mark && pred->next == curr;
	}

	bool Add(int key)
	{
		ebr_enter_critical();
		NODE* pred = head;
		NODE* curr = pred->next;
		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}

		std::lock_guard <std::mutex> pl{ pred->sm };
		std::lock_guard <std::mutex> cl{ curr->sm };
		if (validate(pred, curr)) {
			if (curr->key == key) {
				ebr_exit_critical();
				return false;
			}
			else {
				auto n = ebr_new_lazy(key);
				n->next = curr;
				pred->next = n;
				ebr_exit_critical();
				return true;
			}
		}
		ebr_exit_critical();
		return false;
	}
	bool Remove(int key)
	{
		ebr_enter_critical();
		NODE* pred = head;
		NODE* curr = pred->next;
		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}

		std::lock_guard <std::mutex> pl{ pred->sm };
		std::lock_guard <std::mutex> cl{ curr->sm };
		if (true == validate(pred, curr)) {
			if (curr->key == key) {
				curr->mark = true;
				pred->next = curr->next;
				ebr_delete(curr);
				ebr_exit_critical();
				return true;
			}
			else {
				ebr_exit_critical();
				return false;
			}
		}
		ebr_exit_critical();
		return false;
	}
	bool Contains(int key)
	{
		ebr_enter_critical();
		NODE* pred = head;
		NODE* curr = pred->next;

		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}
		ebr_exit_critical();
		return curr->key == key && !curr->mark;
	}
	void print20()
	{
		auto p = head->next;

		for (int i = 0; i < 20; ++i) {
			if (tail == p) break;
			std::cout << p->key << ", ";
			p = p->next;
		}
		std::cout << std::endl;
	}
};

class HISTORY {
public:
	int op;
	int i_value;
	bool o_value;
	HISTORY(int o, int i, bool re) : op(o), i_value(i), o_value(re) {}
};

std::array<std::vector<HISTORY>, 16> history;


constexpr int NUM_TEST = 4000000;
constexpr int KEY_RANGE = 1000;

EBR_LFLIST g_set;

void check_history(int num_threads)
{
	std::array <int, KEY_RANGE> survive = {};
	std::cout << "Checking Consistency : ";
	if (history[0].size() == 0) {
		std::cout << "No history.\n";
		return;
	}
	for (int i = 0; i < num_threads; ++i) {
		for (auto& op : history[i]) {
			if (false == op.o_value) continue;
			if (op.op == 3) continue;
			if (op.op == 0) survive[op.i_value]++;
			if (op.op == 1) survive[op.i_value]--;
		}
	}
	for (int i = 0; i < KEY_RANGE; ++i) {
		int val = survive[i];
		if (val < 0) {
			std::cout << "ERROR. The value " << i << " removed while it is not in the set.\n";
			exit(-1);
		}
		else if (val > 1) {
			std::cout << "ERROR. The value " << i << " is added while the set already have it.\n";
			exit(-1);
		}
		else if (val == 0) {
			if (g_set.Contains(i)) {
				std::cout << "ERROR. The value " << i << " should not exists.\n";
				exit(-1);
			}
		}
		else if (val == 1) {
			if (false == g_set.Contains(i)) {
				std::cout << "ERROR. The value " << i << " shoud exists.\n";
				exit(-1);
			}
		}
	}
	std::cout << " OK\n";
}
void benchmark_check(int num_threads, int th_id)
{
	ebr_thread_initialize(th_id);
	for (int i = 0; i < NUM_TEST / num_threads; ++i) {
		int op = rand() % 3;
		switch (op) {
			case 0: {
				int v = rand() % KEY_RANGE;
				history[th_id].emplace_back(0, v, g_set.Add(v));
				break;
			}
			case 1: {
				int v = rand() % KEY_RANGE;
				history[th_id].emplace_back(1, v, g_set.Remove(v));
				break;
			}
			case 2: {
				int v = rand() % KEY_RANGE;
				history[th_id].emplace_back(2, v, g_set.Contains(v));
				break;
			}
		}
	}
	ebr_thread_finalize(th_id);
}
void benchmark(int num_thread, int th_id)
{
	int key;
	const int num_loop = NUM_TEST / num_thread;
	ebr_thread_initialize(th_id);
	for (int i = 0; i < num_loop; i++) {
		switch (rand() % 3) {
		case 0: key = rand() % KEY_RANGE;
			g_set.Add(key);
			break;
		case 1: key = rand() % KEY_RANGE;
			g_set.Remove(key);
			break;
		case 2: key = rand() % KEY_RANGE;
			g_set.Contains(key);
			break;
		default: std::cout << "Error\n";
			exit(-1);
		}
	}
	ebr_thread_finalize(th_id);
}

int main()
{
	using namespace std::chrono;

	{
		auto start_t = system_clock::now();
		int key;
		LIST g_set;
		for (int i = 0; i < NUM_TEST; i++) {
			switch (rand() % 3) {
			case 0: key = rand() % KEY_RANGE;
				g_set.Add(key);
				break;
			case 1: key = rand() % KEY_RANGE;
				g_set.Remove(key);
				break;
			case 2: key = rand() % KEY_RANGE;
				g_set.Contains(key);
				break;
			default: std::cout << "Error\n";
				exit(-1);
			}
		}
		auto end_t = system_clock::now();
		auto exec_t = end_t - start_t;
		auto exec_ms = duration_cast<milliseconds>(exec_t).count();

		std::cout << "Single Thread SET = ";
		g_set.print20();
		std::cout << ", Exec time = " << exec_ms << "ms.\n;";
	}

	// 알고리즘 정확성 검사
	{
		for (int i = 1; i <= 16; i = i * 2) {
			thread_count = 0;
			g_num_threads = i;
			g_ebr_counter = 0;

			std::vector <std::thread> threads;
			g_set.clear();
			for (auto& h : history) h.clear();
			auto start_t = system_clock::now();
			for (int j = 0; j < i; ++j)
				threads.emplace_back(benchmark_check, i, j);
			for (auto& th : threads)
				th.join();
			auto end_t = system_clock::now();
			auto exec_t = end_t - start_t;
			auto exec_ms = duration_cast<milliseconds>(exec_t).count();
			std::cout << i << " Threads : SET = ";
			g_set.print20();
			std::cout << ", Exec time = " << exec_ms << "ms.\n;";
			check_history(i);
		}
	}
	{
		for (int i = 1; i <= 16; i = i * 2) {
			thread_count = 0;
			g_num_threads = i;
			g_ebr_counter = 0;

			std::vector <std::thread> threads;
			g_set.clear();
			auto start_t = system_clock::now();
			for (int j = 0; j < i; ++j)
				threads.emplace_back(benchmark, i, j);
			for (auto& th : threads)
				th.join();
			auto end_t = system_clock::now();
			auto exec_t = end_t - start_t;
			auto exec_ms = duration_cast<milliseconds>(exec_t).count();

			std::cout << i << " Threads : SET = ";
			g_set.print20();
			std::cout << ", Exec time = " << exec_ms << "ms.\n;";
		}
	}
}