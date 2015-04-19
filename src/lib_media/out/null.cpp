#include "null.hpp"
#include "lib_utils/log.hpp"


namespace Modules {
namespace Out {

Null::Null() {
	auto input = addInput(new Input<DataBase>(this));
}

void Null::process(Data data) {
}

}
}
