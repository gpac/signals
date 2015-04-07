#include "lib_utils/log.hpp"
#include "lib_utils/tools.hpp"
#include "file.hpp"

#define IOSIZE (64*1024)

namespace Modules {
namespace In {

File::File(std::string const& fn) {
	file = fopen(fn.c_str(), "rb");
	if (!file) {
		Log::msg(Log::Error, "Can't open file for reading: %s", fn);
		throw std::runtime_error("File not found");
	}

	fseek(file, 0, SEEK_END);
	auto size = ftell(file);
	fseek(file, 0, SEEK_SET);
	if (size > IOSIZE)
		Log::msg(Log::Info, "File %s size is %s, will be sent by %s bytes chunks. Check the downstream modules are able to agregate data frames.", fn, size, IOSIZE);

	output = addPin(new PinDefault);
}

File::~File() {
	fclose(file);
}

void File::process(std::shared_ptr<const Data> /*data*/) {
	for(;;) {
		auto out = output->getBuffer(IOSIZE);
		size_t read = fread(out->data(), 1, IOSIZE, file);
		if (read < IOSIZE) {
			if (read == 0) {
				break;
			}
			out->resize(read);
		}
		output->emit(out);
	}
}

}
}
