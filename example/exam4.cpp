#include "../logging.hpp"

int main()
{
	Logging::fileHandler file1("test1.log");
	Logging::fileHandler file2("test2.log");
	size_t log_id1 = logging::config("tester1", Logging::DEBUG, "This is a header.", &file1);
	size_t log_id2 = logging::config("tester2", Logging::DEBUG, "This is a header.", &file2);
	logging::info("This is a test.", log_id1);
	logging::debug("This is another test.", "source", log_id2);
	logging::flush();
	return 0;
}
