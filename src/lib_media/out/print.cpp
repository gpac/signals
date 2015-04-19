#include "print.hpp"
#include "lib_utils/log.hpp"


namespace Modules {
namespace Out {

void Print::process(Data data_) {
	auto data = safe_cast<const DataRaw>(data_);
	os << "Print: Received data of size: " << data->size() << std::endl;
}

Print::Print(std::ostream &os) : os(os) {
	addInput(new Input<DataRaw>(this));
}

}
}
