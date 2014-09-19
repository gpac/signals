#include "../utils/log.hpp"
#include "../utils/tools.hpp"
#include "file.hpp"

#define IOSIZE 65536

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
		Log::msg(Log::Error, "Can't open file: %s", fn);
		throw std::runtime_error("File not found");
	}
	return new File(f);
}

bool File::process(std::shared_ptr<Data> /*data*/) {
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
	return false; // no more data to process
}

bool File::handles(const std::string &url) {
	return File::canHandle(url);
}

bool File::canHandle(const std::string &/*url*/) {
	return true;
}

}
}
