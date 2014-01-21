#include "../utils/log.hpp"
#include "file.hpp"


#define IOSIZE 65536

namespace Modules {
namespace In {

File::File(FILE *file)
	: file(file) {
	signals.push_back(new Pin());
}

File::~File() {
	fclose(file);
	delete signals[0];
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
	assert(false); // this module has no input
	return false;
}

void File::push() {
	for(;;) {
		std::shared_ptr<Data> out(signals[0]->getBuffer(IOSIZE));
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

bool File::handles(const std::string &url) {
	return File::canHandle(url);
}

bool File::canHandle(const std::string &/*url*/) {
	return true;
}

}
}
