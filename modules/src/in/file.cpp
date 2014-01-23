#include "../utils/log.hpp"
#include "file.hpp"

#include <iostream> //Romain

#define IOSIZE 65536

namespace Modules {
namespace In {

File::File(FILE *file)
	: file(file) {
	signals.push_back(new PinSync);
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
	std::shared_ptr<Data> out(signals[0]->getBuffer(IOSIZE));
	size_t read = fread(out->data(), 1, IOSIZE, file);
	std::cout << "File: read data of size: " << read << std::endl;
	if (read < IOSIZE) {
		if (read == 0) {
			return false;
		}
		out->resize(read);
	}
	signals[0]->emit(out);
	return true;
}

bool File::handles(const std::string &url) {
	return File::canHandle(url);
}

bool File::canHandle(const std::string &/*url*/) {
	return true;
}

}
}
