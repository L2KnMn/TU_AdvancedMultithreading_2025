#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
#include <array>
#include <queue>
#include <set>

class HISTORY {
public:
	int op;
	int i_value;
	bool o_value;
	HISTORY(int o, int i, bool re) : op(o), i_value(i), o_value(re) {}
};

constexpr int MAX_THREADS = 32;

std::atomic<int> g_ebr_counter = 0;
volatile int num_threads = 1;

thread_local int thread_id;
std::atomic_int thread_ebr[MAX_THREADS * 16];

std::array<std::vector<HISTORY>, MAX_THREADS> history;

struct LFNODE;
thread_local std::queue<LFNODE*> free_list;

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
	int ebr_counter;
	LFNODE() : key(-1) {}
	LFNODE(int x) : key(x) {}
};

LFNODE* ebr_new(int x)
{
	if (free_list.empty()) return new LFNODE(x);
	LFNODE* p = free_list.front();

	int ebr_counter = p->ebr_counter;

	for (int i = 0; i < num_threads; ++i)
		if (thread_ebr[i * 16] < ebr_counter) {
			return new LFNODE(x);
		}
	free_list.pop();
	p->key = x;
	p->next.set_ptr(nullptr);
	p->ebr_counter = 0;
	return p;
}

void ebr_delete(LFNODE* p)
{
	p->ebr_counter = g_ebr_counter;
	free_list.push(p);
}

struct RESPONSE {
	bool m_bool;
};

enum METHOD { M_ADD, M_REMOVE, M_CONTAINS, M_CLEAR, M_PRINT20 };

struct INVOCATION {
	METHOD m_method;
	int x;
};

class SEQOBJECT {
	std::set <int> m_set;
public:
	SEQOBJECT()
	{
	}
	void clear()
	{
		m_set.clear();
	}
	RESPONSE apply(INVOCATION& inv)
	{
		RESPONSE r{ true };
		switch (inv.m_method) {
		case M_ADD:
			r.m_bool = m_set.insert(inv.x).second;
			break;
		case M_REMOVE:
			r.m_bool = (0 != m_set.count(inv.x));
			if (r.m_bool == true)
				m_set.erase(inv.x);
			break;
		case M_CONTAINS:
			r.m_bool = (0 != m_set.count(inv.x));
			break;
		case M_CLEAR:
			m_set.clear();
			break;
		case M_PRINT20: {
			int count = 20;
			for (auto x : m_set) {
				if (count-- == 0) break;
				std::cout << x << ", ";
			}
			std::cout << std::endl;
		}
			break;
		}
		return r;
	}
};

class STD_SEQ_SET {
	SEQOBJECT m_set;
public:
	STD_SEQ_SET()
	{
	}
	void clear()
	{
		INVOCATION inv{ M_CLEAR, 0 };
		m_set.apply(inv);
	}
	bool Add(int x)
	{
		INVOCATION inv{ M_ADD, x };
		return m_set.apply(inv).m_bool;
	}
	bool Remove(int x)
	{
		INVOCATION inv{ M_REMOVE, x };
		return m_set.apply(inv).m_bool;
	}
	bool Contains(int x)
	{
		INVOCATION inv{ M_CONTAINS, x };
		return m_set.apply(inv).m_bool;

	}
	void print20()
	{
		INVOCATION inv{ M_PRINT20, 0 };
		m_set.apply(inv);
	}
};

class U_NODE {
public:
	INVOCATION m_inv;
	std::atomic_int m_seq;
	U_NODE* volatile next;
};

