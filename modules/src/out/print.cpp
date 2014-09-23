#include "print.hpp"
#include "../utils/log.hpp"


namespace Out {

bool Print::process(std::shared_ptr<Data> data) {
	os << "Print: Received data of size: " << data->size() << std::endl;
	return true;
}

Print::Print(std::ostream &os) : os(os) {
}

}
