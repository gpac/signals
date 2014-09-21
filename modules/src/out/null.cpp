#include "null.hpp"
#include "../utils/log.hpp"


namespace Out {

Null* Null::create() {
	return new Null();
}

Null::Null() {
}

bool Null::process(std::shared_ptr<Data> data) {
	return true;
}

}
