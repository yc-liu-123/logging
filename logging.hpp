#ifndef MYLOGGING_HPP_
#define MYLOGGING_HPP_

#include "myThread.hpp"
#include "handler.hpp"
#include "Timer.hpp"

#include <cstring>

namespace Logging {

	enum levels { CRITICAL = 50, ERROR = 40, WARNING = 30, INFO = 20, DEBUG = 10, VERBOSE = 0 };

	void* writer(void*);

	class Records {
		static Records* Recs;
		static Mutex Lock_;

		friend void* writer(void*);

		Records();
	public:
		static const unsigned int INIT_SIZE = 8;
		static const unsigned int TEXT_SIZE = 1024 * 1024;
		static const unsigned int SLOT_SIZE = 1024;

		static Records& instance();
		void input(const char*, const char*, const char*, const unsigned int);
		void flush();

	private:

		class Cell {
		public:
			Cell* next_;
			int wout_;
			bool full_;
			unsigned int cur_pos;

			Cell(Cell* next = nullptr) :
				next_(next), full_(false), text_(""), cur_pos(0), wout_(-1) {}
			void input(const char*, const unsigned int, const unsigned int);
			void output();

		private:
			char text_[TEXT_SIZE];
		};

		UTC_timer timer_;
		unsigned int num_;
		Cell* in_, * out_;
		Mutex record_, sig_;
		Condvar cond_;
		bool ready_, writing_;
		Thread daemon_;
	};

	struct Logger {
		Logger(const int level, const unsigned int out, const char* source): 
			level_(level), out_(out), source_(new char[strlen(source) + 1]) {
			strcpy(source_, source);
		}
		~Logger() { delete [] source_; }
		char* source_;
		unsigned int out_;
		int level_;
	};
}

class logging {
	static logging* man;
	static Mutex Lock_;
	static UTC_timer timer_;
	static const int num_ = 10;
	int use_log, free_log, free_han;
	Mutex llock, hlock;
	bool blocking_;
	Logging::Logger* logMap[num_];
	Logging::Handler* handleMap[num_ + 1];
public:
	static logging& instance();
	static int config(const char* name = "main", const int level = Logging::INFO, const char* header = "",
		Logging::Handler* han = nullptr);
	static int close(unsigned int idx);
	static Logging::Logger* getlogger(const unsigned int);
	static Logging::Handler* gethandle(const unsigned int);
	static void flush();
	static void verbose(const char* msg, const unsigned int log_idx = 0);
	static void verbose(const char* msg, const char* source, const unsigned int log_idx = 0);
	static void debug(const char* msg, const unsigned int log_idx = 0);
	static void debug(const char* msg, const char* source, const unsigned int log_idx = 0);
	static void info(const char* msg, const unsigned int log_idx = 0);
	static void info(const char* msg, const char* source, const unsigned int log_idx = 0);
	static void warning(const char* msg, const unsigned int log_idx = 0);
	static void warning(const char* msg, const char* source, const unsigned int log_idx = 0);
	static void critical(const char* msg, const unsigned int log_idx = 0);
	static void critical(const char* msg, const char* source, const unsigned int log_idx = 0);
private:
	logging();
	Logging::Logger* glogger(const unsigned int);
	int createLogger(const char* name, const int level = Logging::INFO, const unsigned int out = 0, const char* header = "");
	int rmLogger(unsigned int);
	int addHandle(Logging::Handler*);
	int rmHandle(unsigned int);
};

#endif // !LOGGING_HPP_