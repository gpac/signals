#include "print.hpp"
#include "../utils/log.hpp"


namespace Out {

Print* Print::create(std::ostream &os) {
	return new Print(os);
}

bool Print::process(std::shared_ptr<Data> data) {
	os << "Print: Received data of size: " << data->size() << std::endl;
	return true;
}

Print::Print(std::ostream &os) : os(os) {
}

}
