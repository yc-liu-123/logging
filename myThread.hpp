#ifndef MYTHREAD_HPP_
#define MYTHREAD_HPP_

#include <pthread.h>
#include <iostream>
#include <cstring>

class Thread {
	pthread_t thread_;
	bool detached_;
	bool joinable_;
public:
	Thread(void* (*func)(void*), void *arg, bool detach = false, int stack_size = 16383, void* stack_addr = NULL):
		thread_(), detached_(detach), joinable_(!detach) {
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		detached_ ? 
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) : 
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		if (stack_size > 16383 && stack_addr != NULL) {
			if (pthread_attr_setstack(&attr, stack_addr, stack_size) != 0)
				threadError("fail to set stack size", true);
		}
		else if (stack_size > 16383) {
			if (pthread_attr_setstacksize(&attr, stack_size) != 0)
				threadError("fail to set stack size", true);
		}
		pthread_create(&thread_, &attr, func, arg);
		pthread_attr_destroy(&attr);
	}
	~Thread() {
		if (joinable_) {
			threadError("thread unjoined");
			join();
		}
	}
	bool operator==(Thread& rhs) { return pthread_equal(thread_, (rhs.thread_)); }
	void join(void* res_ptr = nullptr) {
		if (!joinable_)
			threadError("thread can not be joined", true);
		int iRet = pthread_join(thread_, &res_ptr);
		if (iRet != 0)
			res_ptr = 0;
		else
			joinable_ = false;
	}
	void detach() {
		if (detached_)
			threadError("thread has been detached");
		detached_ = true;
		joinable_ = false;
		int iRet = pthread_detach(thread_);
		if (iRet != 0)
			threadError("error happen in detaching thread");
	}
	bool joinable() { return joinable_; }
	bool getdetachstate() { return detached_; }
	pthread_t getID() { return thread_; }
private:
	Thread(Thread& other) {}
	Thread& operator=(Thread& rhs) { return *this; }
	void threadError(const char* msg = "", bool throw_ = false) {
		std::cerr << msg << std::endl;
		if (throw_)
			throw std::runtime_error(msg);
	}
};

class Mutex {
	friend class Condvar;
public:
	Mutex(): mutex_() {
		if (pthread_mutex_init(&mutex_, NULL) != 0)
			mutexError("fail to create mutex", true);
	}
	~Mutex() {
		pthread_mutex_destroy(&mutex_);
	}
	int lock(bool block = true) {
		return block ? pthread_mutex_lock(&mutex_) : pthread_mutex_trylock(&mutex_);
	}
	int unlock() {
		return pthread_mutex_unlock(&mutex_);
	}
private:
	pthread_mutex_t mutex_;
	Mutex(Mutex& other) {}
	Mutex& operator=(Mutex& rhs) { return *this; }
	void mutexError(const char* msg = "", bool throw_ = false) {
		std::cerr << msg << std::endl;
		if (throw_)
			throw std::runtime_error(msg);
	}
};

class Condvar {
public:
	Condvar(): cond_(PTHREAD_COND_INITIALIZER) {}
	~Condvar() { pthread_cond_destroy(&cond_); }
	void wait(Mutex& mu) {
		pthread_cond_wait(&cond_, &(mu.mutex_));
	}
	void signal() {
		pthread_cond_signal(&cond_);
	}
	void boardcast() {
		pthread_cond_broadcast(&cond_);
	}
private:
	pthread_cond_t cond_;
	Condvar(Condvar& other) {}
	Condvar& operator=(Condvar& rhs) { return *this; }
};

class lock_guard {
public:
	lock_guard(Mutex& mutex): myMutex(&mutex) { myMutex->lock(); }
	~lock_guard() { myMutex->unlock(); }
private:
	Mutex* myMutex;
};

#endif // !MYTHREAD