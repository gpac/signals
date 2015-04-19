#include "print.hpp"
#include "lib_utils/log.hpp"


namespace Modules {
namespace Out {

void Print::process(Data data_) {
	auto data = safe_cast<const RawData>(data_);
	os << "Print: Received data of size: " << data->size() << std::endl;
}

Print::Print(std::ostream &os) : os(os) {
	auto input = addInput(new Input<RawData>(this));
}

}
}
