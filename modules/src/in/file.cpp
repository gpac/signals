#include "../utils/log.hpp"
#include "../utils/tools.hpp"
#include "file.hpp"

#define IOSIZE (64*1024) //FIXME: this can lead to random errors when the module cannot agregate the data itself (e.g. JPEGTurbo decoder)

namespace Modules {
namespace In {

File::File(FILE *file)
	: file(file) {
	signals.push_back(uptr(pinFactory->createPin()));
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
	return new File(f);
}

void File::process(std::shared_ptr<Data> /*data*/) {
	for(;;) {
		auto out(signals[0]->getBuffer(IOSIZE));
		size_t read = fread(out->data(), 1, IOSIZE, file);
		if (read < IOSIZE) {
			if (read == 0) {
				break;
			}
			out->resize(read);
		}
		signals[0]->emit(out);
	}
}

}
}
