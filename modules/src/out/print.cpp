#include "print.hpp"
#include "../utils/log.hpp"


Print* Print::create(std::ostream &os) {
	return new Print(os);
}

bool Print::process(std::shared_ptr<Data> data) {
	os << "Print: Received data of size: " << data->size() << std::endl;
	return true;
}

bool Print::handles(const std::string &url) {
	return Print::canHandle(url);
}

bool Print::canHandle(const std::string &url) {
	return true;
}

Print::Print(std::ostream &os) : os(os) {
}
