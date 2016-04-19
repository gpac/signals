#include "print.hpp"


namespace Modules {
namespace Out {

void Print::process(Data data_) {
	auto data = safe_cast<const DataBase>(data_);
	os << "Print: Received data of size: " << data->size() << std::endl;
}

Print::Print(std::ostream &os) : os(os) {
	addInput(new Input<DataBase>(this));
}

}
}
