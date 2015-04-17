#include "lib_utils/tools.hpp"
#include "recorder.hpp"

namespace Modules {
namespace Utils {

void Recorder::flush() {
	record.clear();
}

void Recorder::process(Data data) {
	record.push(data);
}

Data Recorder::pop() {
	return record.pop();
}

}
}
