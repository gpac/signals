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

bool Null::handles(const std::string &url) {
	return Null::canHandle(url);
}

bool Null::canHandle(const std::string &/*url*/) {
	return true;
}

}
