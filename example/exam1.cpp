#include "../logging.hpp"

int main()
{
	logging::info("This is a test.");
	logging::debug("This is another test.", "NamedSource");
	logging::flush();
	return 0;
}
