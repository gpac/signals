#include "../utils/tools.hpp"
#include "recorder.hpp"

namespace Modules {
namespace Utils {

void Recorder::flush() {
	record.clear();
}

Recorder::~Recorder() {
}

void Recorder::process(std::shared_ptr<Data> data) {
	record.push(data);
}

std::shared_ptr<Data> Recorder::pop() {
	return record.pop();
}

}
}
