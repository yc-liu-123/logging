#include "logging.hpp"

//definition of writing function as daemon thread

void* Logging::writer(void* meanless) {
	lock_guard g(Records::instance().sig_);
	Records::instance().ready_ = true;
	while (true) {
		Records::instance().cond_.wait(Records::instance().sig_);
		while (Records::instance().out_->full_) {
			if (Records::instance().out_->cur_pos > 0)
				Records::instance().out_->output();
			Records::instance().out_ = Records::instance().out_->next_;
		}
	}
	return 0;
}

//definition of menber functions in <Logging::Records::Cell>

void Logging::Records::Cell::input(const char* text, const unsigned int len, const unsigned int out)
{
	wout_ = out;
	strncpy(text_ + cur_pos, text, len);
	cur_pos += len;
	if (cur_pos >= TEXT_SIZE - 1)
		full_ = true;
}

void Logging::Records::Cell::output() {
	if (cur_pos > 0) {
		logging::gethandle(wout_)->write(text_, cur_pos);
		wout_ = -1;
		cur_pos = 0;
		full_ = false;
	}
	else
		throw std::runtime_error("empty Cell");
}

//definition of menber functions in <Logging::Records>

Logging::Records& Logging::Records::instance() {
	if (Recs == nullptr) {
		lock_guard guard(Lock_);
		if (Recs == nullptr)
			Recs = new Records();
	}
	return *Recs;
}

void Logging::Records::input(const char* level, const char* source, const char* msg, const unsigned int textout)
{
	double past;
	char text[TEXT_SIZE];
	timer_.get_curr_time(past);
	int len = snprintf(text, Records::TEXT_SIZE, "%s [%s] %.2f [%s]: %s\n", 
		timer_.utc_fmt, level, past, source, msg);
	bool sigw(false);
	record_.lock();
	if ((in_->full_ || TEXT_SIZE - in_->cur_pos - 1 < len) || 
		(in_->wout_ != -1 && textout != in_->wout_)) {
		if (!in_->next_->full_ && num_ < SLOT_SIZE) {
			in_->next_ = new Cell(in_->next_);
			++num_;
		}
		sigw = true;
		if (!(in_->next_->full_)) {
			in_->full_ = true;
			in_ = in_->next_;
			in_->input(text, len, textout);
		}
	}
	else {
		in_->input(text, len, textout);
	}
	record_.unlock();
	if (sigw) {
		if (ready_ && sig_.lock(false) == 0) {
			cond_.signal();
			sig_.unlock();
		}
	}
}

void Logging::Records::flush()
{
	in_->full_ = true;
	while (!ready_);
	while (out_->cur_pos != 0 || out_->cur_pos != 0) {
		if (sig_.lock(false) == 0) {
			cond_.signal();
			sig_.unlock();
		}
	}
}

Logging::Records::Records() : 
	num_(INIT_SIZE), record_(), sig_(), cond_(), timer_(),
	ready_(false), writing_(true), daemon_(writer, (void*)NULL, true) {
	Cell* head(nullptr), * tail(nullptr);
	for (int i = 0; i < num_; ++i) {
		in_ = new Cell(head);
		head = in_;
		if (tail == nullptr)
			tail = in_;
	}
	tail->next_ = head;
	in_ = head;
	out_ = in_;
}

//definition of menber functions in <logging>

logging& logging::instance()
{
	if (man == nullptr) {
		lock_guard guard(Lock_);
		if (man == nullptr)
			man = new logging();
	}
	return *man;
}

int logging::config(const char* name, const int level, const char* header, Logging::Handler* han) {
	instance();
	Logging::Records::instance();
	if (han == nullptr)
		return instance().createLogger(name, level, 0, header);
	else {
		int idx(instance().addHandle(han));
		return instance().createLogger(name, level, idx, header);
	}
}

int logging::close(unsigned int idx)
{
	return instance().rmLogger(idx);
}

