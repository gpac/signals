#include "print.hpp"
#include "internal/log.hpp"


Print* Print::create(const Param &parameters) {
	return new Print();
}

bool Print::process() {
#if 0 //Romain
	Log::get(Log::Error) << "Print: received data of size: " << in.size() << std::endl;
	//FIXME: implicit passthru
	return in;
#endif
	return true;
}

bool Print::handles(const std::string &url) {
	return Print::canHandle(url);
}

bool Print::canHandle(const std::string &url) {
	return true;
}

Print::Print() {
}
