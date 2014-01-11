#include "internal/log.hpp"
#include "file.hpp"


#define IOSIZE 65536

File::File(FILE *file)
: file(file) {
	signals.push_back(new Pin);
}

File::~File() {
	fclose(file);
	delete signals[0];//FIXME: use unique_ptr
}

File* File::create(const Param &parameters) {
	auto filename = parameters.find("filename");
	if (filename == parameters.end()) {
		return NULL;
	}
	const std::string &fn = (*filename).second;
	FILE *f = fopen(fn.c_str(), "rb");
	if (!f) {
		Log::get(Log::Error) << "Can't open file: " << fn << std::endl;
		return NULL;
	} else {
		return new File(f);
	}
}

bool File::process(std::shared_ptr<Data> /*data*/) {
	std::shared_ptr<Data> out(new Data(IOSIZE));
	size_t read = fread(out->data(), 1, IOSIZE, file);
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

bool File::canHandle(const std::string &url) {
	return true;
}
