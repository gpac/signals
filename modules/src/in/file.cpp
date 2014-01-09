#include "internal/log.hpp"
#include "file.hpp"


#define IOSIZE 16384

File::File(FILE *file)
: file(file) {
}

File::~File() {
	fclose(file);
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

bool File::process() {
#if 0 //Romain
	if (in.size()) {
		//FIXME
		Log::get(Log::Error) << "Module File doesn't take any input!" << std::endl;
		in.clear();
	}

	std::vector<char*> &data = in;
	data.resize(IOSIZE);
	size_t read = fread(data.data(), 1, IOSIZE, file);
	if (read < IOSIZE) {
		data.resize(read);
	}

	return data;
#endif
	return true;
}

bool File::handles(const std::string &url) {
	return File::canHandle(url);
}

bool File::canHandle(const std::string &url) {
	return true;
}
