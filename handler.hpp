#ifndef HANDLER_HPP_
#define HANDLER_HPP_

#include <cstdio>

namespace Logging {

	class Handler {
		Handler(Handler& rhs) {}
		Handler& operator=(Handler& rhs) { return *this; }
	public:
		explicit Handler() {}
		virtual ~Handler() {}
		virtual void write(const char* text, const size_t len) {
			printf("%.*s", len, text);
		}
	};

	class fileHandler : public Handler {
		fileHandler(fileHandler& rhs) {}
		fileHandler& operator=(fileHandler& rhs) { return *this; }
	public:
		explicit fileHandler(const char* name, const char* mode = "w") : file_(fopen(name, mode)) {
			if (!file_)
				throw std::runtime_error("can not create output file");
		}
		~fileHandler() { fclose(file_); }
		void write(const char* text, const size_t len) {
			fwrite(text, sizeof(char), len, file_);
		}
	private:
		FILE* file_;
	};
}

#endif