Logging::Logger* logging::getlogger(const unsigned int idx)
{
	return instance().glogger(idx);
}

Logging::Handler* logging::gethandle(const unsigned int idx)
{
	if (idx < num_ && instance().handleMap[idx] != nullptr) {
		return instance().handleMap[idx];
	}
	return instance().handleMap[0];
}

void logging::flush()
{
	instance().blocking_ = true;
	Logging::Records::instance().flush();
	instance().blocking_ = false;
}

void logging::verbose(const char* msg, const unsigned int log_idx)
{
	if (!instance().blocking_) {
		Logging::Logger* lg(instance().glogger(log_idx));
		if (lg == nullptr)
			Logging::Records::instance().input("VERBOSE", "default", msg, 0);
		else if (lg->level_ <= Logging::VERBOSE)
			Logging::Records::instance().input("VERBOSE", lg->source_, msg, lg->out_);
	}
}

void logging::verbose(const char* msg, const char* source, const unsigned int log_idx)
{
	if (!instance().blocking_) {
		Logging::Logger* lg(getlogger(log_idx));
		if (lg == nullptr)
			Logging::Records::instance().input("VERBOSE", source, msg, 0);
		else if (lg->level_ <= Logging::VERBOSE)
			Logging::Records::instance().input("VERBOSE", source, msg, lg->out_);
	}
}

void logging::debug(const char* msg, const unsigned int log_idx)
{
	if (!instance().blocking_) {
		Logging::Logger* lg(instance().glogger(log_idx));
		if (lg == nullptr)
			Logging::Records::instance().input("DEBUG", "default", msg, 0);
		else if (lg->level_ <= Logging::DEBUG)
			Logging::Records::instance().input("DEBUG", lg->source_, msg, lg->out_);
	}
}

void logging::debug(const char* msg, const char* source, const unsigned int log_idx)
{
	if (!instance().blocking_) {
		Logging::Logger* lg(getlogger(log_idx));
		if (lg == nullptr)
			Logging::Records::instance().input("DEBUG", source, msg, 0);
		else if (lg->level_ <= Logging::DEBUG)
			Logging::Records::instance().input("DEBUG", source, msg, lg->out_);
	}
}

void logging::info(const char* msg, const unsigned int log_idx)
{
	if (!instance().blocking_) {
		Logging::Logger* lg(instance().glogger(log_idx));
		if (lg == nullptr)
			Logging::Records::instance().input("INFO", "default", msg, 0);
		else if (lg->level_ <= Logging::INFO)
			Logging::Records::instance().input("INFO", lg->source_, msg, lg->out_);
	}
}

void logging::info(const char* msg, const char* source, const unsigned int log_idx)
{
	if (!instance().blocking_) {
		Logging::Logger* lg(getlogger(log_idx));
		if (lg == nullptr)
			Logging::Records::instance().input("INFO", source, msg, 0);
		else if (lg->level_ <= Logging::INFO)
			Logging::Records::instance().input("INFO", source, msg, lg->out_);
	}
}

void logging::warning(const char* msg, const unsigned int log_idx)
{
	if (!instance().blocking_) {
		Logging::Logger* lg(instance().glogger(log_idx));
		if (lg == nullptr)
			Logging::Records::instance().input("WARNING", "default", msg, 0);
		else if (lg->level_ <= Logging::WARNING)
			Logging::Records::instance().input("WARNING", lg->source_, msg, lg->out_);
	}
}

void logging::warning(const char* msg, const char* source, const unsigned int log_idx)
{
	if (!instance().blocking_) {
		Logging::Logger* lg(getlogger(log_idx));
		if (lg == nullptr)
			Logging::Records::instance().input("WARNING", source, msg, 0);
		else if (lg->level_ <= Logging::WARNING)
			Logging::Records::instance().input("WARNING", source, msg, lg->out_);
	}
}

