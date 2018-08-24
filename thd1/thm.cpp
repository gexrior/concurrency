#include <queue>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include "unistd.h" 
#include <windows.h>
//first time to push
class Task {
private:
	int no;
public:
	Task(int n) {
		no = n;
	}
	//���Լ̳��������д�÷���ִ������
	virtual void run() {
		Sleep(no); //����ʱ����ִ�м��룬ģ���߳�����
		std::cout << no << "T\n";
	}
};

class Thread {
private:
	std::thread _thread;
	bool _isfree;
	Task *_task;
	std::mutex _locker;
public:
	//����
	Thread() : _isfree(true), _task(nullptr) {
		_thread = std::thread(&Thread::run, this);
		_thread.detach(); //�ŵ���̨�� join�ǵȴ��߳̽���
	}
	//�Ƿ����
	bool isfree() {
		return _isfree;
	}
	//�������
	void add_task(Task *task) {
		if (_isfree) {
			_locker.lock();
			_task = task;
			_isfree = false;
			_locker.unlock();
		}
	}
	//�����������ִ�����񣬷�������
	void run() {
		while (true) {
			if (_task) {
				_locker.lock();
				_isfree = false;
				_task->run();
				_isfree = true;
				_task = nullptr;
				_locker.unlock();
			}
		}
	}
};
class ThreadPool {
private:
	std::queue<Task *> task_queue;
	std::vector<Thread *> _pool;
	std::mutex _locker;
public:
	//�����̲߳���ִ̨�У�Ĭ������Ϊ10
	ThreadPool(int n = 10) {
		while (n--) {
			Thread *t = new Thread();
			_pool.push_back(t);
		}
		std::thread main_thread(&ThreadPool::run, this);
		main_thread.detach();
	}
	//�ͷ��̳߳�
	~ThreadPool() {
		for (int i = 0; i < _pool.size(); ++i) {
			delete _pool[i];
		}
	}
	//�������
	void add_task(Task *task) {
		_locker.lock();
		task_queue.push(task);
		_locker.unlock();
	}
	//��ѯ
	void run() {
		while (true) {
			_locker.lock();
			if (task_queue.empty()) {
				continue;
			}
			// Ѱ�ҿ����߳�ִ������
			for (int i = 0; i < _pool.size(); ++i) {
				if (_pool[i]->isfree()) {
					_pool[i]->add_task(task_queue.front());
					task_queue.pop();
					break;
				}
			}
			_locker.unlock();
		}
	}
};
int main() {
	ThreadPool tp(2);

	Task t1(1);
	Task t2(3);
	Task t3(2);
	tp.add_task(&t1);
	tp.add_task(&t2);
	tp.add_task(&t3);

	Sleep(4);   //�ȴ���������������Ȼ�����
	return 0;
}