class LFUNV_OBJECT {
	U_NODE* volatile m_head[MAX_THREADS];
	U_NODE tail;
	U_NODE* get_max_head()
	{
		U_NODE* h = m_head[0];
		for (int i = 1; i < MAX_THREADS; ++i)
			if (h->m_seq < m_head[i]->m_seq)
				h = m_head[i];
		return h;
	}
public:
	LFUNV_OBJECT() {
		tail.m_seq = 0;
		tail.next = nullptr;
		for (auto& h : m_head) h = &tail;
	}
	void clear()
	{
		U_NODE* p = tail.next;
		while (nullptr != p) {
			U_NODE* old_p = p;
			p = p->next;
			delete old_p;
		}
		tail.next = nullptr;
		for (auto& h : m_head) h = &tail;
	}

	void print20()
	{
		SEQOBJECT std_set;
		U_NODE* p = tail.next;
		while (p != nullptr) {
			std_set.apply(p->m_inv);
			p = p->next;
		}
		INVOCATION inv{ M_PRINT20, 0 };
		std_set.apply(inv);
	}

	RESPONSE apply(INVOCATION& inv)
	{
		U_NODE* prefer = new U_NODE{ inv, 0, nullptr };
		while (0 == prefer->m_seq) {
			U_NODE* head = get_max_head();
			long long temp = 0;
			std::atomic_compare_exchange_strong(
				reinterpret_cast<volatile std::atomic_llong*>(&head->next),
				&temp,
				reinterpret_cast<long long>(prefer));
			U_NODE* after = head->next;
			after->m_seq = head->m_seq + 1;
			m_head[thread_id] = after;
		}

		SEQOBJECT std_set;
		U_NODE* p = tail.next;
		while (p != prefer) {
			std_set.apply(p->m_inv);
			p = p->next;
		}
		return std_set.apply(inv);
	}
};

class STD_LF_SET {
	LFUNV_OBJECT m_set;
public:
	STD_LF_SET()
	{
	}
	void clear()
	{
		//INVOCATION inv{ M_CLEAR, 0 };
		//m_set.apply(inv);
		m_set.clear();
	}
	bool Add(int x)
	{
		INVOCATION inv{ M_ADD, x };
		return m_set.apply(inv).m_bool;
	}
	bool Remove(int x)
	{
		INVOCATION inv{ M_REMOVE, x };
		return m_set.apply(inv).m_bool;
	}
	bool Contains(int x)
	{
		INVOCATION inv{ M_CONTAINS, x };
		return m_set.apply(inv).m_bool;

	}
	void print20()
	{
		//INVOCATION inv{ M_PRINT20, 0 };
		//m_set.apply(inv);
		m_set.print20();
	}
};


class WFUNV_OBJECT {
	std::atomic<U_NODE*> announce[MAX_THREADS];
	U_NODE* volatile head[MAX_THREADS];
	U_NODE tail;

