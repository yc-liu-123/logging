#include "../logging.hpp"

int main()
{
	Logging::fileHandler file("test.log");
	logging::config("tester", Logging::DEBUG, "This is a header.", &file);
	logging::info("This is a test.");
	logging::debug("This is another test.", "source");
	logging::flush();
	return 0;
}
