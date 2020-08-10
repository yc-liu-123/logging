#include "../logging.hpp"

int main()
{
	logging::config("tester", Logging::DEBUG, "This is a header.");
	logging::info("This is a test.");
	logging::debug("This is another test.", "source");
	logging::flush();
	return 0;
}
