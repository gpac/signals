#include "../utils/log.hpp"
#include "../utils/tools.hpp"
#include "file.hpp"

#define IOSIZE (64*1024)

namespace Modules {
namespace In {

File::File(FILE *file)
	: file(file) {
	PinDefaultFactory pinFactory;
	pins.push_back(uptr(pinFactory.createPin()));
}

File::~File() {
	fclose(file);
}

File* File::create(std::string const& fn) {
	FILE *f = fopen(fn.c_str(), "rb");
	if (!f) {
		Log::msg(Log::Error, "Can't open file for reading: %s", fn);
		throw std::runtime_error("File not found");
	}

	fseek(f, 0, SEEK_END);
	auto size = ftell(f);
	fseek(f, 0, SEEK_SET);
	if (size > IOSIZE)
		Log::msg(Log::Info, "File %s size is %s, will be sent by %s bytes chunks. Check the downstream modules are able to agregate data frames.", fn, size, IOSIZE);

	return new File(f);
}

void File::process(std::shared_ptr<const Data> /*data*/) {
	for(;;) {
		auto out(pins[0]->getBuffer(IOSIZE));
		size_t read = fread(out->data(), 1, IOSIZE, file);
		if (read < IOSIZE) {
			if (read == 0) {
				break;
			}
			out->resize(read);
		}
		pins[0]->emit(out);
	}
}

}
}