	U_NODE* get_max_head()
	{
		U_NODE* h = head[0];
		for (int i = 1; i < MAX_THREADS; ++i)
			if (h->m_seq < head[i]->m_seq)
				h = head[i];
		return h;
	}
public:
	WFUNV_OBJECT() {
		tail.m_seq = 1;
		tail.next = nullptr;
		for (int i = 0; i < MAX_THREADS; ++i) {
			head[i] = announce[i] = &tail;
		}
	}
	void clear()
	{
		U_NODE* p = tail.next;
		while (nullptr != p) {
			U_NODE* old_p = p;
			p = p->next;
			delete old_p;
		}
		tail.next = nullptr;
		for (auto& h : head) {
			h = &tail;
		}
		for (auto& h : announce) {
			h = &tail;
		}
	}
	void print20()
	{
		SEQOBJECT std_set;
		U_NODE* p = tail.next;
		while (p != nullptr) {
			std_set.apply(p->m_inv);
			p = p->next;
		}
		INVOCATION inv{ M_PRINT20, 0 };
		std_set.apply(inv);
	}
	RESPONSE apply(INVOCATION& invoc) {
		U_NODE* my_node = new U_NODE{ invoc, 0, nullptr }; 
		announce[thread_id].store(my_node, std::memory_order_release);

		while (announce[thread_id].load(std::memory_order_acquire)->m_seq.load(std::memory_order_acquire) == 0) {
			U_NODE* before = get_max_head(); // 현재 내가 아는 가장 마지막 노드

			int help_thread_index = (before->m_seq.load(std::memory_order_acquire) + 1) % num_threads;
			U_NODE* help_target_node = announce[help_thread_index].load(std::memory_order_acquire);

			U_NODE* prefer_node;
			if (help_target_node != nullptr && help_target_node->m_seq.load(std::memory_order_acquire) == 0) {
				prefer_node = help_target_node; // 다른 쓰레드의 아직 처리 안 된 작업을 우선 돕는다
			}
			else {
				prefer_node = announce[thread_id].load(std::memory_order_acquire); // 내 작업을 처리 시도
			}

			long long temp = 0;
			if (std::atomic_compare_exchange_strong(
				reinterpret_cast<volatile std::atomic_llong*>(&before->next),
				&temp,
				reinterpret_cast<long long>(my_node))) {
				before->next->m_seq.store(before->m_seq + 1, std::memory_order_release);
				head[thread_id] = before->next;
			}
		}

		SEQOBJECT myObject; // 로컬 순차 객체
		U_NODE* current = tail.next; 
		U_NODE* own_node_in_log = announce[thread_id].load(std::memory_order_acquire);

		while (current != nullptr && current != own_node_in_log) {
			myObject.apply(current->m_inv);
			current = current->next;
		}
		
		// 자신의 연산을 적용하고 결과 반환
		if (current == own_node_in_log) { // current가 자신의 노드까지 도달했는지 확인
			return myObject.apply(current->m_inv);
		}
		else {
			// 여기까지 오면 뭔가 문제가 있는 상황 (예: own_node_in_log가 로그에 없거나, current가 nullptr이 됨)
			// 실제 환경에서는 오류 처리 또는 예외 발생
			std::cerr << "Error: Thread " << thread_id << " could not find its own node in the log." << std::endl;
			RESPONSE error_response;
			// error_response.m_bool = false; // 예시
			return error_response;
		}
	}
};


class STD_WF_SET {
	WFUNV_OBJECT m_set;
public:
	STD_WF_SET()
	{
	}
	void clear()
	{
		//INVOCATION inv{ M_CLEAR, 0 };
		//m_set.apply(inv);
		m_set.clear();
	}
	bool Add(int x)
	{
		INVOCATION inv{ M_ADD, x };
		return m_set.apply(inv).m_bool;
	}
	bool Remove(int x)
	{
		INVOCATION inv{ M_REMOVE, x };
		return m_set.apply(inv).m_bool;
	}
	bool Contains(int x)
	{
		INVOCATION inv{ M_CONTAINS, x };
		return m_set.apply(inv).m_bool;

	}
	void print20()
	{
		//INVOCATION inv{ M_PRINT20, 0 };
		//m_set.apply(inv);
		m_set.print20();
	}
};


constexpr int NUM_TEST = 40000;
constexpr int KEY_RANGE = 1000;

STD_LF_SET g_set;

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
	thread_id = th_id;
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
}

void benchmark(int num_thread, int th_id)
{
	thread_id = th_id;
	int key;
	const int num_loop = NUM_TEST / num_thread;

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
}

int main()
{
	using namespace std::chrono;

	while (false)
	{
		auto start_t = system_clock::now();
		benchmark(1, 0);
		auto end_t = system_clock::now();
		auto exec_t = end_t - start_t;
		auto exec_ms = duration_cast<milliseconds>(exec_t).count();

		std::cout << "Single Thread SET = ";
		g_set.print20();
		std::cout << ", Exec time = " << exec_ms << "ms.\n;";
	}

	// 알고리즘 정확성 검사
	{
		for (int i = 1; i <= MAX_THREADS; i = i * 2) {
			num_threads = i;
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
		for (int i = 1; i <= MAX_THREADS; i = i * 2) {
			num_threads = i;
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

