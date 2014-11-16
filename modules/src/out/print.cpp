#include "print.hpp"
#include "../utils/log.hpp"


namespace Modules {
namespace Out {

void Print::process(std::shared_ptr<const Data> data) {
	os << "Print: Received data of size: " << data->size() << std::endl;
}

Print::Print(std::ostream &os) : os(os) {
}

}
}