void logging::critical(const char* msg, const unsigned int log_idx)
{
	if (!instance().blocking_) {
		Logging::Logger* lg(instance().glogger(log_idx));
		if (lg == nullptr)
			Logging::Records::instance().input("CRITICAL", "default", msg, 0);
		else if (lg->level_ <= Logging::CRITICAL)
			Logging::Records::instance().input("CRITICAL", lg->source_, msg, lg->out_);
	}
}

void logging::critical(const char* msg, const char* source, const unsigned int log_idx)
{
	if (!instance().blocking_) {
		Logging::Logger* lg(getlogger(log_idx));
		if (lg == nullptr)
			Logging::Records::instance().input("CRITICAL", source, msg, 0);
		else if (lg->level_ <= Logging::CRITICAL)
			Logging::Records::instance().input("CRITICAL", source, msg, lg->out_);
	}
}

logging::logging() : blocking_(false), use_log(-1), free_log(0), free_han(1), llock(), hlock() {
	Logging::Handler* std_han = new Logging::Handler();
	handleMap[0] = std_han;
}

Logging::Logger* logging::glogger(const unsigned int idx)
{
	while (blocking_);
	if (idx < num_ && logMap[idx] != nullptr) {
		return logMap[idx];
	}
	else if (use_log != -1)
		return logMap[use_log];
	return nullptr;
}

int logging::createLogger(const char* name, const int level, const unsigned int out, const char* header)
{
	if (free_log < num_) {
		Logging::Logger* lg = new Logging::Logger(level, out, name);
		llock.lock();
		int p(free_log);
		instance().logMap[free_log] = lg;
		if (use_log < 0)
			use_log = free_log;
		while (free_log < num_ && instance().logMap[free_log] != nullptr)
			++free_log;
		llock.unlock();
		double past;
		timer_.get_curr_time(past);
		unsigned int len(strlen(name) + 19 + 24);
		char str[len];
		snprintf(str, len + 1, "Log for [%s] created at: %s\n", name, timer_.utc_fmt);
		handleMap[out]->write(str, strlen(str));
		if (strlen(header) > 0) {
			handleMap[out]->write(header, strlen(header));
			handleMap[out]->write("\n", 1);
		}
		return p;
	}
	return -1;
}

int logging::rmLogger(unsigned int idx)
{
	if (idx >= 0 && idx < num_ && logMap[idx] != nullptr) {
		lock_guard g(llock);
		if (idx == use_log) {
			int p(0);
			while (p < num_ && logMap[p] == nullptr && p != idx)
				++p;
			if (p == num_)
				return 0;
			use_log = p;
		}
		if (idx < free_log)
			free_log = idx;
		rmHandle(logMap[idx]->out_);
		delete logMap[idx];
		logMap[idx] = nullptr;
		return 1;
	}
	return 0;
}

int logging::addHandle(Logging::Handler* han)
{
	if (free_han < num_) {
		lock_guard g(hlock);
		int p(free_han);
		instance().handleMap[free_han] = han;
		while (free_han < num_ && instance().handleMap[free_han] != nullptr)
			++free_han;
		return p;
	}
	return -1;
}

int logging::rmHandle(unsigned int idx)
{
	if (idx > 0 && idx < num_ + 1 && handleMap[idx] != nullptr) {
		lock_guard g(hlock);
		delete handleMap[idx];
		handleMap[idx] = nullptr;
		if (idx < free_han)
			free_han = idx;
		return 1;
	}
	return 0;
}

const unsigned int Logging::Records::INIT_SIZE;
const unsigned int Logging::Records::TEXT_SIZE;
const unsigned int Logging::Records::SLOT_SIZE;
Logging::Records* Logging::Records::Recs(nullptr);
Mutex Logging::Records::Lock_;

logging* logging::man(nullptr);
Mutex logging::Lock_;
UTC_timer logging::timer_;
const int logging::num_;