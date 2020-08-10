#include "../logging.hpp"

void* test(void* no) {
	pthread_t tid = pthread_self();
	char id[17];
	snprintf(id, 17, "%x", tid);
	std::string text("This is a test: ");
	text += std::string(100, 'a');
	for (int i = 0; i < 5000000; ++i) {
		logging::info(text.c_str(), id);
	}
	return 0;
}

struct income {
	income(size_t no): no_(no) {}
	size_t no_;
};

void* multi_test(void* no) {
	pthread_t tid = pthread_self();
	char id[19];
	income* log = (income*)no;
	snprintf(id, 19, "0x%x", tid);
	std::string text("This is a test: ");
	text += std::string(100, 'a');
	for (int i = 0; i < 10000000; ++i) {
		logging::info(text.c_str(), id, log->no_);
	}
	return 0;
}

class logtest {
public:
	logtest(const char* name): log_(new Logging::fileHandler(name)) {
		logging::config("tester", Logging::DEBUG, "This is a lonnnng header.", log_);
	}
	~logtest() { delete log_; }
	void test_sync() {
		std::string text("This is a test: ");
		text += std::string(100, 'a');
		for (int i = 0; i < 100000000; ++i) {
			logging::info(text.c_str());
		}
		logging::info("I am done.", "main");
		logging::flush();
	}
	void test_asyn() {
		const int tnum = 10;
		Thread* threads[tnum];
		for (int i = 0; i < tnum; ++i)
			threads[i] = new Thread(test, (void*)1, false);
		for (int i = 0; i < tnum; ++i) {
			threads[i]->join();
			delete threads[i];
		}
		logging::info("I am done.", "main");
		//std::cout << "waiting flush" << std::endl;
		logging::flush();
	}
	void test_multi_asyn() {
		Logging::fileHandler file("multi_asyn2.log");
		size_t id = logging::config("tester2", Logging::INFO, "", &file);
		income out1(0), out2(id);
		Thread dft(multi_test, (void*)&out1);
		Thread other(multi_test, (void*)&out2);
		for (int i = 0; i < 10000000; ++i)
			logging::debug("This is a debug information", id);
		dft.join();
		other.join();
		logging::flush();
	}
private:
	Logging::Handler* log_;
	void print(const bool flag) {
		if (flag)
			std::cout << "result is true" << std::endl;
		else
			std::cout << "result is flase" << std::endl;
	}
};
